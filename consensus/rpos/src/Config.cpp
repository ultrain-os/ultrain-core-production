#include "rpos/Config.h"

namespace ultrainio {
    const int Config::kAverageBlockPerMinutes = 6;

    const int Config::kDeadlineCnt = 60;

    const double Config::kProposerStakeNumber = 7.0;

    const double Config::kVoterStakeNumber = 1000.0;

    const int Config::kDesiredProposerNumber = 2;

    const int Config::kDesiredVoterNumber = 100;

    const double Config::kSendEchoThresholdRatio = 0.33;

    const double Config::kNextRoundThresholdRatio = 0.67;

    const double Config::kEmptyBlockThresholdRatio = 0.80;

    const double Config::kEmptyBlock2ThresholdRatio = 0.67;

    int Config::s_maxRoundSeconds = 10;

    int Config::s_maxPhaseSeconds = 5;

    const int Config::kMaxLaterNumber = 3;

    // This is the deadline for all the trx times when proposing a block.
    // Make it smaller if we want consensus to be more stable.
    int Config::s_maxTrxMicroSeconds = 2700000;

    bool Config::s_allowReportEvil = false;

    int Config::kMaxBaxCount = 20;
}
