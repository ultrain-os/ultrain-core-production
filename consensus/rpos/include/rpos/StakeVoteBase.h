#pragma once

#include <memory>
#include <string>
#include <vector>

#include <core/Message.h>
#include <core/Redefined.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <rpos/CommitteeInfo.h>

namespace ultrainio {
    // forward declare
    struct CommitteeState;
    class PublicKey;
    class Proof;
    class NodeInfo;

    class StakeVoteBase {
    public:
        // static function
        static std::shared_ptr<NodeInfo> getKeyKeeper();

        static AccountName getMyAccount();

        static PrivateKey getMyPrivateKey();

        static bool committeeHasWorked();

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

        PublicKey getPublicKey(const AccountName& account) const;

        chain::checksum256_type getCommitteeMroot() { return m_committeeMroot; }

        virtual Proof getVoterProof(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        virtual Proof getProposerProof(uint32_t blockNum);

        // StakeVoteVrf
        virtual int calSelectedStake(const Proof& proof); // for Voter only

        // StakeVoteRandom
        virtual uint32_t proposerPriority(const AccountName& account, ConsensusPhase phase, int baxCount);

        virtual void moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount);
    protected:
        StakeVoteBase(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr);

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

        bool isGenesisPeriod() const;

        bool isGenesisLeader(const AccountName& account) const;

        bool committeeHasWorked2() const;

        PublicKey findInCommitteeMemberList(const AccountName& account) const;

        uint32_t m_blockNum = 0;

    private:
        static std::shared_ptr<NodeInfo> s_keyKeeper;

        // get committee state from world state
        static std::shared_ptr<CommitteeState> getCommitteeState();

        void computeCommitteeMroot();

        chain::checksum256_type m_committeeMroot;
    };
}
