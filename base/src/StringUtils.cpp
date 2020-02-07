#include "base/StringUtils.h"
namespace ultrainio {
    std::string StringUtils::shorten(const std::string& str) {
        if (str.length() < 16) return str;
        return str.substr(0, 8) + "..." + str.substr(str.length() - 8, 8);
    }

    void StringUtils::tokenlize(const std::string& str, char t, std::vector<std::string>& v) {
        size_t currIndex = 0;
        size_t tokenIndex = str.find(t, currIndex);
        size_t len = tokenIndex - currIndex;
        while (tokenIndex != std::string::npos) {
            v.push_back(str.substr(currIndex, len));
            currIndex = tokenIndex + 1;
            tokenIndex = str.find(t, currIndex);
            len = tokenIndex - currIndex;
        }
        if (str.length() > currIndex) {
            v.push_back(str.substr(currIndex));
        }
    }
}