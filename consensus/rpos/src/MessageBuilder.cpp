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
        echo.timestamp = UranusNode::getInstance()->getRoundCount();
        echo.account = VoterSystem::getMyAccount();
        echo.proof = std::string(MessageManager::getInstance()->getVoterProof(block.block_num(), echo.phase, echo.baxCount));
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, VoterSystem::getMyPrivateKey()));
        return echo;
    }

    EchoMsg MessageBuilder::constructMsg(const ProposeMsg &propose) {
        EchoMsg echo;
        echo.blockHeader = propose.block;
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.timestamp = UranusNode::getInstance()->getRoundCount();
        echo.account = VoterSystem::getMyAccount();
        echo.proof = std::string(MessageManager::getInstance()->getVoterProof(propose.block.block_num(), echo.phase, echo.baxCount));
        ilog("account : ${account} sign block ${id}", ("account", std::string(VoterSystem::getMyPrivateKey()))("id", propose.block.id()));
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, VoterSystem::getMyPrivateKey()));
        return echo;
    }

    EchoMsg MessageBuilder::constructMsg(const EchoMsg &echo) {
        EchoMsg myEcho = echo;
        myEcho.timestamp = UranusNode::getInstance()->getRoundCount();
        myEcho.account = VoterSystem::getMyAccount();
        myEcho.proof = std::string(MessageManager::getInstance()->getVoterProof(echo.blockHeader.block_num(), echo.phase, echo.baxCount));
        myEcho.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, VoterSystem::getMyPrivateKey()));
        return myEcho;
    }
}
