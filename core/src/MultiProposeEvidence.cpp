#include "core/MultiProposeEvidence.h"

#include <fc/variant.hpp>
#include <fc/io/json.hpp>

#include <crypto/Signature.h>
#include <crypto/Validator.h>

namespace ultrainio {
    const std::string MultiProposeEvidence::kA = "a";

    const std::string MultiProposeEvidence::kB = "b";

    MultiProposeEvidence::MultiProposeEvidence() {}

    MultiProposeEvidence::MultiProposeEvidence(const std::string& str) {
        fc::variant v = fc::json::from_string(str);
        fc::variant_object o = v.get_object();
        fc::variant aVar = o[kA];
        fc::variant bVar = o[kB];
        aVar.as<SignedBlockHeader>(m_A);
        bVar.as<SignedBlockHeader>(m_B);
    }

    MultiProposeEvidence::MultiProposeEvidence(const SignedBlockHeader& one, const SignedBlockHeader& other)
            : m_A(one), m_B(other) {
    }

    std::string MultiProposeEvidence::toString() const {
        fc::mutable_variant_object o;
        fc::variant typeVar(Evidence::kMultiPropose);
        fc::variant aVar(m_A);
        fc::variant bVar(m_B);
        o[kType] = typeVar;
        o[kA] = aVar;
        o[kB] = bVar;
        return fc::json::to_string(fc::variant(o));
    }

    AccountName MultiProposeEvidence::getEvilAccount() const {
        return m_A.proposer;
    }

    int MultiProposeEvidence::verify(const AccountName& accountName, const PublicKey& pk) const {
        if (accountName != m_B.proposer || accountName != m_A.proposer) {
            return Evidence::kReporterEvil;
        }
        if (m_A.proposer == m_B.proposer
                && m_A.block_num() == m_B.block_num()
                && m_A.previous == m_B.previous
                && m_A.id() != m_B.id()
                && Validator::verify<BlockHeader>(Signature(m_A.signature), m_A, pk)
                && Validator::verify<BlockHeader>(Signature(m_B.signature), m_B, pk)) {
            return Evidence::kMultiPropose;
        }
        return Evidence::kNone;
    }

    bool MultiProposeEvidence::simpleVerify() const {
        if (m_A.proposer == m_B.proposer
            && m_A.block_num() == m_B.block_num()
            && m_A.previous == m_B.previous
            && m_A.id() != m_B.id()) {
            return true;
        }
        return false;
    }
}
