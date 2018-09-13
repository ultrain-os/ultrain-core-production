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
    class Proof;
    class KeyKeeper;

    class VoterSystem {
    public:
        static std::shared_ptr<VoterSystem> create(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr);

        static std::shared_ptr<KeyKeeper> getKeyKeeper();

        static AccountName getMyAccount();

        static PrivateKey getMyPrivateKey();

        static bool committeeHasWorked();

        // TODO should remove committeeHasWorked
        bool committeeHasWorked2() const;

        int getStakes(const AccountName& account, bool isNonProducingNode);

        double getProposerRatio();

        double getVoterRatio();

        int getCommitteeMemberNumber() const;

        int count(const Proof& proof, int stakes, double p);

        PublicKey getPublicKey(const AccountName& account) const;
    private:
        static std::shared_ptr<KeyKeeper> s_keyKeeper;

        VoterSystem(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr);

        // get committee state from world state
        static std::shared_ptr<CommitteeState> getCommitteeState();

        PublicKey findInCommitteeMemberList(const AccountName& account) const;

        bool isGenesisLeader(const AccountName& account) const;

        bool isCommitteeMember(const AccountName& account) const;

        bool isGenesisPeriod() const;

        long getTotalStakes() const;

        int reverseBinoCdf(double rand, int stake, double p);

        uint32_t m_blockNum = 0;
        std::shared_ptr<CommitteeState> m_committeeStatePtr = nullptr;
        double m_proposerRatio = 0.0;
        double m_voterRatio = 0.0;
    };
}
