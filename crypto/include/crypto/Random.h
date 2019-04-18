#pragma once

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>

namespace ultrainio {
    struct ecvrf_suite {
        ecvrf_suite(EC_GROUP* _group, const EVP_MD* _hash, const size_t _proof_size,  const size_t _ecp_size, const size_t _c_size, const size_t _s_size)
                : group(_group), hash(_hash), proof_size(_proof_size), ecp_size(_ecp_size), c_size(_c_size), s_size(_s_size) {}

        ~ecvrf_suite() {
            EC_GROUP_free(group);
        }
        EC_GROUP *group;
        const EVP_MD *hash;
        const size_t proof_size;
        const size_t ecp_size;
        const size_t c_size;
        const size_t s_size;
    };

    bool verify_with_pk(char* pkStr, char* proofStr, char* msgStr);
} // end of namespace