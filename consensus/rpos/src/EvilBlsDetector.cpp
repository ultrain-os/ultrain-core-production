#include "rpos/EvilBlsDetector.h"

#include <base/Hex.h>
#include <crypto/Bls.h>

namespace ultrainio {
    void EvilBlsDetector::detect(const VoterSet& voterSet, const CommitteeSet& committeeSet,
                                 VoterSet& newVoterSet, std::vector<AccountName>& evilAccounts) {
        realDetect(voterSet, 0, voterSet.accountPool.size(), committeeSet, evilAccounts);
        newVoterSet = voterSet.exclude(evilAccounts);
    }

    void EvilBlsDetector::realDetect(const VoterSet& voterSet, int fromIndex, int toIndex, const CommitteeSet& committeeSet,
            std::vector<AccountName>& evilAccounts) {
        if (toIndex <= fromIndex) {
            return;
        }
        // why 7?
        // Assume 100 node and only one evil in 99% time, and 50, 25, 13, 7.
        if (toIndex - fromIndex <= 7) {
            std::shared_ptr<Bls> blsPtr = Bls::getDefault();
            std::vector<AccountName> accounts;
            for (int i = fromIndex; i < toIndex; i++) {
                accounts.push_back(voterSet.accountPool[i]);
            }
            std::vector<std::string> blsPks = committeeSet.getBlsPk(accounts);
            for (int i = fromIndex; i < toIndex; i++) {
                std::string blsPkStr = blsPks[i - fromIndex];
                unsigned char blsPk[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
                Hex::fromHex<unsigned char>(blsPkStr, blsPk, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);
                unsigned char blsSignUC[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
                Hex::fromHex(voterSet.blsSignPool[i], blsSignUC, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
                fc::sha256 h = fc::sha256::hash(voterSet.commonEchoMsg);
                if (!blsPtr->verify(blsPk, blsSignUC, (void*)(h.str().c_str()), h.str().length())) {
                    evilAccounts.push_back(voterSet.accountPool[i]);
                }
            }
            return;
        }
        int middle = (toIndex + fromIndex) / 2;
        VoterSet leftVoterSet = voterSet.subVoterSet(fromIndex, middle);
        BlsVoterSet leftBlsVoterSet = leftVoterSet.toBlsVoterSet(middle - fromIndex);
        std::vector<std::string> leftBlsPkV = committeeSet.getBlsPk(leftBlsVoterSet.accountPool);
        if (!leftBlsVoterSet.verifyBls(leftBlsPkV)) {
            realDetect(voterSet, fromIndex, middle, committeeSet, evilAccounts);
        }

        VoterSet rightVoterSet = voterSet.subVoterSet(middle, toIndex);
        BlsVoterSet rightBlsVoterSet = rightVoterSet.toBlsVoterSet(toIndex - middle);
        std::vector<std::string> rightBlsPkV = committeeSet.getBlsPk(rightBlsVoterSet.accountPool);
        if (!rightBlsVoterSet.verifyBls(rightBlsPkV)) {
            realDetect(voterSet, middle, toIndex, committeeSet, evilAccounts);
        }
    }
}