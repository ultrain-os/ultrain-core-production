#pragma once

#include "core/Evidence.h"
#include "core/Message.h"
#include "core/types.h"

namespace ultrainio {
    class EchoBlsEvidence : public Evidence {
    public:
        EchoBlsEvidence();

        EchoBlsEvidence(const std::string& str);

        EchoBlsEvidence(const EchoMsg& echoMsg);

        virtual std::string toString() const;

        virtual AccountName getEvilAccount() const;

        virtual int verify(const AccountName& accountName, const PublicKey& pk, const std::string& blsPk) const;
    private:
        static const std::string kA;

        EchoMsg m_A;
    };
}
