#include <rpos/VoterSet.h>

#include <base/Hex.h>
#include <base/Memory.h>
#include <crypto/Bls.h>

namespace ultrainio {
    // class VoterSet
    bool VoterSet::empty() const {
        if (accountPool.empty()) {
            return true;
        }
        if (accountPool.size() != blsSignPool.size()) {
            return true;
        }
        return false;
    }

    BlsVoterSet VoterSet::toBlsVoterSet(int weight) const {
        if (empty()) {
            return BlsVoterSet();
        }
        BlsVoterSet blsVoterSet;
        size_t N = this->accountPool.size() > weight ? weight : this->accountPool.size();
        blsVoterSet.commonEchoMsg = this->commonEchoMsg;
        for (int i = 0; i < N; i++) {
            blsVoterSet.accountPool.push_back(this->accountPool[i]);
        }
#ifdef CONSENSUS_VRF
        for (int i = 0; i < N; i++) {
            blsVoterSet.proofPool.push_back(this->proofPool[i]);
        }
#endif
        std::vector<std::string> tmpBlsSignPool;
        for (int i = 0; i < N; i++) {
            tmpBlsSignPool.push_back(this->blsSignPool[i]);
        }
        blsVoterSet.sigX = generateSigX(tmpBlsSignPool);
        return blsVoterSet;
    }

    std::string VoterSet::generateSigX(const std::vector<std::string>& aggBlsSignPool) const {
        std::shared_ptr<Bls> blsPtr = Bls::getDefault();
        int n = aggBlsSignPool.size();
        unsigned char** blsSignV = (unsigned char**)malloc(n * sizeof(unsigned char*));
        for (int i = 0; i < n; i++) {
            blsSignV[i] = (unsigned char*)malloc(Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
            Hex::fromHex<unsigned char>(aggBlsSignPool[i], blsSignV[i], Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        }
        unsigned char sigX[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
        bool res = blsPtr->aggregate(blsSignV, n, sigX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        Memory::freeMultiDim<unsigned char>(blsSignV, n);
        if (!res) {
            elog("aggregate bls error");
            return std::string();
        }
        return Hex::toHex(sigX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
    }

    VoterSet VoterSet::subVoterSet(int startIndex, int endIndex) const {
        ULTRAIN_ASSERT(this->accountPool.size() >= endIndex, chain::chain_exception, "${endIndex} > ${accountPool}", ("endIndex", endIndex)("accountPool", this->accountPool.size()));
        VoterSet voterSet;
        voterSet.commonEchoMsg = this->commonEchoMsg;
        for (int i = startIndex; i < endIndex; i++) {
            voterSet.accountPool.push_back(this->accountPool[i]);
            voterSet.sigPool.push_back(this->sigPool[i]);
            voterSet.blsSignPool.push_back(this->blsSignPool[i]);
            voterSet.timePool.push_back(this->timePool[i]);
        }
        return voterSet;
    }

    VoterSet VoterSet::exclude(const std::vector<AccountName>& accounts) const {
        if (accounts.empty()) {
            return *this;
        }
        VoterSet voterSet;
        voterSet.commonEchoMsg = this->commonEchoMsg;
        for (int i = 0; i < this->accountPool.size(); i++) {
            if (std::find(accounts.begin(), accounts.end(), this->accountPool[i]) != accounts.end()) {
                continue;
            }
            voterSet.accountPool.push_back(this->accountPool[i]);
            voterSet.sigPool.push_back(this->sigPool[i]);
            voterSet.blsSignPool.push_back(this->blsSignPool[i]);
            voterSet.timePool.push_back(this->timePool[i]);
        }
        return voterSet;
    }

    void VoterSet::toStringStream(std::stringstream& ss) const {
        if (empty()) {
            return;
        }
        this->commonEchoMsg.toStringStream(ss);
        int n = this->accountPool.size();
        ss << n << " ";
        for (int i = 0; i < n; i++) {
            ss << this->accountPool[i].to_string() << " ";
            ss << this->sigPool[i] << " ";
            ss << this->blsSignPool[i] << " ";
            ss << this->timePool[i] << " ";
        }
    }

    std::string VoterSet::toString() const {
        std::stringstream ss;
        this->toStringStream(ss);
        return ss.str();
    }

    bool VoterSet::fromStringStream(std::stringstream& ss) {
        if (!this->commonEchoMsg.fromStringStream(ss)) {
            return false;
        }
        int n;
        if (!(ss >> n)) {
            return false;
        }
        for (int i = 0; i < n; i++) {
            std::string account;
            if (!(ss >> account)) {
                return false;
            }
            this->accountPool.push_back(AccountName(account));

            std::string sign;
            if (!(ss >> sign)) {
                return false;
            }
            this->sigPool.push_back(sign);

            std::string blsSign;
            if (!(ss >> blsSign)) {
                return false;
            }
            this->blsSignPool.push_back(blsSign);

            uint32_t time;
            if (!(ss >> time)) {
                return false;
            }
            this->timePool.push_back(time);
        }
        return true;
    }

    VoterSet::VoterSet() {}

    VoterSet::VoterSet(const std::string& str) {
        std::stringstream ss(str);
        this->fromStringStream(ss);
    }
}