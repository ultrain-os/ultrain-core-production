#include "gm/sm2/Signature.h"

#include <fc/crypto/ripemd160.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/exception/exception.hpp>
#include <base/Hex.h>

namespace gm {
    namespace sm2 {
        const std::string Signature::kSignaturePrefix = "SIG_GM_";

        Signature::Signature(const std::string& base58Str) : m_binSig(base2bin(base58Str)) {}

        Signature::Signature(const Signature::DataType& binSig) : m_binSig(binSig) {}

        Signature::operator std::string() const {
            return bin2base58(m_binSig);
        }

        void Signature::reset(const std::string& r, const std::string& s) {
            m_binSig = rs2bin(r, s);
        }

        std::string Signature::r() const {
            return bin2r(m_binSig);
        }

        std::string Signature::s() const {
            return bin2s(m_binSig);
        }

        bool operator == ( const Signature& p1, const Signature& p2) {
            if (&p1 == &p2) {
                return true;
            }
            return p1.m_binSig.get<Signature::SignatureData>() == p2.m_binSig.get<Signature::SignatureData>();
        }

        bool operator != ( const Signature& p1, const Signature& p2) {
            return !(p1 == p2);
        }

        bool operator < ( const Signature& p1, const Signature& p2) {
            return p1.m_binSig.get<Signature::SignatureData>() < p2.m_binSig.get<Signature::SignatureData>();
        }

        std::size_t hash_value(const gm::sm2::Signature& b) {
            static_assert(sizeof(b.m_binSig.get<Signature::SignatureData>().data) == Signature::kSignatureLength + 1, "sig size is expected to be 65");
            return *(size_t*)&b.m_binSig.get<Signature::SignatureData>().data[32-sizeof(size_t)] + *(size_t*)&b.m_binSig.get<Signature::SignatureData>().data[32-sizeof(size_t)];
        }

        std::string Signature::bin2base58(const Signature::DataType& bin) {
            size_t sizeOfHashBytes = 4;
            size_t sizeOfDataToHash = 1 + Signature::kSignatureLength;
            unsigned char bytes[sizeOfDataToHash + sizeOfHashBytes];
            memcpy(bytes, bin.get<Signature::SignatureData>().begin(), sizeOfDataToHash);
            uint32_t checksum = calcChecksum((char*)bytes, sizeOfDataToHash);
            memcpy(bytes + sizeOfDataToHash, (char*)&checksum, sizeOfHashBytes);
            return kSignaturePrefix + fc::to_base58((char*)bytes, sizeOfDataToHash + sizeOfHashBytes);
        }

        Signature::DataType Signature::base2bin(const std::string& base58Str) {
            auto index = base58Str.find(Signature::kSignaturePrefix);
            FC_ASSERT(index != std::string::npos, "signature $(s) not start with $(prefix)", ("s", base58Str)("prefix", Signature::kSignaturePrefix));
            std::string realSigBase58 = base58Str.substr(Signature::kSignaturePrefix.length());
            FC_ASSERT(realSigBase58.length() > 64, "signature: ${s} less than 64", ("s", realSigBase58));
            auto bin = fc::from_base58(realSigBase58);
            // do checksum
            size_t sizeOfDataToHash = 1 + Signature::kSignatureLength;
            size_t sizeOfHashBytes = 4;
            uint32_t checksum = calcChecksum(bin.data(), sizeOfDataToHash);
            FC_ASSERT(memcmp((char*)&checksum, bin.data() + sizeOfDataToHash, sizeOfHashBytes) == 0, "base58Str ${base58Str} to signature checksum error", ("base58Str", base58Str));
            Signature::SignatureData data;
            memcpy(data.begin(), bin.data(), sizeOfDataToHash);
            return Signature::DataType(data);
        }

        std::string Signature::bin2r(const Signature::DataType& bin) {
            return ultrainio::Hex::toHex<unsigned char>(bin.get<Signature::SignatureData>().begin() + 1, Signature::kRLength, false);
        }

        std::string Signature::bin2s(const Signature::DataType& bin) {
            return ultrainio::Hex::toHex<unsigned char>(bin.get<Signature::SignatureData>().begin() + 1 + Signature::kRLength, Signature::kSLength, false);
        }

        Signature::DataType Signature::rs2bin(const std::string& r, const std::string& s) {
            return hex2bin(r + s);
        }

        Signature::DataType Signature::hex2bin(const std::string& hexSig) {
            FC_ASSERT(hexSig.length() == 2 * Signature::kSignatureLength, "hexSig: ${hexSig} length < 64", ("hexSig", hexSig));
            Signature::SignatureData data;
            data.begin()[0] = 33;
            ultrainio::Hex::fromHex<unsigned char>(hexSig, data.begin() + 1, Signature::kSignatureLength);
            return Signature::DataType(data);
        }

        uint32_t Signature::calcChecksum(const char* data, size_t dataLen) {
            auto encoder = fc::ripemd160::encoder();
            const char* GM = "GM";
            encoder.write(data, dataLen);
            encoder.write(GM, strlen(GM));
            return encoder.result()._hash[0];
        }
    }
}

namespace fc {
    void to_variant(const gm::sm2::Signature& var,  variant& vo) {
        vo = string(var);
    }

    void from_variant(const variant& var, gm::sm2::Signature& vo) {
        vo = gm::sm2::Signature(var.as_string());
    }
} // namespace fc
