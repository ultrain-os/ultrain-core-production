#include "rpos/MessageBuilder.h"

#include <rpos/MessageManager.h>
#include <rpos/Node.h>
#include <rpos/Signer.h>

namespace ultrainio {
    EchoMsg MessageBuilder::constructMsg(const Block &block) {
        EchoMsg echo;
        echo.blockHeader = block;
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.pk = std::string(UranusNode::getInstance()->getPublicKey());
        echo.proof = std::string(MessageManager::getInstance()->getVoterProof(block.block_num(), echo.phase, echo.baxCount));
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, UranusNode::getInstance()->getPrivateKey()));
        return echo;
    }

    EchoMsg MessageBuilder::constructMsg(const ProposeMsg &propose) {
        EchoMsg echo;
        echo.blockHeader = propose.block;
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.pk = std::string(UranusNode::getInstance()->getPublicKey());
        echo.proof = std::string(MessageManager::getInstance()->getVoterProof(propose.block.block_num(), echo.phase, echo.baxCount));
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, UranusNode::getInstance()->getPrivateKey()));
        return echo;
    }

    EchoMsg MessageBuilder::constructMsg(const EchoMsg &echo) {
        EchoMsg myEcho = echo;
        myEcho.pk = std::string(UranusNode::getInstance()->getPublicKey());
        myEcho.proof = std::string(MessageManager::getInstance()->getVoterProof(echo.blockHeader.block_num(), echo.phase, echo.baxCount));
        myEcho.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, UranusNode::getInstance()->getPrivateKey()));
        return myEcho;
    }
}