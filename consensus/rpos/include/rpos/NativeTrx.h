#pragma once

#include <ultrainio/chain/action.hpp>
#include <core/Evidence.h>
#include <core/types.h>

namespace ultrainio {
    using Action = ultrainio::chain::action;
    using ActionName = ultrainio::chain::action_name;
    using PermissionLevel = ultrainio::chain::permission_level;

    struct EmptyBlockReason {
        uint32_t blockNum;
        int phase;
        uint32_t baxCount;
        int currentPhase;
        uint32_t currentBaxCount;
        int proposeCount;
        bool isBa0Empty;
    };

    class NativeTrx {
    public:
        static const std::string kDesc;

        static const std::string kEvidence;

        static void sendEvilTrx(const AccountName& reporter, const fc::crypto::private_key& sk, const AccountName& evil, const Evidence& evidence);

        static void reportEvil(const EvilDesc& desc, const Evidence& evidence);

        static void reportEmptyBlockReason(const std::string& chain_name, uint32_t blockNum, const EmptyBlockReason& reason);

    private:
        static SignedTransaction buildTrx(const Action& action, const BlockIdType& referId, const ChainIdType& chainId,
                const fc::crypto::private_key& sk, const fc::time_point_sec& expiration, fc::unsigned_int usageWords);
    };
}

FC_REFLECT( ultrainio::EmptyBlockReason, (blockNum)(phase)(baxCount)(currentPhase)(currentBaxCount)(proposeCount)(isBa0Empty))
