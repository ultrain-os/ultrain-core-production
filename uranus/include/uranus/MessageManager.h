#pragma once

#include <memory>
#include <vector>

#include <crypto/Vrf.h>
#include <core/Message.h>
#include <uranus/VoterSystem.h>

namespace ultrainio {
    enum MessageStatus {
        kSuccess,
        kDuplicate,
        kSignatureError,
        kFaultProposer,
        kTheSameRoundButPhase,
        kLaterRound
    };

    struct EchoMsgSet {
        std::vector<EchoMsg> echoMsgV;
        std::vector<std::string> pkPool; // pk pool of echoMsgDescV, speed up search
        int totalVoterCount = 0;
        BlockHeader blockHeader;
    };

    class PhaseMessage {
    public:
        void insert(const EchoMsg& echoMsg);
        void moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        ConsensusPhase phase = kPhaseInit;
        int baxCount = 0;
        int voterCountAsVoter = 0;
        uint8_t proof[VRF_PROOF_LEN] = { 0 };
    private:
        std::map<chain::block_id_type, EchoMsgSet> echoMsgSetMap;
    };

    typedef std::shared_ptr<PhaseMessage> PhaseMessagePtr;

    class BlockMessage {
    public:
        void insert(const EchoMsg& echoMsg);
        void insert(const ProposeMsg& proposeMsg);
        void moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount);
        const uint8_t* getVoterProof(ConsensusPhase phase, int baxCount);
        int getVoterVoterCount(ConsensusPhase phase, int baxCount);

        uint32_t blockNum = 0;
        int voterCountAsProposer = 0;
        uint8_t proposerProof[VRF_PROOF_LEN] = { 0 };
    private:
        PhaseMessagePtr initIfNeed(ConsensusPhase phase, int baxCount);
        std::vector<ProposeMsg> proposeMsgList;
        std::map<int, PhaseMessagePtr> phaseMessageMap; // key = phase + bax_count
    };

    typedef std::shared_ptr<BlockMessage> BlockMessagePtr;

    class MessageManager {
    public:
        static std::shared_ptr<MessageManager> getInstance();
        MessageManager& operator = (const MessageManager&) = delete;
        MessageManager(const MessageManager&) = delete;

        void insert(const EchoMsg& echoMsg);

        void insert(const ProposeMsg& proposeMsg);

        void moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        const uint8_t* getVoterProof(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        const uint8_t* getProposerProof(uint32_t blockNum);

        bool isVoter(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        bool isProposer(uint32_t blockNum);
    private:
        MessageManager() = default;

        int getProposerVoterCount(uint32_t blockNum);

        int getVoterVoterCount(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        BlockMessagePtr initIfNeed(uint32_t blockNum);

        void clearSomeBlockMessage(uint32_t blockNum);

        static std::shared_ptr<MessageManager> s_self;
        std::map<int, BlockMessagePtr> blockMessageMap; // key - blockNum
    };
}