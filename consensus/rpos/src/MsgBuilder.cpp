#include "rpos/MsgBuilder.h"

#include <rpos/MsgMgr.h>
#include <rpos/Node.h>
#include <rpos/Utils.h>
#include <rpos/StakeVoteBase.h>
#include <crypto/Bls.h>
#include <crypto/Signer.h>

namespace ultrainio {
    EchoMsg MsgBuilder::constructMsg(const Block &block) {
        EchoMsg echo;
        echo.blockId = block.id();
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.account = StakeVoteBase::getMyAccount();
        echo.proposer = block.proposer;
        unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
        StakeVoteBase::getMyBlsPrivateKey(sk, Bls::BLS_PRI_KEY_LENGTH);
        echo.blsSignature = Signer::sign<CommonEchoMsg>(echo, sk);
        echo.timestamp = UranusNode::getInstance()->getRoundCount();
        // initialized at the end
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, StakeVoteBase::getMyPrivateKey()));
        ilog("account : ${account} sign block ${id} signature ${signature}",
             ("account", std::string(StakeVoteBase::getMyAccount()))("id", short_hash(block.id()))("signature", short_sig(echo.signature)));
        return echo;
    }

    EchoMsg MsgBuilder::constructMsg(const ProposeMsg &propose) {
        EchoMsg echo;
        echo.blockId = propose.block.id();
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.account = StakeVoteBase::getMyAccount();
        echo.proposer = propose.block.proposer;
        unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
        StakeVoteBase::getMyBlsPrivateKey(sk, Bls::BLS_PRI_KEY_LENGTH);
        echo.blsSignature = Signer::sign<CommonEchoMsg>(echo, sk);
        echo.timestamp = UranusNode::getInstance()->getRoundCount();
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, StakeVoteBase::getMyPrivateKey()));
        ilog("account : ${account} sign block ${id} signature ${signature}",
             ("account", std::string(StakeVoteBase::getMyAccount()))("id", short_hash(propose.block.id()))("signature", short_sig(echo.signature)));
        return echo;
    }

    EchoMsg MsgBuilder::constructMsg(const EchoMsg &echo) {
        EchoMsg myEcho = echo;
        myEcho.account = StakeVoteBase::getMyAccount();
        unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
        StakeVoteBase::getMyBlsPrivateKey(sk, Bls::BLS_PRI_KEY_LENGTH);
        myEcho.blsSignature = Signer::sign<CommonEchoMsg>(myEcho, sk);
        myEcho.timestamp = UranusNode::getInstance()->getRoundCount();
        myEcho.signature = std::string(Signer::sign<UnsignedEchoMsg>(myEcho, StakeVoteBase::getMyPrivateKey()));
        ilog("timestamp : ${timestamp} account : ${account} sign block ${id} signature ${signature}",
             ("timestamp", myEcho.timestamp)("account", std::string(StakeVoteBase::getMyAccount()))
             ("id", short_hash(myEcho.blockId))("signature", short_sig(myEcho.signature)));
        return myEcho;
    }
}
