#pragma once

#include <string>

#include <crypto/PublicKey.h>

#include "core/types.h"

namespace ultrainio {
    class Evidence {
    public:
        static const std::string kType;

        static const int kReporterEvil;

        static const int kNone;

        static const int kSignMultiPropose; // send multi-propose message

        static const int kVoteMultiPropose; // eg. ba0 vote multi-propose

        virtual ~Evidence();

        virtual bool isNull() const;

        virtual std::string toString() const;

        virtual AccountName getEvilAccount() const;

        virtual int verify(const AccountName& accountName, const PublicKey& pk);
    };
}
