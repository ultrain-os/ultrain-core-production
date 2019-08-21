#pragma once

#include <ultrainio/chain/action.hpp>
#include <core/types.h>

namespace ultrainio {
    using Action = ultrainio::chain::action;
    using ActionName = ultrainio::chain::action_name;
    using PermissionLevel = ultrainio::chain::permission_level;

    class NativeTrx {
    public:
        static void sendMultiSignTrx(const AccountName& p, const fc::crypto::private_key& sk,
                const SignedBlockHeader& one, const SignedBlockHeader& other);

    private:
        static Action buildAction(const AccountName& accountName, const ActionName& actionName, const std::vector<PermissionLevel>& auth, const std::string& data);

        static SignedTransaction buildTrx(const Action& action, const BlockIdType& referId, const ChainIdType& chainId,
                const fc::crypto::private_key& sk, const fc::time_point_sec& expiration, fc::unsigned_int usageWords);
    };
}
