#include <rpos/EvilType.h>

namespace ultrainio {
    const int EvilType::kNonEvil          = 0x0;
    const int EvilType::kSignMultiPropose = 0x1; // send multi-propose message
    const int EvilType::kVoteMultiPropose = 0x2; // eg. ba0 vote multi-propose
}