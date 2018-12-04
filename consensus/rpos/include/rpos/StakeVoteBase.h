#pragma once

#include <memory>
#include <string>
#include <vector>

#include <core/Redefined.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <rpos/CommitteeInfo.h>

namespace ultrainio {
    // forward declare
    struct CommitteeState;
    class PublicKey;
    class NodeInfo;

    class StakeVoteBase {
    public:
        // static function
        static std::shared_ptr<NodeInfo> getKeyKeeper();

        static AccountName getMyAccount();

        static PrivateKey getMyPrivateKey();

        static bool committeeHasWorked();

        bool isProposer(const AccountName& account, bool myIsNonProducingNode);

        bool isVoter(const AccountName& account, bool myIsNonProducingNode);

        int getSendEchoThreshold() const;

        int getNextRoundThreshold() const;

        int getEmptyBlockThreshold() const;

        int getEmptyBlock2Threshold() const;

        uint32_t getProposerNumber() const;

        int getCommitteeMemberNumber() const;

        PublicKey getPublicKey(const AccountName& account) const;

        chain::checksum256_type getCommitteeMroot() { return m_committeeMroot; }

        virtual uint32_t proposerPriority(const AccountName& account);
    protected:
        StakeVoteBase(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr);

        virtual bool realIsProposer(const AccountName& account);

        virtual bool realIsVoter(const AccountName& account);

        virtual int realGetSendEchoThreshold() const;

        virtual int realGetNextRoundThreshold() const;

        virtual int realGetEmptyBlockThreshold() const;

        virtual int realGetEmptyBlock2Threshold() const;

        virtual uint32_t realGetProposerNumber() const;

        std::shared_ptr<CommitteeState> m_committeeStatePtr = nullptr;

        bool isGenesisPeriod() const;

        bool isGenesisLeader(const AccountName& account) const;

        bool committeeHasWorked2() const;

    private:
        static std::shared_ptr<NodeInfo> s_keyKeeper;

        // get committee state from world state
        static std::shared_ptr<CommitteeState> getCommitteeState();

        PublicKey findInCommitteeMemberList(const AccountName& account) const;

        void computeCommitteeMroot();

        uint32_t m_blockNum = 0;
        chain::checksum256_type m_committeeMroot;
    };
}