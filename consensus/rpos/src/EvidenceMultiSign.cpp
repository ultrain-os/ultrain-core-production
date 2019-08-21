#include "rpos/EvidenceMultiSign.h"

#include <fc/variant.hpp>
#include <fc/io/json.hpp>

namespace ultrainio {
    const std::string EvidenceMultiSign::kA = "a";

    const std::string EvidenceMultiSign::kB = "b";

    const std::string EvidenceMultiSign::kAccount = "account";

    EvidenceMultiSign::EvidenceMultiSign(const std::string& str) {
        fc::variant v = fc::json::from_string(str);
        fc::variant_object o = v.get_object();
        ULTRAIN_ASSERT(o[kType].as_int64() == EvidenceMultiSign::kSignMultiPropose, chain::chain_exception, "type error expect kSignMultiPropose while ${actual} actually", ("actual", o[kType].as_int64()));
        fc::variant accountVar = o[kAccount];
        fc::variant aVar = o[kA];
        fc::variant bVar = o[kB];
        accountVar.as<AccountName>(m_Account);
        aVar.as<SignedBlockHeader>(m_A);
        bVar.as<SignedBlockHeader>(m_B);
    }

    EvidenceMultiSign::EvidenceMultiSign(const AccountName& acc, const SignedBlockHeader& one, const SignedBlockHeader& other)
            : m_Account(acc), m_A(one), m_B(other) {
    }

    std::string EvidenceMultiSign::toString() const {
        fc::mutable_variant_object o;
        fc::variant typeVar(Evidence::kSignMultiPropose);
        fc::variant accountVar(m_Account.to_string());
        fc::variant aVar(m_A);
        fc::variant bVar(m_B);
        o[kType] = typeVar;
        o[kAccount] = accountVar;
        o[kA] = aVar;
        o[kB] = bVar;
        return fc::json::to_string(fc::variant(o));
    }
}
