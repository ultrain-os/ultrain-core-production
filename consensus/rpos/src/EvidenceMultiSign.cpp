#include "rpos/EvidenceMultiSign.h"

#include <fc/variant.hpp>
#include <fc/io/json.hpp>

namespace ultrainio {
    const std::string EvidenceMultiSign::kA = "a";

    const std::string EvidenceMultiSign::kB = "b";

    const std::string EvidenceMultiSign::kAccount = "account";

    const std::string EvidenceMultiSign::kType = "type";

    EvidenceMultiSign::EvidenceMultiSign(const std::string& str) {
        fc::variant v(str);
        if (!v.is_object()) {
            return;
        }
        fc::variant_object o = v.get_object();
        fc::variant typeVar = o[kType];
        fc::variant accountVar = o[kAccount];
        fc::variant aVar = o[kA];
        fc::variant bVar = o[kB];
        accountVar.as<AccountName>(mAccount);
        aVar.as<SignedBlockHeader>(mA);
        bVar.as<SignedBlockHeader>(mB);
        typeVar.as<int>(mType);
    }

    EvidenceMultiSign::EvidenceMultiSign(const AccountName& acc, const SignedBlockHeader& one, const SignedBlockHeader& other)
            : mType(EvilType::kSignMultiPropose), mAccount(acc), mA(one), mB(other) {
    }

    std::string EvidenceMultiSign::toString() const {
        fc::mutable_variant_object o;
        fc::variant typeVar(mType);
        fc::variant accountVar(mAccount.to_string());
        fc::variant aVar(mA);
        fc::variant bVar(mB);
        o[kType] = typeVar;
        o[kAccount] = accountVar;
        o[kA] = aVar;
        o[kB] = bVar;
        fc::variant v(o);
        return fc::json::to_string(v);
    }
}
