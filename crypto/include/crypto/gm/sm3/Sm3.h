#pragma once

#include <string>
#include <openssl/evp.h>
#include <fc/reflect/reflect.hpp>

namespace gm {
    namespace sm3 {
        class Sm3 {
        public:
            static Sm3 hash(const uint8_t* msg, size_t msgLen);

            Sm3();

            operator std::string() const;

            uint8_t* data();

            const uint8_t* data() const;

            int dataSize() const;

        private:
            static const int kHashMaxLen = EVP_MAX_MD_SIZE;

            static bool hash(const uint8_t* msg, size_t msgLen, uint8_t* hash, unsigned int* hashLen);

            unsigned int m_size = 0;

            uint8_t m_hash[kHashMaxLen];

            friend bool operator == (const gm::sm3::Sm3& lhs, const gm::sm3::Sm3& rhs);

            friend struct fc::reflector<gm::sm3::Sm3>;
        };
    }
}

FC_REFLECT(gm::sm3::Sm3, (m_size)(m_hash) )
