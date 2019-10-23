#pragma once

#include <string>

#include <crypto/PublicKey.h>

#include "core/ExtType.h"
#include "core/types.h"

namespace ultrainio {
    struct EvilDesc {
        EvilDesc(const chain::name& _chainName, const AccountName& _evil, uint32_t _blockNum, const std::vector<ExtType>& _exts = std::vector<ExtType>())
            : chainName(_chainName), evil(_evil), blockNum(_blockNum), exts(_exts) {}
        chain::name chainName;
        AccountName evil;
        uint32_t blockNum;
        std::vector<ExtType> exts;
    };

    class Evidence {
    public:
        static const std::string kType;

        static const int kReporterEvil;

        static const int kNone;

        static const int kMultiPropose; // send multi-propose message

        static const int kMultiVote; // eg. ba0 vote multi-propose

        static const int kEchoBls; // echo.blsSignature

        virtual ~Evidence();

        virtual bool isNull() const;

        virtual std::string toString() const;

        virtual AccountName getEvilAccount() const;

        virtual int verify(const AccountName& accountName, const PublicKey& pk, const std::string& blsPk) const;

        virtual bool simpleVerify() const;
    };
}

FC_REFLECT( ultrainio::EvilDesc, (chainName)(evil)(blockNum)(exts))
