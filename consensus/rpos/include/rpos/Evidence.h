#pragma once

#include <string>

namespace ultrainio {
    class Evidence {
    public:
        static const std::string kType;

        static const int kSignMultiPropose; // send multi-propose message

        static const int kVoteMultiPropose; // eg. ba0 vote multi-propose

        virtual ~Evidence();

        virtual bool isNull() const;
    };
}