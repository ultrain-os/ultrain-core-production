#pragma once

#include <core/types.h>

#include "core/Evidence.h"

namespace ultrainio {
    class MultiSignEvidence : public Evidence {
    public:
        MultiSignEvidence(const std::string& str);

        MultiSignEvidence(const SignedBlockHeader& one, const SignedBlockHeader& other);

        virtual std::string toString() const;

        virtual AccountName getEvilAccount() const;

        virtual bool verify(const PublicKey& pk);

        virtual int type() const;

    private:
        static const std::string kA;

        static const std::string kB;

        SignedBlockHeader m_A;

        SignedBlockHeader m_B;
    };
}
