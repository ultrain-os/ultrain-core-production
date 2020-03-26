#pragma once

#include <string>
#include <fc/array.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/static_variant.hpp>

namespace gm {
    namespace sm2 {
        class Signature {
        public:
            static const size_t kSignatureLength = 64;
            using SignatureData =  fc::array<unsigned char, kSignatureLength + 1>; // 65 Bytes
            using DataType = fc::static_variant<SignatureData>;

            static const size_t kRLength = 32;

            static const size_t kSLength = 32;

            static const std::string kSignaturePrefix;

            Signature() = default;

            Signature(const Signature& ) = default;

            Signature(Signature&& ) = default;

            explicit Signature(const std::string& base58Str);

            explicit Signature(const Signature::DataType& binSig);

            Signature& operator = (const Signature& rhs) = default;

            explicit operator std::string() const;

            void reset(const std::string& r, const std::string& s);

            std::string r() const;

            std::string s() const;

        private:

            static std::string bin2base58(const Signature::DataType& bin);

            static Signature::DataType base2bin(const std::string& base58Str);

            static std::string bin2r(const Signature::DataType& bin);

            static std::string bin2s(const Signature::DataType& bin);

            static Signature::DataType rs2bin(const std::string& r, const std::string& s);

            static Signature::DataType hex2bin(const std::string& hexSig);

            static uint32_t calcChecksum(const char* data, size_t dataLen);

            Signature::DataType m_binSig;

            friend std::size_t hash_value(const gm::sm2::Signature& b);

            friend bool operator == ( const Signature& p1, const Signature& p2);
            friend bool operator != ( const Signature& p1, const Signature& p2);
            friend bool operator < ( const Signature& p1, const Signature& p2);

            friend struct fc::reflector<gm::sm2::Signature>;
        };
    }
}

namespace fc {
    void to_variant(const gm::sm2::Signature& var,  variant& vo);

    void from_variant(const variant& var, gm::sm2::Signature& vo);
} // namespace fc

FC_REFLECT(gm::sm2::Signature, (m_binSig) )
