#ifndef DEFINE_H_
#define DEFINE_H_

#define BUFSIZE 1024
#define MIN_MSG_HEAD 28

#define MSG_TYPE_NOMSG   0
#define MSG_TYPE_PROPOSE 1
#define MSG_TYPE_ECHO    2
#define MSG_TYPE_EMPTY   3

#define MSG_ECHO_SEND_WAIT 0
#define MSG_ECHO_NEED_SEND 1
#define MSG_ECHO_SEND_OK 2

//#define THRESHOLD_SEND_ECHO 70
//#define THRESHOLD_NEXT_ROUND 135
#define THRESHOLD_SEND_ECHO 1
#define THRESHOLD_NEXT_ROUND 1

#define MSG_LENGTH_KEY   32
#define MSG_LENGTH_HASH  32

#define PHASE_INIT       0
#define PHASE_BA0        1
#define PHASE_BA1        2

enum processresult : unsigned int {
    processOk = 0,
    processErr,
    IntoBA,
    SendMsg,
    Block,
};

enum uranusState : unsigned int {
    NONE = 0,
    PROPOSER,
    VOTER,
    LISTENER,
};

enum MsgFieldType : unsigned int {
    PROOF = 0,
    SIGN,
    PROPOSERPROOF,
    PROPOSERSIGN,
    TXS,
    TXSHASH,
    TXSEMPTY,
    TAIL,
};


#endif


