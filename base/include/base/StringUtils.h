#pragma once

#include <string>

namespace ultrainio {
    class StringUtils {
    public:
        static std::string shorten(const std::string& str);
    };
}