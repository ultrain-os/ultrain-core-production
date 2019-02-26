#pragma once

#include <string>

#include <fc/reflect/reflect.hpp>
#include <fc/variant.hpp>

namespace ultrainio {
    struct CommitteeInfo {
        std::string accountName;
        std::string pk;
        std::string blsPk;
        bool isEmpty();
        void toVariants(fc::variants& vs) const;
        void fromVariants(const fc::variants& vs);
        bool operator == (const CommitteeInfo& rhs) const;
    };
}

FC_REFLECT( ultrainio::CommitteeInfo, (accountName)(pk)(blsPk))
