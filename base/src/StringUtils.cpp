#include "base/StringUtils.h"

namespace ultrainio {
    std::string StringUtils::shorten(const std::string& str) {
        if (str.length() < 16) return str;
        return str.substr(0, 8) + "..." + str.substr(str.length() - 8, 8);
    }
}