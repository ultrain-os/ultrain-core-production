#pragma once

namespace ultrainio {

    class Config {
    public:
        static constexpr double VOTER_STAKES_NUMBER = 1000.0;
        static constexpr double PROPOSER_STAKES_NUMBER = 20.0;
        static constexpr int DEFAULT_THRESHOLD = 6000;
    };

    // TODO(qinxiaofen) should add into Config class
#define INVALID_BLOCK_NUM          0xFFFFFFFF
#define THRESHOLD_SEND_ECHO        330
#define THRESHOLD_NEXT_ROUND       670
#define THRESHOLD_SYNCING          670
#define MAX_PROPOSE_TRX_COUNT      5000
}