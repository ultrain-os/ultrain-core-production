#include "core/Message.h"

namespace ultrainio {
    // CommonEchoMsg

    void CommonEchoMsg::toVariants(fc::variants& v) const {
        fc::variant blockIdV;
        fc::to_variant(blockId, blockIdV);
        v.push_back(blockIdV);
        v.push_back(fc::variant(static_cast<int>(phase)));
        v.push_back(fc::variant(baxCount));
#ifdef CONSENSUS_VRF
        v.push_back(fc::variant(proposerPriority));
#else
        v.push_back(fc::variant(std::string(proposer)));
#endif
    }

    int CommonEchoMsg::fromVariants(const fc::variants& v) {
        int nextIndex = 0;
        blockId = v[nextIndex++].as<fc::sha256>();
        phase = ConsensusPhase(v[nextIndex++].as<int>());
        baxCount = v[nextIndex++].as<uint32_t>();
#ifdef CONSENSUS_VRF
        proposerPriority = v[nextIndex++].as<uint32_t>();
#else
        proposer = AccountName(v[nextIndex++].as_string());
#endif
        return nextIndex;
    }

    bool CommonEchoMsg::operator == (const CommonEchoMsg& rhs) {
        if (this == &rhs) {
            return true;
        }
        if (blockId == rhs.blockId
                && phase == rhs.phase
                && baxCount == rhs.baxCount
#ifdef CONSENSUS_VRF
                && proposerPriority == rhs.proposerPriority) {
#else
                && proposer == rhs.proposer ) {
#endif
            return true;
        }
        return false;
    }

    // BlsVoterSet
    bool BlsVoterSet::empty() {
        return accountPool.empty();
    }

    void BlsVoterSet::toVariants(fc::variants& v) const {
        commonEchoMsg.toVariants(v);
        uint32_t n = accountPool.size();
        v.push_back(fc::variant(n));
        for (int i = 0; i < n; i++) {
            v.push_back(fc::variant(std::string(accountPool[i])));
        }
#ifdef CONSENSUS_VRF
        for (int i = 0; i < n; i++) {
            v.push_back(fc::variant(std::string(proofPool[i])));
        }
#endif
        v.push_back(fc::variant(sigX));
    }

    void BlsVoterSet::fromVariants(const fc::variants& v) {
        int nextIndex = commonEchoMsg.fromVariants(v);
        uint32_t n = v[nextIndex++].as<uint32_t>();
        for (int i = 0; i < n; i++) {
            accountPool.push_back(v[nextIndex++].as_string());
        }
#ifdef CONSENSUS_VRF
        for (int i = 0; i < n; i++) {
            proofPool.push_back(v[nextIndex++].as_string());
        }
#endif
        // sigX
        sigX = v[nextIndex].as_string();
    }

    bool BlsVoterSet::operator == (const BlsVoterSet& rhs) {
        if (this == &rhs) {
            return true;
        }
        if (commonEchoMsg == rhs.commonEchoMsg
                && accountPool == rhs.accountPool
#ifdef CONSENSUS_VRF
                && proofPool == rhs.proofPool
#endif
                && sigX == rhs.sigX) {
            return true;
        }
        return false;
    }
}