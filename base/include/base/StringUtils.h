#pragma once

#include <string>
#include <vector>

namespace ultrainio {
    class StringUtils {
    public:
        static std::string shorten(const std::string& str);

        static void tokenlize(const std::string& str, char t, std::vector<std::string>& v);
    };
}