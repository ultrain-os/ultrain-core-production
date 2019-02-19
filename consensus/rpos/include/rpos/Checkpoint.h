#pragma once

class Checkpoint {
public:
    static bool isCheckpoint(const BlockHeader& blockHeader);

    Checkpoint(const BlockHeader& blockHeader);
private:
    BlockIdType m_blockId;
    Checksum256Type m_committeeMroot;

    // header extensions
    BlockIdTyep m_preCheckpointBlockId;
    std::vector<CommitteeInfo> m_committeeInfo;
    BlsVoterSet m_blsVoterSet;
};
