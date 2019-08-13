#pragma once

#include <core/types.h>

#include "rpos/Evidence.h"

namespace ultrainio {
    class EvidenceMultiSign : public Evidence {
    public:
        EvidenceMultiSign(const std::string& str);

        EvidenceMultiSign(const AccountName& acc, const SignedBlockHeader& one, const SignedBlockHeader& other);

        std::string toString() const;

    private:
        static const std::string kA;

        static const std::string kB;

        static const std::string kAccount;

        AccountName mAccount;

        SignedBlockHeader mA;

        SignedBlockHeader mB;
    };
}
