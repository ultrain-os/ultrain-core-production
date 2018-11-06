#include "rpos/MsgBuilder.h"

#include <rpos/MsgMgr.h>
#include <rpos/Node.h>
#include <rpos/Signer.h>

namespace ultrainio {
    EchoMsg MsgBuilder::constructMsg(const Block &block) {
        EchoMsg echo;
        echo.blockId = block.id();
        echo.proposerPriority = Proof(block.proposerProof).getPriority();
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.timestamp = UranusNode::getInstance()->getRoundCount();
        echo.account = StakeVote::getMyAccount();
        echo.proof = std::string(MsgMgr::getInstance()->getVoterProof(block.block_num(), echo.phase, echo.baxCount));
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, StakeVote::getMyPrivateKey()));
        ilog("account : ${account} sign block ${id} signature ${signature}",
             ("account", std::string(StakeVote::getMyAccount()))("id", block.id())("signature", echo.signature));
        return echo;
    }

    EchoMsg MsgBuilder::constructMsg(const ProposeMsg &propose) {
        EchoMsg echo;
        echo.blockId = propose.block.id();
        echo.proposerPriority = Proof(propose.block.proposerProof).getPriority();
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.timestamp = UranusNode::getInstance()->getRoundCount();
        echo.account = StakeVote::getMyAccount();
        echo.proof = std::string(MsgMgr::getInstance()->getVoterProof(propose.block.block_num(), echo.phase, echo.baxCount));
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, StakeVote::getMyPrivateKey()));
        ilog("account : ${account} sign block ${id} signature ${signature}",
                ("account", std::string(StakeVote::getMyAccount()))("id", propose.block.id())("signature", echo.signature));
        return echo;
    }

    EchoMsg MsgBuilder::constructMsg(const EchoMsg &echo) {
        uint32_t blockNum = BlockHeader::num_from_id(echo.blockId);
        EchoMsg myEcho = echo;
        myEcho.timestamp = UranusNode::getInstance()->getRoundCount();
        myEcho.account = StakeVote::getMyAccount();
        myEcho.proof = std::string(MsgMgr::getInstance()->getVoterProof(blockNum, myEcho.phase, myEcho.baxCount));
        myEcho.signature = std::string(Signer::sign<UnsignedEchoMsg>(myEcho, StakeVote::getMyPrivateKey()));
        ilog("timestamp : ${timestamp} proof : ${proof} account : ${account} sign block ${id} signature ${signature}",
             ("proof", myEcho.proof)("timestamp", myEcho.timestamp)("account", std::string(StakeVote::getMyAccount()))
             ("id", myEcho.blockId)("signature", myEcho.signature));
        return myEcho;
    }
}