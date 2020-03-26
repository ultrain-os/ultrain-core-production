#pragma once

#include <string>
#include <openssl/evp.h>

namespace gm {
    namespace sm3 {
        class Digest {
        public:
            static const int kDigestMaxLen;

            static bool sm3Hash(const unsigned char* msg, size_t msgLen, unsigned char* hash, unsigned int* hashLen);

            static bool sm3Hash(const unsigned char* msg, size_t msgLen, Digest& outDigest);

            Digest();

            Digest(const std::string& digest);

            operator std::string() const;

            void reset(const std::string& digest);

        private:
            std::string m_digest;
        };
    }
}
