#include "rpos/PhaseMsg.h"

#include <crypto/PrivateKey.h>
#include <rpos/MsgMgr.h>
#include <rpos/Node.h>
#include <rpos/Proof.h>
#include <rpos/Seed.h>
#include <rpos/StakeVoteBase.h>
#include <rpos/Vrf.h>

namespace ultrainio {

    void PhaseMsg::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ultrainio::chain::block_id_type blockId = UranusNode::getInstance()->getPreviousHash();
        std::string previousHash(blockId.data());
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(blockNum);
        ULTRAIN_ASSERT(stakeVotePtr != nullptr, chain::chain_exception, "stakeVotePtr is nullptr");
        AccountName myAccount = StakeVoteBase::getMyAccount();
        m_isVoter = stakeVotePtr->isVoter(myAccount, UranusNode::getInstance()->getNonProducingNode());
    }

    bool PhaseMsg::isVoter() const {
        return m_isVoter;
    }
}