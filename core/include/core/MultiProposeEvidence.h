#pragma once

#include <core/types.h>

#include "core/Evidence.h"

namespace ultrainio {
    class MultiProposeEvidence : public Evidence {
    public:
        MultiProposeEvidence();

        MultiProposeEvidence(const std::string& str);

        MultiProposeEvidence(const SignedBlockHeader& one, const SignedBlockHeader& other);

        virtual std::string toString() const;

        virtual AccountName getEvilAccount() const;

        virtual int verify(const AccountName& accountName, const PublicKey& pk, const std::string& blsPk) const;

        virtual bool simpleVerify() const;

    private:
        static const std::string kA;

        static const std::string kB;

        SignedBlockHeader m_A;

        SignedBlockHeader m_B;
    };
}
