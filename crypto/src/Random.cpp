#include <crypto/Random.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <arpa/inet.h>

#include <fc/exception/exception.hpp>

#include <base/Hex.h>

namespace ultrainio {
/**
 * Get EC-VRF-P256-SHA256 implementation.
 */
static std::shared_ptr<ecvrf_suite> ecvrf_p256()
{
    EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    if (!group) {
        return nullptr;
    }
    std::shared_ptr<ecvrf_suite> suite = std::make_shared<ecvrf_suite>(group, EVP_sha256(), 81, 33, 16, 32);
    if (!suite) {
        EC_GROUP_free(group);
    }
    return suite;
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
        return nullptr;
    }

    const EC_POINT *points[] = { p1, p2 };
    const BIGNUM *factors[] = { f1, f2 };
    if (EC_POINTs_mul(group, result, nullptr, 2, points, factors, nullptr) != 1) {
        EC_POINT_clear_free(result);
        return nullptr;
    }

    return result;
}

/**
 * Try converting random string to EC point.
 *
 * @return EC point or nullptr if the random string cannot be interpreted as an EC point.
 */
static EC_POINT *RS2ECP(const EC_GROUP *group, const uint8_t *data, size_t size)
{
    uint8_t buffer[size + 1];
    buffer[0] = 0x02;
    memcpy(buffer + 1, data, size);

    EC_POINT *point = EC_POINT_new(group);
    if (EC_POINT_oct2point(group, point, buffer, sizeof(buffer), nullptr) == 1) {
        return point;
    } else {
        EC_POINT_clear_free(point);
        return nullptr;
    }
}

/**
 * Convert hash value to an EC point.
 *
 * This implementation will work for any curve but execution is not time-constant.
 *
 * @return EC point or nullptr in case of failure.
 */
static EC_POINT *ECVRF_hash_to_curve1(const ecvrf_suite *vrf, const EC_POINT *pubkey, const uint8_t *data, size_t size)
{
    int degree = bits_in_bytes(EC_GROUP_get_degree(vrf->group));
    uint8_t _pubkey[degree + 1];
    if (EC_POINT_point2oct(vrf->group, pubkey, POINT_CONVERSION_COMPRESSED, _pubkey, sizeof(_pubkey), nullptr) != sizeof(_pubkey)) {
        return nullptr;
    }

    EC_POINT *result = nullptr;

    EVP_MD_CTX *md_template = EVP_MD_CTX_create();
    if (!md_template) {
        return nullptr;
    }
    EVP_DigestInit_ex(md_template, vrf->hash, nullptr);
    EVP_DigestUpdate(md_template, _pubkey, sizeof(_pubkey));
    EVP_DigestUpdate(md_template, data, size);

    EVP_MD_CTX *md = EVP_MD_CTX_create();
    if (!md) {
        EVP_MD_CTX_destroy(md_template);
        return nullptr;
    }

    for (uint32_t _counter = 0; result == nullptr || EC_POINT_is_at_infinity(vrf->group, result); _counter++) {
        assert(_counter < 256); // hard limit for debugging
        uint32_t counter = htonl(_counter);
        static_assert(sizeof(counter) == 4, "counter is 4-byte");

        uint8_t hash[EVP_MAX_MD_SIZE] = {0};
        unsigned hash_size = sizeof(hash);

        EVP_DigestInit_ex(md, vrf->hash, nullptr);
        EVP_MD_CTX_copy_ex(md, md_template);
        EVP_DigestUpdate(md, &counter, sizeof(counter));
        if (EVP_DigestFinal_ex(md, hash, &hash_size) != 1) {
            EC_POINT_clear_free(result);
            result = nullptr;
            break;
        }

        // perform multiplication with cofactor if cofactor is > 1
        BIGNUM *cofactor=BN_new();
        EC_GROUP_get_cofactor(vrf->group,cofactor,nullptr);
        assert(cofactor);
        result = RS2ECP(vrf->group, hash, hash_size);
        if (result != nullptr && !BN_is_one(cofactor)) {
            EC_POINT *tmp = EC_POINT_new(vrf->group);
            if (EC_POINT_mul(vrf->group, tmp, nullptr, result, cofactor, nullptr) != 1) {
                EC_POINT_clear_free(tmp);
                EC_POINT_clear_free(result);
                result = nullptr;
                break;
            }
            EC_POINT_clear_free(result);
            result = tmp;
        }
        BN_free(cofactor);
    }

    EVP_MD_CTX_destroy(md);
    EVP_MD_CTX_destroy(md_template);

    return result;
}

/**
 * Hash several EC points into an unsigned integer.
 */
static BIGNUM *ECVRF_hash_points(const ecvrf_suite *vrf, const EC_POINT **points, size_t count)
{
    BIGNUM *result = nullptr;
    uint8_t hash[EVP_MAX_MD_SIZE] = {0};
    unsigned hash_size = sizeof(hash);

    EVP_MD_CTX *md = EVP_MD_CTX_create();
    if (!md) {
        return nullptr;
    }
    EVP_DigestInit_ex(md, vrf->hash, nullptr);

    for (size_t i = 0; i < count; i++) {
        uint8_t buffer[vrf->ecp_size];
        if (EC_POINT_point2oct(vrf->group, points[i], POINT_CONVERSION_COMPRESSED, buffer, sizeof(buffer), nullptr) != sizeof(buffer)) {
            goto fail;
        }
        EVP_DigestUpdate(md, buffer, sizeof(buffer));
    }

    if (EVP_DigestFinal_ex(md, hash, &hash_size) != 1) {
        goto fail;
    }

    assert(hash_size >= vrf->c_size);
    result = BN_bin2bn(hash, vrf->c_size, nullptr);
fail:
    EVP_MD_CTX_destroy(md);

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
    if (EC_POINT_oct2point(vrf->group, gamma, gamma_raw, vrf->ecp_size, nullptr) != 1) {
        EC_POINT_clear_free(gamma);
        return false;
    }

    BIGNUM *c = BN_bin2bn(c_raw, vrf->c_size, nullptr);
    if (!c) {
        EC_POINT_clear_free(gamma);
        return false;
    }

    BIGNUM *s = BN_bin2bn(s_raw, vrf->s_size, nullptr);
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

    EC_POINT *gamma = nullptr;
    EC_POINT *u = nullptr;
    EC_POINT *v = nullptr;
    BIGNUM *c = nullptr;
    BIGNUM *s = nullptr;
    BIGNUM *c2 = nullptr;
    if (!ECVRF_decode_proof(vrf, proof, proof_size, &gamma, &c, &s)) {
        goto fail;
    } else {
        const EC_POINT *generator = EC_GROUP_get0_generator(vrf->group);
        assert(generator);

        EC_POINT *hash = ECVRF_hash_to_curve1(vrf, pubkey, data, size);
        assert(hash);

        u = ec_mul_two(vrf->group, pubkey, c, generator, s);
        if (!u) {
            goto fail;
        } else {
            v = ec_mul_two(vrf->group, gamma, c, hash, s);
            if (!v) {
                goto fail;
            } else {
                const EC_POINT *points[] = {generator, hash, pubkey, gamma, u, v};
                c2 = ECVRF_hash_points(vrf, points, sizeof(points) / sizeof(EC_POINT *));
                if (!c2) {
                    goto fail;
                }
            }
        }
        EC_POINT_clear_free(hash);
    }
    valid = BN_cmp(c, c2) == 0;

fail:
    EC_POINT_clear_free(gamma);
    EC_POINT_clear_free(u);
    EC_POINT_clear_free(v);
    BN_clear_free(c);
    BN_clear_free(s);
    BN_clear_free(c2);

    return valid;
}

bool verify_with_pk(char* pkStr, char* proofStr, char* msgStr)
{
    // initialize vrf environment
    //ilog("verify_with_pk: pk = ${pk_str}, proof_str = ${proof_str}, message = ${message}",
         //("pk_str", std::string(pk_str, 66))("proof_str", std::string(proof_str, 162))("message", (std::string(message, 64))));

    std::shared_ptr<ecvrf_suite> vrf = ecvrf_p256();
    if (!vrf) {
        ilog("init vrf error");
        return false;
    }

    uint8_t pk[vrf->ecp_size];
    Hex::fromHex<uint8_t>(std::string(pkStr, vrf->ecp_size * 2), pk, vrf->ecp_size);

    EC_POINT* pubKey = EC_POINT_new(vrf->group);
    if (EC_POINT_oct2point(vrf->group, pubKey, pk, vrf->ecp_size, nullptr) != 1) {
        ilog("failed to create public key");
        EC_POINT_clear_free(pubKey);
        return false;
    }

    uint8_t proof[vrf->proof_size];
    Hex::fromHex<uint8_t>(std::string(proofStr, vrf->proof_size * 2), proof, vrf->proof_size);

    uint8_t msg[vrf->s_size];
    Hex::fromHex<uint8_t>(std::string(msgStr, vrf->s_size * 2), msg, vrf->s_size);

    bool valid = ECVRF_verify(vrf.get(), pubKey, msg, vrf->s_size, proof, vrf->proof_size);
    //ilog("valid = ${valid} vrf proof_size : ${size} pkSize : ${pkSize} msgSize : ${msgSize}\n", ("valid", valid)("size", vrf->proof_size)("pkSize", vrf->ecp_size)("msgSize", vrf->s_size));

    EC_POINT_clear_free(pubKey);
    return valid;
}

} // end of namespace
