#pragma once

#include <string>

namespace ultrainio { namespace chain {

const int version_num_major = 1;
const int version_num_minor = 0;

inline std::string get_version_str() {
    return (std::to_string(version_num_major) + "." + std::to_string(version_num_minor));
}

} } /// ultrainio::chain