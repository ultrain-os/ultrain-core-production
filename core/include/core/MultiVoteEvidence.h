#pragma once

#include "core/Evidence.h"
#include "core/Message.h"

namespace ultrainio {
    class MultiVoteEvidence : public Evidence {
    public:
        MultiVoteEvidence();

        MultiVoteEvidence(const std::string& str);

        MultiVoteEvidence(const EchoMsg& one, const EchoMsg& other);

        virtual std::string toString() const;

        virtual AccountName getEvilAccount() const;

        virtual int verify(const AccountName& accountName, const consensus::PublicKeyType& pk, const std::string& blsPk) const;

        virtual bool simpleVerify() const;

    private:
        static const std::string kA;

        static const std::string kB;

        EchoMsg m_A;

        EchoMsg m_B;
    };
}