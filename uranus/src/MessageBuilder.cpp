#include "uranus/MessageBuilder.h"

#include <uranus/MessageManager.h>
#include <uranus/Node.h>
#include <uranus/Signer.h>

namespace ultrainio {
    EchoMsg MessageBuilder::constructMsg(const Block &block) {
        EchoMsg echo;
        echo.blockHeader = block;
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.pk = std::string(UranusNode::getInstance()->getPublicKey());
        echo.proof = std::string(
                (char *) MessageManager::getInstance()->getVoterProof(block.block_num(), echo.phase, echo.baxCount),
                VRF_PROOF_LEN);
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, UranusNode::getInstance()->getPrivateKey()));
        return echo;
    }

    EchoMsg MessageBuilder::constructMsg(const ProposeMsg &propose) {
        EchoMsg echo;
        echo.blockHeader = propose.block;
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.pk = std::string(UranusNode::getInstance()->getPublicKey());
        echo.proof = std::string(
                (char *) MessageManager::getInstance()->getVoterProof(propose.block.block_num(), echo.phase,
                        echo.baxCount), VRF_PROOF_LEN);
        echo.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, UranusNode::getInstance()->getPrivateKey()));
        return echo;
    }

    EchoMsg MessageBuilder::constructMsg(const EchoMsg &echo) {
        EchoMsg myEcho = echo;
        myEcho.pk = std::string(UranusNode::getInstance()->getPublicKey());
        myEcho.proof = std::string(
                (char *) MessageManager::getInstance()->getVoterProof(echo.blockHeader.block_num(), echo.phase,
                        echo.baxCount), VRF_PROOF_LEN);
        myEcho.signature = std::string(Signer::sign<UnsignedEchoMsg>(echo, UranusNode::getInstance()->getPrivateKey()));
        return myEcho;
    }
}