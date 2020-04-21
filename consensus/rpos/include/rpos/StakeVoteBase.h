#pragma once

#include <memory>
#include <string>
#include <vector>

#include <core/Message.h>
#include <core/types.h>
#include <rpos/NodeInfo.h>
#include <lightclient/CommitteeInfo.h>
#include <lightclient/CommitteeSet.h>

namespace ultrainio {
    // forward declare
    class Proof;

    struct CommitteeState {
        std::vector<CommitteeInfo> cinfo;
        bool chainStateNormal = false;
    };

    class StakeVoteBase {
    public:
        // static function
        static bool committeeHasWorked();

        // get committee state from world state
        static std::shared_ptr<CommitteeState> getCommitteeState(uint64_t chainNum);

        static bool isGenesisLeaderAndInGenesisPeriod();

        static bool newRound(ConsensusPhase phase, int baxCount);

        // StakeVoteRandom
        bool isProposer(const AccountName& account, bool myIsNonProducingNode);

        // StakeVoteRandom
        bool isVoter(const AccountName& account, ConsensusPhase phase, int baxCount, bool myIsNonProducingNode);

        // StakeVoteVrf
        bool isProposer(const AccountName& account, const Proof& proof, bool myIsNonProducingNode);

        // StakeVoteVrf
        bool isVoter(const AccountName& account, const Proof& proof, bool myIsNonProducingNode);

        int getSendEchoThreshold() const;

        int getNextRoundThreshold() const;

        int getEmptyBlockThreshold() const;

        int getEmptyBlock2Threshold() const;

        uint32_t getProposerNumber() const;

        int getCommitteeMemberNumber() const;

        consensus::PublicKeyType getPublicKey(const AccountName& account) const;

        bool getCommitteeBlsPublicKey(const AccountName& account, unsigned char* blsPublicKey, int pkSize) const;

        SHA256 getCommitteeMroot() const;

        CommitteeSet getCommitteeSet() const;

        bool isGenesisPeriod() const;

        virtual Proof getVoterProof(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        virtual Proof getProposerProof(uint32_t blockNum);

        // StakeVoteVrf
        virtual int calSelectedStake(const Proof& proof); // for Voter only

        // StakeVoteRandom
        virtual uint32_t proposerPriority(const AccountName& account, ConsensusPhase phase, int baxCount);

        virtual void moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount);
    protected:
        StakeVoteBase(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr);

        virtual ~StakeVoteBase();

        virtual bool realIsProposer(const AccountName& account);

        virtual bool realIsVoter(const AccountName& account, ConsensusPhase phase, int baxCount);

        // StakeVoteVrf
        virtual bool realIsProposer(const AccountName& account, const Proof& proof);

        // StakeVoteVrf
        virtual bool realIsVoter(const AccountName& account, const Proof& proof);

        virtual int realGetSendEchoThreshold() const;

        virtual int realGetNextRoundThreshold() const;

        virtual int realGetEmptyBlockThreshold() const;

        virtual int realGetEmptyBlock2Threshold() const;

        virtual uint32_t realGetProposerNumber() const;

        std::shared_ptr<CommitteeState> m_committeeStatePtr = nullptr;

        bool isGenesisLeader(const AccountName& account) const;

        bool committeeHasWorked2() const;

        CommitteeInfo findInCommitteeMemberList(const AccountName& account) const;

        uint32_t m_blockNum = 0;
    };
}
