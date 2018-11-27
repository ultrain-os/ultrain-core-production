#include "rpos/MsgBuilder.h"

#include <rpos/MsgMgr.h>
#include <rpos/Node.h>
#include <rpos/Signer.h>
#include <rpos/StakeVoteBase.h>

namespace ultrainio {
    EchoMsg MsgBuilder::constructMsg(const Block &block) {
        EchoMsg echo;
        echo.blockId = block.id();
        echo.proposer = block.proposer;
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.timestamp = UranusNode::getInstance()->getRoundCount();
        echo.account = StakeVoteBase::getMyAccount();
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, StakeVoteBase::getMyPrivateKey()));
        ilog("account : ${account} sign block ${id} signature ${signature}",
             ("account", std::string(StakeVoteBase::getMyAccount()))("id", block.id())("signature", echo.signature));
        return echo;
    }

    EchoMsg MsgBuilder::constructMsg(const ProposeMsg &propose) {
        EchoMsg echo;
        echo.blockId = propose.block.id();
        echo.proposer = propose.block.proposer;
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.timestamp = UranusNode::getInstance()->getRoundCount();
        echo.account = StakeVoteBase::getMyAccount();
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, StakeVoteBase::getMyPrivateKey()));
        ilog("account : ${account} sign block ${id} signature ${signature}",
                ("account", std::string(StakeVoteBase::getMyAccount()))("id", propose.block.id())("signature", echo.signature));
        return echo;
    }

    EchoMsg MsgBuilder::constructMsg(const EchoMsg &echo) {
        EchoMsg myEcho = echo;
        myEcho.timestamp = UranusNode::getInstance()->getRoundCount();
        myEcho.account = StakeVoteBase::getMyAccount();
        myEcho.signature = std::string(Signer::sign<UnsignedEchoMsg>(myEcho, StakeVoteBase::getMyPrivateKey()));
        ilog("timestamp : ${timestamp} proof : ${proof} account : ${account} sign block ${id} signature ${signature}",
             ("timestamp", myEcho.timestamp)("account", std::string(StakeVoteBase::getMyAccount()))
             ("id", myEcho.blockId)("signature", myEcho.signature));
        return myEcho;
    }
}
