#pragma once

#include <string>
#include <fc/reflect/variant.hpp>
#include <fc/reflect/reflect.hpp>
#include "PublicKey.h"
#include "Signature.h"

namespace gm {
    namespace sm2 {
        class PrivateKey {
        public:
            static const size_t kSm2PrivateKeyLength = 32;

            // TODO(xiaofen.qin@gmail.com)
            static PrivateKey regenerate(const fc::sha256& k);

            static PrivateKey generate();

            PrivateKey() = default;

            explicit PrivateKey(const std::string& wifKey);

            PrivateKey(const PrivateKey&) = default;

            PrivateKey(PrivateKey&&) = default;

            PrivateKey&operator = (const PrivateKey& rhs) = default;

            explicit operator std::string() const;

            bool operator == (const PrivateKey& rhs) const;

            void reset(const std::string& wif);

            PublicKey get_public_key() const;

            PublicKey getPublicKey() const;

            bool sign(const char* data, size_t dataSize, Signature& outSignature) const;

            // TODO(xiaofen.qin@gmail.com)
            Signature sign( const fc::sha256& digest, bool require_canonical = true ) const;

        private:
            std::string m_wifKey;

            // see private_key.cpp to_wif
            static std::string toWif(const std::string& hexKey);

            static std::string fromWif(const std::string& wif);

            friend struct fc::reflector<gm::sm2::PrivateKey>;
        };
    }
}

namespace fc {
    void to_variant(const gm::sm2::PrivateKey& var,  fc::variant& vo);

    void from_variant(const fc::variant& var, gm::sm2::PrivateKey& vo);
} // namespace fc

FC_REFLECT(gm::sm2::PrivateKey, (m_wifKey) )
