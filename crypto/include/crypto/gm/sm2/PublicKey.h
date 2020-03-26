#pragma once

#include <string>
#include <fc/crypto/sha256.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/static_variant.hpp>
#include "gm/sm2/Signature.h"

namespace gm {
    namespace sm2 {
        class PublicKey {
        public:
            static const size_t kSm2PublicKeyCompressedLength = 33;
            using PublicKeyData = fc::array<char, kSm2PublicKeyCompressedLength>;
            using DataType = fc::static_variant<PublicKeyData>;

            static const char* UTR;

            PublicKey() = default ;

            PublicKey(const PublicKey&) = default;

            PublicKey(PublicKey&&) = default;

            explicit PublicKey(const std::string& wifKey);

            PublicKey& operator = (const PublicKey& rhs) = default;

            explicit operator std::string() const;

            void reset(const std::string& hexKey);

            bool valid() const;

            bool verify(const char* data, size_t dataSize, const Signature& signature) const;

            static std::string base58_to_hex(const std::string& base58str);

        private:

            static DataType wif2Bin(const std::string& wif);

            static DataType hex2Bin(const std::string& hexKey);

            static std::string bin2Hex(const DataType& data);

            static std::string bin2Wif(const DataType& data);

            static std::string hex2Wif(const std::string& hexKey);

            static uint32_t calcChecksum(const char* data, size_t dataLen);

            DataType m_binKey;

            // TODO(xiaofen.qin@gmail.com)
            friend bool operator < ( const PublicKey& p1, const PublicKey& p2);
            friend bool operator != ( const PublicKey& p1, const PublicKey& p2);
            friend bool operator == (const PublicKey& p1, const PublicKey& p2);

            friend struct fc::reflector<gm::sm2::PublicKey>;

            friend class PrivateKey;
        };
    }
}

namespace fc {
    void to_variant(const gm::sm2::PublicKey& var,  variant& vo);

    void from_variant(const variant& var, gm::sm2::PublicKey& vo);
} // namespace fc

FC_REFLECT(gm::sm2::PublicKey, (m_binKey) )
