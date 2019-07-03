#include "core/Message.h"

namespace ultrainio {
    // CommonEchoMsg
    void CommonEchoMsg::toStringStream(std::stringstream& ss) const {
        ss << std::string(blockId) << " ";
        ss << static_cast<int>(phase) << " ";
        ss << baxCount << " ";
#ifdef CONSENSUS_VRF
        ss << proposerPriority << " ";
#else
        ss << std::string(proposer) << " ";
#endif
    }

    bool CommonEchoMsg::fromStringStream(std::stringstream& ss) {
        std::string blockIdStr;
        if (!(ss >> blockIdStr)) {
            return false;
        }
        blockId = BlockIdType(blockIdStr);
        int phaseInt;
        if (!(ss >> phaseInt)) {
            return false;
        }
        phase = static_cast<ConsensusPhase>(phaseInt);
        if (!(ss >> baxCount)) {
            return false;
        }
#ifdef CONSENSUS_VRF
        if (!(ss >> proposerPriority)) {
            return false;
        }
#else
        std::string proposerStr;
        if (!(ss >> proposerStr)) {
            return false;
        }
        proposer = proposerStr;
#endif
        return true;
    }

    bool CommonEchoMsg::operator == (const CommonEchoMsg& rhs) const {
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

    bool UnsignedEchoMsg::operator == (const UnsignedEchoMsg& rhs) const {
        if (this == &rhs) {
            return true;
        }
        if (blsSignature != rhs.blsSignature || account != rhs.account || timestamp != rhs.timestamp) {
            return false;
        }
        size_t size = ext.size();
        if (size != rhs.ext.size()) {
            return false;
        }
        if (size > 0) {
            for (int i = 0; i < size; i++) {
                if (ext[i] != rhs.ext[i]) {
                    return false;
                }
            }
        }
        return CommonEchoMsg::operator==(rhs);
    }

    bool EchoMsg::operator == (const EchoMsg& rhs) const {
        if (this == &rhs) {
            return true;
        }
        if (signature != rhs.signature) {
            return false;
        }
        return UnsignedEchoMsg::operator==(rhs);
    }

    bool ExtType::operator == (const ExtType& rhs) const {
        if (this == &rhs) {
            return true;
        }
        return key == rhs.key && value == rhs.value;
    }

    bool ExtType::operator != (const ExtType& rhs) const {
        return !this->operator==(rhs);
    }
}