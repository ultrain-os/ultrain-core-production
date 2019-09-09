#include "rpos/NativeTrx.h"

#include <fc/variant.hpp>
#include <fc/io/json.hpp>

#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <ultrainio/net_plugin/net_plugin.hpp>

#include <core/MultiProposeEvidence.h>

namespace ultrainio {
    void NativeTrx::sendMultiSignTrx(const AccountName& reporter, const fc::crypto::private_key& sk, const AccountName& evil, const Evidence& evidence) {
        std::pair<uint64_t, std::string> t = std::make_pair(evil, evidence.toString());
        bytes data = fc::raw::pack(t);
        Action action(std::vector<PermissionLevel>{{reporter, chain::config::active_name}}, N(ultrainio), NEX(verifyprodevil), data);
        chain::controller& chain = appbase::app().get_plugin<chain_plugin>().chain();
        SignedTransaction trx = buildTrx(action, chain.head_block_id(), chain.get_chain_id(), sk, chain.head_block_time() + fc::seconds(60), 5000);
        app().get_plugin<net_plugin>().broadcast(trx);
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