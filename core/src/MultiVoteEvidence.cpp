#include "core/MultiVoteEvidence.h"

#include <fc/variant.hpp>
#include <fc/io/json.hpp>

#include <crypto/Validator.h>

#include "core/SerializedEchoMsg.h"
#include "core/types.h"

namespace ultrainio {
    const std::string MultiVoteEvidence::kA = "a";

    const std::string MultiVoteEvidence::kB = "b";

    MultiVoteEvidence::MultiVoteEvidence() {}

    MultiVoteEvidence::MultiVoteEvidence(const std::string& str) {
        fc::variant v = fc::json::from_string(str);
        fc::variant_object o = v.get_object();
        fc::variant aVar = o[kA];
        fc::variant bVar = o[kB];
        SerializedEchoMsg a;
        SerializedEchoMsg b;
        aVar.as<SerializedEchoMsg>(a);
        bVar.as<SerializedEchoMsg>(b);
        m_A = a.toEchoMsg();
        m_B = b.toEchoMsg();
    }

    MultiVoteEvidence::MultiVoteEvidence(const EchoMsg& one, const EchoMsg& other) : m_A(one), m_B(other) {}

    std::string MultiVoteEvidence::toString() const {
        fc::mutable_variant_object o;
        fc::variant typeVar(Evidence::kMultiVote);
        SerializedEchoMsg a(m_A);
        SerializedEchoMsg b(m_B);
        fc::variant aVar(a);
        fc::variant bVar(b);
        o[kType] = typeVar;
        o[kA] = aVar;
        o[kB] = bVar;
        return fc::json::to_string(fc::variant(o));
    }

    AccountName MultiVoteEvidence::getEvilAccount() const {
        return m_A.account;
    }

    int MultiVoteEvidence::verify(const AccountName& accountName, const consensus::PublicKeyType& pk, const std::string& blsPk) const {
        if (accountName != m_A.account || accountName != m_B.account) {
            return Evidence::kReporterEvil;
        }
        if (m_A.blockNum() == m_B.blockNum()
                && m_A.phase >= kPhaseBA1
                && m_A.phase == m_B.phase
                && m_A.baxCount == m_B.baxCount
                && m_A.account == m_B.account
                && m_A.blockId != m_B.blockId
                && Validator::verify<UnsignedEchoMsg>(consensus::SignatureType(m_A.signature), m_A, pk)
                && Validator::verify<UnsignedEchoMsg>(consensus::SignatureType(m_A.signature), m_A, pk)) {
            return Evidence::kMultiVote;
        }
        return Evidence::kNone;
    }

    bool MultiVoteEvidence::simpleVerify() const {
        if (m_A.blockNum() == m_B.blockNum()
            && m_A.phase >= kPhaseBA1
            && m_A.phase == m_B.phase
            && m_A.baxCount == m_B.baxCount
            && m_A.account == m_B.account
            && m_A.blockId != m_B.blockId) {
            return true;
        }
        return false;
    }
}
