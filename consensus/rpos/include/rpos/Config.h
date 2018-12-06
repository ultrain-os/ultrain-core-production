#pragma once

#include <string>

namespace ultrainio {

    class Config {
    public:
        static const int kAverageBlockPerMinutes;

        static const int kMaxBaxCount;

        static const int kDeadlineCnt;

        static const double kProposerStakeNumber;

        static const double kVoterStakeNumber;

        static const int kDesiredProposerNumber;

        static const int kDesiredVoterNumber;

        static const double kSendEchoThresholdRatio;

        static const double kNextRoundThresholdRatio;

        static const double kEmptyBlockThresholdRatio;

        static const double kEmptyBlock2ThresholdRatio;

        static int s_maxRoundSeconds;

        static int s_maxPhaseSeconds;

        static const int MAX_LATER_NUMBER;

        static int s_maxTrxMicroSeconds;
    };

    // TODO(qinxiaofen) should add into Config class
#define INVALID_BLOCK_NUM          0xFFFFFFFF
#define THRESHOLD_SYNCING          3
}
