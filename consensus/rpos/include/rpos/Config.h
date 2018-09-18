#pragma once

#include <string>

namespace ultrainio {

    class Config {
    public:
        static const int kAverageBlockPerMinutes;

        static const int kMaxBaxCount;

        static const int kDeadlineCnt;

        static int s_maxRoundSeconds;

        static int s_maxPhaseSeconds;

        // TODO(qinxiaofen) code style
        static const double VOTER_STAKES_NUMBER;
        static const double PROPOSER_STAKES_NUMBER;
        static const int DEFAULT_THRESHOLD;
        static const int MAX_LATER_NUMBER;
    };

    // TODO(qinxiaofen) should add into Config class
#define INVALID_BLOCK_NUM          0xFFFFFFFF
#define THRESHOLD_SEND_ECHO        330
#define THRESHOLD_NEXT_ROUND       670
#define THRESHOLD_SYNCING          3
#define THRESHOLD_EMPTY_BLOCK      888
#define THRESHOLD_EMPTY_BLOCK2     750
}
