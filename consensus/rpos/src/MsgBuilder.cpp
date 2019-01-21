#include "rpos/MsgBuilder.h"

#include <rpos/MsgMgr.h>
#include <rpos/Node.h>
#include <rpos/Signer.h>
#include <rpos/StakeVoteBase.h>
#include <crypto/Bls.h>

namespace ultrainio {
    EchoMsg MsgBuilder::constructMsg(const Block &block) {
        EchoMsg echo;
        echo.blockId = block.id();
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.account = StakeVoteBase::getMyAccount();
#ifdef CONSENSUS_VRF
        echo.proposerPriority = Proof(block.proposerProof).getPriority();
        echo.proof = std::string(MsgMgr::getInstance()->getVoterProof(block.block_num(), echo.phase, echo.baxCount));
#else
        echo.proposer = block.proposer;
#endif
        unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
        StakeVoteBase::getMyBlsPrivateKey(sk, Bls::BLS_PRI_KEY_LENGTH);
        echo.blsSignature = Signer::sign<CommonEchoMsg>(echo, sk);
        echo.timestamp = UranusNode::getInstance()->getRoundCount();
        // initialized at the end
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, StakeVoteBase::getMyPrivateKey()));
        ilog("account : ${account} sign block ${id} signature ${signature}",
             ("account", std::string(StakeVoteBase::getMyAccount()))("id", block.id())("signature", echo.signature));
        return echo;
    }

    EchoMsg MsgBuilder::constructMsg(const ProposeMsg &propose) {
        EchoMsg echo;
        echo.blockId = propose.block.id();
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.account = StakeVoteBase::getMyAccount();
#ifdef CONSENSUS_VRF
        echo.proposerPriority = Proof(propose.block.proposerProof).getPriority();
        echo.proof = std::string(MsgMgr::getInstance()->getVoterProof(propose.block.block_num(), echo.phase, echo.baxCount));
#else
        echo.proposer = propose.block.proposer;
#endif
        unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
        StakeVoteBase::getMyBlsPrivateKey(sk, Bls::BLS_PRI_KEY_LENGTH);
        echo.blsSignature = Signer::sign<CommonEchoMsg>(echo, sk);
        echo.timestamp = UranusNode::getInstance()->getRoundCount();
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, StakeVoteBase::getMyPrivateKey()));
        ilog("account : ${account} sign block ${id} signature ${signature}",
                ("account", std::string(StakeVoteBase::getMyAccount()))("id", propose.block.id())("signature", echo.signature));
        return echo;
    }

    EchoMsg MsgBuilder::constructMsg(const EchoMsg &echo) {
        EchoMsg myEcho = echo;
        myEcho.account = StakeVoteBase::getMyAccount();
#ifdef CONSENSUS_VRF
        myEcho.proof = std::string(MsgMgr::getInstance()->getVoterProof(BlockHeader::num_from_id(echo.blockId), myEcho.phase, myEcho.baxCount));
#endif
        unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
        StakeVoteBase::getMyBlsPrivateKey(sk, Bls::BLS_PRI_KEY_LENGTH);
        myEcho.blsSignature = Signer::sign<CommonEchoMsg>(myEcho, sk);
        myEcho.timestamp = UranusNode::getInstance()->getRoundCount();
        myEcho.signature = std::string(Signer::sign<UnsignedEchoMsg>(myEcho, StakeVoteBase::getMyPrivateKey()));
        ilog("timestamp : ${timestamp} proof : ${proof} account : ${account} sign block ${id} signature ${signature}",
             ("timestamp", myEcho.timestamp)("account", std::string(StakeVoteBase::getMyAccount()))
             ("id", myEcho.blockId)("signature", myEcho.signature));
        return myEcho;
    }
}
