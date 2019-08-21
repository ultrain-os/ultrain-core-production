#include "rpos/NativeTrx.h"

#include <fc/variant.hpp>
#include <fc/io/json.hpp>

#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <ultrainio/net_plugin/net_plugin.hpp>

#include <rpos/EvidenceMultiSign.h>

namespace ultrainio {
    void NativeTrx::sendMultiSignTrx(const AccountName& p, const fc::crypto::private_key& sk,
            const SignedBlockHeader& one, const SignedBlockHeader& other) {
        EvidenceMultiSign evidence(one.proposer, one, other);
        Action action = buildAction(N(ultrainio), NEX(verifyprodevil), vector<PermissionLevel>{{p, chain::config::active_name}}, evidence.toString());
        chain::controller& chain = appbase::app().get_plugin<chain_plugin>().chain();
        SignedTransaction trx = buildTrx(action, chain.head_block_id(), chain.get_chain_id(), sk, chain.head_block_time() + fc::seconds(60), 5000);
        app().get_plugin<net_plugin>().broadcast(trx);
    }

    Action NativeTrx::buildAction(const AccountName& accountName, const ActionName& actionName, const std::vector<PermissionLevel>& auth, const std::string& data) {
        Action act;
        act.account = accountName;
        act.name = actionName;
        act.authorization = auth;
        act.data = fc::raw::pack(data);
        return act;
    }

    SignedTransaction NativeTrx::buildTrx(const Action& action, const BlockIdType& referId, const ChainIdType& chainId,
            const fc::crypto::private_key& sk, const fc::time_point_sec& expiration, fc::unsigned_int usageWords) {
        SignedTransaction trx;
        trx.actions.push_back(action);
        trx.set_reference_block(referId);
        trx.expiration = expiration;
        trx.max_net_usage_words = usageWords;
        trx.sign(sk, chainId);
        return trx;
    }
}