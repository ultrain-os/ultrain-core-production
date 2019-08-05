#pragma once

#include <core/types.h>

#include "rpos/EvilType.h"

namespace ultrainio {
    class EvidenceMultiSign {
    public:
        EvidenceMultiSign(const std::string& str);

        EvidenceMultiSign(const AccountName& acc, const SignedBlockHeader& one, const SignedBlockHeader& other);

        std::string toString() const;

    private:
        static const std::string kA;

        static const std::string kB;

        static const std::string kAccount;

        static const std::string kType;

        int mType;

        AccountName mAccount;

        SignedBlockHeader mA;

        SignedBlockHeader mB;
    };
}
