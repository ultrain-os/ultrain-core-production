#include "core/MultiSignEvidence.h"

#include <fc/variant.hpp>
#include <fc/io/json.hpp>

#include <crypto/Signature.h>
#include <crypto/Validator.h>

namespace ultrainio {
    const std::string MultiSignEvidence::kA = "a";

    const std::string MultiSignEvidence::kB = "b";

    MultiSignEvidence::MultiSignEvidence(const std::string& str) {
        fc::variant v = fc::json::from_string(str);
        fc::variant_object o = v.get_object();
        ULTRAIN_ASSERT(o[kType].as_int64() == Evidence::kSignMultiPropose, chain::chain_exception, "type error expect kSignMultiPropose while ${actual} actually", ("actual", o[kType].as_int64()));
        fc::variant aVar = o[kA];
        fc::variant bVar = o[kB];
        aVar.as<SignedBlockHeader>(m_A);
        bVar.as<SignedBlockHeader>(m_B);
    }

    MultiSignEvidence::MultiSignEvidence(const SignedBlockHeader& one, const SignedBlockHeader& other)
            : m_A(one), m_B(other) {
    }

    std::string MultiSignEvidence::toString() const {
        fc::mutable_variant_object o;
        fc::variant typeVar(Evidence::kSignMultiPropose);
        fc::variant aVar(m_A);
        fc::variant bVar(m_B);
        o[kType] = typeVar;
        o[kA] = aVar;
        o[kB] = bVar;
        return fc::json::to_string(fc::variant(o));
    }

    AccountName MultiSignEvidence::getEvilAccount() const {
        return m_A.proposer;
    }

    bool MultiSignEvidence::verify(const PublicKey& pk) {
        if (m_A.proposer == m_B.proposer
                && m_A.block_num() == m_B.block_num()
                && m_A.id() != m_B.id()
                && Validator::verify<BlockHeader>(Signature(m_A.signature), m_A, pk)
                && Validator::verify<BlockHeader>(Signature(m_B.signature), m_B, pk)) {
            return true;
        }
        return false;
    }

    int MultiSignEvidence::type() const {
        return Evidence::kSignMultiPropose;
    }
}
