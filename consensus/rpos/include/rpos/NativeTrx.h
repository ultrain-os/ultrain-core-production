#pragma once

#include <ultrainio/chain/action.hpp>
#include <core/types.h>

namespace ultrainio {
    using Action = ultrainio::chain::action;
    using ActionName = ultrainio::chain::action_name;
    using PermissionLevel = ultrainio::chain::permission_level;

    class NativeTrx {
    public:
        static void sendMultiSignTrx(const AccountName& sender, const fc::crypto::private_key& sk, const AccountName& evil,
                const SignedBlockHeader& one, const SignedBlockHeader& other);

    private:
        static SignedTransaction buildTrx(const Action& action, const BlockIdType& referId, const ChainIdType& chainId,
                const fc::crypto::private_key& sk, const fc::time_point_sec& expiration, fc::unsigned_int usageWords);
    };
}
