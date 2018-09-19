#include "rpos/Config.h"

namespace ultrainio {
    const int Config::kAverageBlockPerMinutes = 6;

    const int Config::kMaxBaxCount = 20;

    const int Config::kDeadlineCnt = 100;

    const double Config::kProposerStakeNumber = 7.0;

    int Config::s_maxRoundSeconds = 10;

    int Config::s_maxPhaseSeconds = 5;

    const double Config::VOTER_STAKES_NUMBER = 1000.0;
    const int Config::DEFAULT_THRESHOLD = 6000;
    const int Config::MAX_LATER_NUMBER = 3;
}

