#pragma once

#include <string>
#include <vector>

namespace ultrainio {
    class StringUtils {
    public:
        static std::string shorten(const std::string& str);

        static void split(const std::string& str, char token, std::vector<std::string>& outV);

        static std::string paddingPrefixZero(const std::string& str, size_t finalSize);
    };
}