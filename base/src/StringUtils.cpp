#include "base/StringUtils.h"
namespace ultrainio {
    std::string StringUtils::shorten(const std::string& str) {
        if (str.length() < 16) return str;
        return str.substr(0, 8) + "..." + str.substr(str.length() - 8, 8);
    }

    void StringUtils::split(const std::string& str, char token, std::vector<std::string>& outV) {
        size_t currIndex = 0;
        size_t tokenIndex = str.find(token, currIndex);
        size_t len = tokenIndex - currIndex;
        while (tokenIndex != std::string::npos) {
            outV.push_back(str.substr(currIndex, len));
            currIndex = tokenIndex + 1;
            tokenIndex = str.find(token, currIndex);
            len = tokenIndex - currIndex;
        }
        if (str.length() > currIndex) {
            outV.push_back(str.substr(currIndex));
        }
    }

    std::string StringUtils::paddingPrefixZero(const std::string& str, size_t finalSize) {
        std::string result;
        std::string zero = "0";
        if (str.length() < finalSize) {
            size_t n = finalSize - str.length();
            // TODO(xiaofen.qin@gmail.com
            for (int i = 0; i < n; i++) {
                result.append(zero);
            }
        }
        result.append(str);
        return result;
    }
}