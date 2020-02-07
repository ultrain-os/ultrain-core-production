#include "rpos/MsgBuilder.h"

#include <rpos/MsgMgr.h>
#include <rpos/Node.h>
#include <rpos/Utils.h>
#include <rpos/NodeInfo.h>
#include <crypto/Bls.h>
#include <crypto/Signer.h>

namespace ultrainio {
    EchoMsg MsgBuilder::constructMsg(const Block &block, size_t index) {
        EchoMsg echo;
        echo.blockId = block.id();
        echo.phase = Node::getInstance()->getPhase();
        echo.baxCount = Node::getInstance()->getBaxCount();
        echo.account = NodeInfo::getInstance()->getAccount(index);
        echo.proposer = block.proposer;
        unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
        NodeInfo::getInstance()->getBlsPrivateKey(sk, Bls::BLS_PRI_KEY_LENGTH, index);
        echo.blsSignature = Signer::sign<CommonEchoMsg>(echo, sk);
        echo.timestamp = Node::getInstance()->getRoundCount();
        // initialized at the end
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, NodeInfo::getInstance()->getPrivateKey(index)));
        ilog("account : ${account} sign block ${id} signature ${signature}",
             ("account", NodeInfo::getInstance()->getAccount(index))("id", short_hash(block.id()))("signature", short_sig(echo.signature)));
        return echo;
    }

    EchoMsg MsgBuilder::constructMsg(const ProposeMsg &propose, size_t index) {
        return constructMsg(propose.block, index);
    }

    EchoMsg MsgBuilder::constructMsg(const EchoMsg &echo, size_t index) {
        EchoMsg myEcho = echo;
        myEcho.account = NodeInfo::getInstance()->getAccount(index);
        unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
        NodeInfo::getInstance()->getBlsPrivateKey(sk, Bls::BLS_PRI_KEY_LENGTH, index);
        myEcho.blsSignature = Signer::sign<CommonEchoMsg>(myEcho, sk);
        myEcho.timestamp = Node::getInstance()->getRoundCount();
        myEcho.signature = std::string(Signer::sign<UnsignedEchoMsg>(myEcho, NodeInfo::getInstance()->getPrivateKey(index)));
        ilog("timestamp : ${timestamp} account : ${account} sign block ${id} signature ${signature}",
             ("timestamp", myEcho.timestamp)("account", NodeInfo::getInstance()->getAccount(index))
             ("id", short_hash(myEcho.blockId))("signature", short_sig(myEcho.signature)));
        return myEcho;
    }
}
