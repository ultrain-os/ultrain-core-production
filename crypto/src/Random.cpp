#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>

#include <fc/exception/exception.hpp>

namespace ultrainio {

/**
 * EC VRF suite.
 */
struct ecvrf_suite {
    EC_GROUP *group;
    const EVP_MD *hash;
    const size_t proof_size;
    const size_t ecp_size;
    const size_t c_size;
    const size_t s_size;
};

typedef struct ecvrf_suite ecvrf_suite;

/**
 * Get EC-VRF-P256-SHA256 implementation.
 */
static ecvrf_suite *ecvrf_p256(void)
{
    struct ecvrf_suite tmp = {
        .group = EC_GROUP_new_by_curve_name(NID_secp256k1),
        .hash = EVP_sha256(),
        .proof_size = 81,
        .ecp_size = 33,
        .c_size = 16,
        .s_size = 32,
    };

    if (!tmp.group) {
        return NULL;
    }

    struct ecvrf_suite *result = (struct ecvrf_suite*)malloc(sizeof(struct ecvrf_suite));
    if (!result) {
        return NULL;
    }

    memcpy(result, &tmp, sizeof(ecvrf_suite));
    return result;
}

/**
 * Free EC VRF implementation.
 */
static void ecvfr_free(struct ecvrf_suite *suite) {
    if (!suite) {
        return;
    }

    EC_GROUP_free(suite->group);
    free(suite);
}

/**
 * Get number of bytes that fit given number of bits.
 *
 * ceil(div(bits/8))
 */
static int bits_in_bytes(int bits)
{
    return (bits + 7) / 8;
}

/**
 * Compute r = p1^f1 + p2^f2
 */
static EC_POINT *ec_mul_two(const EC_GROUP *group, const EC_POINT *p1, const BIGNUM *f1, const EC_POINT *p2, const BIGNUM *f2)
{
    EC_POINT *result = EC_POINT_new(group);
    if (!result) {
        return NULL;
    }

    const EC_POINT *points[] = { p1, p2 };
    const BIGNUM *factors[] = { f1, f2 };
    if (EC_POINTs_mul(group, result, NULL, 2, points, factors, NULL) != 1) {
        EC_POINT_clear_free(result);
        return NULL;
    }

    return result;
}

/**
 * Try converting random string to EC point.
 *
 * @return EC point or NULL if the random string cannot be interpreted as an EC point.
 */
static EC_POINT *RS2ECP(const EC_GROUP *group, const uint8_t *data, size_t size)
{
    uint8_t buffer[size + 1];
    buffer[0] = 0x02;
    memcpy(buffer + 1, data, size);

    EC_POINT *point = EC_POINT_new(group);
    if (EC_POINT_oct2point(group, point, buffer, sizeof(buffer), NULL) == 1) {
        return point;
    } else {
        EC_POINT_clear_free(point);
        return NULL;
    }
}

// hex string to uint8_t[]
// https://stackoverflow.com/questions/3408706/hexadecimal-string-to-byte-array-in-c
void str2arr(uint8_t* val, char* string, size_t len) {
    // const char hexstring[] = "00e3d3789271e630673c1098e76700c413b0ee9ad52b6ae1715c1e8d2eea9b2de9", *pos = hexstring;
    char *hexstring = string;
    char *pos = hexstring;
    // unsigned char val[33];

     /* WARNING: no sanitization or error-checking whatsoever */
    for (size_t count = 0; count < len; count++) {
        sscanf(pos, "%2hhx", &val[count]);
        pos += 2;
    }
}

/**
 * Convert hash value to an EC point.
 *
 * This implementation will work for any curve but execution is not time-constant.
 *
 * @return EC point or NULL in case of failure.
 */
static EC_POINT *ECVRF_hash_to_curve1(const ecvrf_suite *vrf, const EC_POINT *pubkey, const uint8_t *data, size_t size)
{
    int degree = bits_in_bytes(EC_GROUP_get_degree(vrf->group));
    uint8_t _pubkey[degree + 1];
    if (EC_POINT_point2oct(vrf->group, pubkey, POINT_CONVERSION_COMPRESSED, _pubkey, sizeof(_pubkey), NULL) != sizeof(_pubkey)) {
        return NULL;
    }

    EC_POINT *result = NULL;

    EVP_MD_CTX *md_template = EVP_MD_CTX_new();
    if (!md_template) {
        return NULL;
    }
    EVP_DigestInit_ex(md_template, vrf->hash, NULL);
    EVP_DigestUpdate(md_template, _pubkey, sizeof(_pubkey));
    EVP_DigestUpdate(md_template, data, size);

    EVP_MD_CTX *md = EVP_MD_CTX_new();
    if (!md) {
        EVP_MD_CTX_free(md_template);
        return NULL;
    }

    for (uint32_t _counter = 0; result == NULL || EC_POINT_is_at_infinity(vrf->group, result); _counter++) {
        assert(_counter < 256); // hard limit for debugging
        uint32_t counter = htonl(_counter);
        static_assert(sizeof(counter) == 4, "counter is 4-byte");

        uint8_t hash[EVP_MAX_MD_SIZE] = {0};
        unsigned hash_size = sizeof(hash);

        EVP_DigestInit_ex(md, vrf->hash, NULL);
        EVP_MD_CTX_copy_ex(md, md_template);
        EVP_DigestUpdate(md, &counter, sizeof(counter));
        if (EVP_DigestFinal_ex(md, hash, &hash_size) != 1) {
            EC_POINT_clear_free(result);
            result = NULL;
            break;
        }

        // perform multiplication with cofactor if cofactor is > 1
        const BIGNUM *cofactor = EC_GROUP_get0_cofactor(vrf->group);
        assert(cofactor);
        result = RS2ECP(vrf->group, hash, hash_size);
        if (result != NULL && !BN_is_one(cofactor)) {
            EC_POINT *tmp = EC_POINT_new(vrf->group);
            if (EC_POINT_mul(vrf->group, tmp, NULL, result, cofactor, NULL) != 1) {
                EC_POINT_clear_free(tmp);
                EC_POINT_clear_free(result);
                result = NULL;
                break;
            }
            EC_POINT_clear_free(result);
            result = tmp;
        }
    }

    EVP_MD_CTX_free(md);
    EVP_MD_CTX_free(md_template);

    return result;
}

/**
 * Hash several EC points into an unsigned integer.
 */
static BIGNUM *ECVRF_hash_points(const ecvrf_suite *vrf, const EC_POINT **points, size_t count)
{
    BIGNUM *result = NULL;

    EVP_MD_CTX *md = EVP_MD_CTX_new();
    if (!md) {
        return NULL;
    }
    EVP_DigestInit_ex(md, vrf->hash, NULL);

    do {
        for (size_t i = 0; i < count; i++) {
            uint8_t buffer[vrf->ecp_size];
            if (EC_POINT_point2oct(vrf->group, points[i], POINT_CONVERSION_COMPRESSED, buffer, sizeof(buffer), NULL) != sizeof(buffer)) {
                break;
            }
            EVP_DigestUpdate(md, buffer, sizeof(buffer));
        }

        uint8_t hash[EVP_MAX_MD_SIZE] = {0};
        unsigned hash_size = sizeof(hash);
        if (EVP_DigestFinal_ex(md, hash, &hash_size) != 1) {
            break;
        }

        assert(hash_size >= vrf->c_size);
        result = BN_bin2bn(hash, vrf->c_size, NULL);
    } while (false);

 // fail:
    EVP_MD_CTX_free(md);

    return result;
}

/**
 * ECVRF proof decoding.
 */
static bool ECVRF_decode_proof(
    const ecvrf_suite *vrf, const uint8_t *proof, size_t size,
    EC_POINT **gamma_ptr, BIGNUM **c_ptr, BIGNUM **s_ptr)
{
    if (size != vrf->proof_size) {
        return false;
    }

    assert(vrf->ecp_size + vrf->c_size + vrf->s_size == size);

    const uint8_t *gamma_raw = proof;
    const uint8_t *c_raw = gamma_raw + vrf->ecp_size;
    const uint8_t *s_raw = c_raw + vrf->c_size;
    assert(s_raw + vrf->s_size == proof + size);

    EC_POINT *gamma = EC_POINT_new(vrf->group);
    if (EC_POINT_oct2point(vrf->group, gamma, gamma_raw, vrf->ecp_size, NULL) != 1) {
        EC_POINT_clear_free(gamma);
        return false;
    }

    BIGNUM *c = BN_bin2bn(c_raw, vrf->c_size, NULL);
    if (!c) {
        EC_POINT_clear_free(gamma);
        return false;
    }

    BIGNUM *s = BN_bin2bn(s_raw, vrf->s_size, NULL);
    if (!s) {
        EC_POINT_clear_free(gamma);
        BN_clear_free(c);
        return false;
    }

    *gamma_ptr = gamma;
    *c_ptr = c;
    *s_ptr = s;

    return true;
}

static bool ECVRF_verify(
    const ecvrf_suite *vrf, const EC_POINT *pubkey,
    const uint8_t *data, size_t size,
    const uint8_t *proof, size_t proof_size)
{
    bool valid = false;

    EC_POINT *gamma = NULL;
    EC_POINT *u = NULL;
    EC_POINT *v = NULL;
    BIGNUM *c = NULL;
    BIGNUM *s = NULL;
    BIGNUM *c2 = NULL;
    do {
        if (!ECVRF_decode_proof(vrf, proof, proof_size, &gamma, &c, &s)) {
            break;
        }

        const EC_POINT *generator = EC_GROUP_get0_generator(vrf->group);
        assert(generator);

        EC_POINT *hash = ECVRF_hash_to_curve1(vrf, pubkey, data, size);
        assert(hash);

        u = ec_mul_two(vrf->group, pubkey, c, generator, s);
        if (!u) {
            break;
        }

        v = ec_mul_two(vrf->group, gamma, c, hash, s);
        if (!u) {
            break;
        }

        const EC_POINT *points[] = {generator, hash, pubkey, gamma, u, v};
        c2 = ECVRF_hash_points(vrf, points, sizeof(points) / sizeof(EC_POINT *));
        if (!c2) {
            break;
        }

        valid = BN_cmp(c, c2) == 0;
    } while (false);
// fail:
    EC_POINT_clear_free(gamma);
    EC_POINT_clear_free(u);
    EC_POINT_clear_free(v);
    BN_clear_free(c);
    BN_clear_free(s);
    BN_clear_free(c2);

    return valid;
}

bool verify_with_pk(char* pk_str, char* proof_str, char* message)
{
    // initialize vrf environment
    ilog("verify_with_pk: pk = ${pk_str}, proof_str = ${proof_str}, message = ${message}",
            ("pk_str", std::string(pk_str, 66))("proof_str", std::string(proof_str, 162))("message", (std::string(message, 64))));

    ecvrf_suite* vrf = ecvrf_p256();
    if (!vrf) {
        ilog("init vrf error");
        return false;
    }

    uint8_t pk_arr[33];
    str2arr(pk_arr, pk_str, sizeof(pk_arr));
    EC_POINT* pubkey = EC_POINT_new(vrf->group);
    if (EC_POINT_oct2point(vrf->group, pubkey, pk_arr, sizeof(pk_arr), NULL) != 1) {
        ilog("failed to create public key");
        return false;
    }

    uint8_t proof[81];
    str2arr(proof, proof_str, vrf->proof_size);

    uint8_t msg_arr[32];
    str2arr(msg_arr, message, sizeof(msg_arr));

    bool valid = ECVRF_verify(vrf, pubkey, msg_arr, sizeof(msg_arr), proof, vrf->proof_size);
    ilog("valid = ${valid}\n", ("valid", valid));

    EC_POINT_clear_free(pubkey);
    ecvfr_free(vrf);
    return valid;
}

} // end of namespace
