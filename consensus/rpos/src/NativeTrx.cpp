#include "rpos/NativeTrx.h"

#include <fc/variant.hpp>
#include <fc/io/json.hpp>

#include <ultrainio/net_plugin/net_plugin.hpp>

#include <core/MultiProposeEvidence.h>
#include "rpos/Config.h"
#include "rpos/Node.h"

namespace ultrainio {
    const std::string NativeTrx::kDesc = "kDesc";

    const std::string NativeTrx::kEvidence = "kEvidence";

    void NativeTrx::sendEvilTrx(const AccountName& reporter, const fc::crypto::private_key& sk, const AccountName& evil, const Evidence& evidence) {
        if (Config::s_allowReportEvil) {
            std::pair<uint64_t, std::string> t = std::make_pair(evil, evidence.toString());
            bytes data = fc::raw::pack(t);
            Action action(std::vector<PermissionLevel>{{reporter, chain::config::active_name}}, N(ultrainio), NEX(verifyprodevil), data);
            chain::controller& chain = appbase::app().get_plugin<chain_plugin>().chain();
            SignedTransaction trx = buildTrx(action, chain.head_block_id(), chain.get_chain_id(), sk, chain.head_block_time() + fc::seconds(60), 5000);
            app().get_plugin<net_plugin_n::net_plugin>().broadcast(trx);
        }
    }

    void NativeTrx::reportEvil(const EvilDesc& desc, const Evidence& evidence) {
        if (Config::s_allowReportEvil) {
            fc::mutable_variant_object o;
            fc::variant descVar(desc);
            fc::variant evidenceVar(evidence.toString());
            o[kDesc] = descVar;
            o[kEvidence] = evidenceVar;
            Node::getInstance()->getEvilReportHandler()(std::string(desc.chainName), desc.blockNum, fc::json::to_string(fc::variant(o)), std::string());
        }
    }

    void NativeTrx::reportEmptyBlockReason(const std::string& chain_name, uint32_t blockNum, const EmptyBlockReason& reason) {
        if (Config::s_allowReportEvil) {
            Node::getInstance()->getBlockReportHandler()(chain_name, blockNum, fc::json::to_string(fc::variant(reason)), std::string());
        }
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
