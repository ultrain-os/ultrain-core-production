#pragma once

#define BUFSIZE 1024
#define MIN_MSG_HEAD 28

/**
 * MOST_ATTACK_NUMBER_F = upper(COMMITTER_NUMBER_N/3)
 */
#define GLOBAL_NODE_NUMBER         100
#define COMMITTEE_NODE_PERCENT     0.2
#define COMMITTEE_NUMBER_N         (int)(GLOBAL_NODE_NUMBER * COMMITTEE_NODE_PERCENT)
#define MOST_ATTACK_NUMBER_F       ((COMMITTEE_NUMBER_N + 2)/3)
#define THRESHOLD_SEND_ECHO        (MOST_ATTACK_NUMBER_F + 1)
#define THRESHOLD_NEXT_ROUND       (2 * MOST_ATTACK_NUMBER_F + 1)

enum consensus_phase {
    PHASE_INIT = 0,
    PHASE_BA0,
    PHASE_BA1
};

const std::string consensus_phase_string[] = {
        std::string("PHASE_INIT"),
        std::string("PHASE_BA0"),
        std::string("PHASE_BA1")
};

enum uranus_role : unsigned int {
    NONE = 0,
    PROPOSER,
    VOTER,
    LISTENER,
};

enum msg_type {
    MSG_TYPE_NOMSG = 0,
    MSG_TYPE_PROPOSE,
    MSG_TYPE_ECHO,
    MSG_TYPE_EMPTY
};

enum msg_field_type : unsigned int {
    TXS_HASH = 0,
    TXS_HASH_SIGN,
    PK,
    ROLE_VRF,
    TXS,
    TXS_SIGN,
    PROPOSER_PK,
    PROPOSER_ROLE_VRF,
    TXS_EMPTY,
    TAIL
};
