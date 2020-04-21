#include "core/EchoBlsEvidence.h"

#include <fc/variant.hpp>
#include <fc/io/json.hpp>

#include <crypto/Validator.h>

#include "core/SerializedEchoMsg.h"

namespace ultrainio {
    const std::string EchoBlsEvidence::kA = "a";

    EchoBlsEvidence::EchoBlsEvidence() {}

    EchoBlsEvidence::EchoBlsEvidence(const EchoMsg& echoMsg) : m_A(echoMsg) {}

    EchoBlsEvidence::EchoBlsEvidence(const std::string& str) {
        fc::variant v = fc::json::from_string(str);
        fc::variant_object o = v.get_object();
        fc::variant aVar = o[kA];
        SerializedEchoMsg a;
        aVar.as<SerializedEchoMsg>(a);
        m_A = a.toEchoMsg();
    }

    std::string EchoBlsEvidence::toString() const {
        fc::mutable_variant_object o;
        fc::variant typeVar(Evidence::kEchoBls);
        SerializedEchoMsg a(m_A);
        fc::variant aVar(a);
        o[kType] = typeVar;
        o[kA] = aVar;
        return fc::json::to_string(fc::variant(o));
    }

    AccountName EchoBlsEvidence::getEvilAccount() const {
        return m_A.account;
    }

    int EchoBlsEvidence::verify(const AccountName& accountName, const consensus::PublicKeyType& pk, const std::string& blsPkStr) const {
        if (accountName == m_A.account) {
            return Evidence::kReporterEvil;
        }
        unsigned char blsPk[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
        Hex::fromHex(blsPkStr, blsPk, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        if (Validator::verify<UnsignedEchoMsg>(consensus::SignatureType(m_A.signature), m_A, pk) &&
                !Validator::verify<CommonEchoMsg>(m_A.blsSignature, m_A, blsPk)) {
            return Evidence::kEchoBls;
        }
        return Evidence::kNone;
    }
}
