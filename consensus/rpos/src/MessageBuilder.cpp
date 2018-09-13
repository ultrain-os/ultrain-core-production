#include "rpos/MessageBuilder.h"

#include <rpos/MessageManager.h>
#include <rpos/Node.h>
#include <rpos/Signer.h>

namespace ultrainio {
    EchoMsg MessageBuilder::constructMsg(const Block &block) {
        EchoMsg echo;
        echo.blockId = block.id();
        echo.proposerPriority = Proof(block.proposerProof).getPriority();
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.timestamp = UranusNode::getInstance()->getRoundCount();
        echo.account = VoterSystem::getMyAccount();
        echo.proof = std::string(MessageManager::getInstance()->getVoterProof(block.block_num(), echo.phase, echo.baxCount));
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, VoterSystem::getMyPrivateKey()));
        ilog("account : ${account} sign block ${id} signature ${signature}",
             ("account", std::string(VoterSystem::getMyAccount()))("id", block.id())("signature", echo.signature));
        return echo;
    }

    EchoMsg MessageBuilder::constructMsg(const ProposeMsg &propose) {
        EchoMsg echo;
        echo.blockId = propose.block.id();
        echo.proposerPriority = Proof(propose.block.proposerProof).getPriority();
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.timestamp = UranusNode::getInstance()->getRoundCount();
        echo.account = VoterSystem::getMyAccount();
        echo.proof = std::string(MessageManager::getInstance()->getVoterProof(propose.block.block_num(), echo.phase, echo.baxCount));
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, VoterSystem::getMyPrivateKey()));
        ilog("account : ${account} sign block ${id} signature ${signature}",
                ("account", std::string(VoterSystem::getMyAccount()))("id", propose.block.id())("signature", echo.signature));
        return echo;
    }

    EchoMsg MessageBuilder::constructMsg(const EchoMsg &echo) {
        uint32_t blockNum = BlockHeader::num_from_id(echo.blockId);
        EchoMsg myEcho = echo;
        myEcho.timestamp = UranusNode::getInstance()->getRoundCount();
        myEcho.account = VoterSystem::getMyAccount();
        myEcho.proof = std::string(MessageManager::getInstance()->getVoterProof(blockNum, echo.phase, echo.baxCount));
        myEcho.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, VoterSystem::getMyPrivateKey()));
        ilog("account : ${account} sign block ${id} signature ${signature}",
             ("account", std::string(VoterSystem::getMyAccount()))("id", myEcho.blockId)("signature", myEcho.signature));
        return myEcho;
    }
}
