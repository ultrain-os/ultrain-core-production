#pragma once

namespace ultrainio {
    class EvilType {
    public:
        static const int kNonEvil;
        static const int kSignMultiPropose; // send multi-propose message
        static const int kVoteMultiPropose; // eg. ba0 vote multi-propose
    };
}
