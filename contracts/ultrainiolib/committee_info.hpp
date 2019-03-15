/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#pragma once
#include <string>
#include <sstream>

//don't use namespace ultrainio, otherwise the stringstream >> will has conflict with ultrainio::datastream >>
//so here a different namespace should be used
namespace ultrainstd {
    struct CommitteeInfo {
        std::string accountName;
        std::string pk;
        std::string blsPk;

        CommitteeInfo() {}
        CommitteeInfo(const std::string& an, const std::string& key, const std::string& bls_key): accountName(an),
                      pk(key), blsPk(bls_key) {}
        bool isEmpty() const {
            return accountName.empty() || pk.empty() || blsPk.empty();
        }

        void toStrStream(std::stringstream& ss) const {
            if(isEmpty()) {
                return;
            }
            ss << accountName << " ";
            ss << pk << " ";
            ss << blsPk;
        }

        bool fromStrStream(std::stringstream& ss) {
 
            if(!(ss >> accountName)) {
                return false;
            }
            if(!(ss >> pk)) {
                return false;
            }
            if(!(ss >> blsPk)) {
                return false;
            }

            return true;
        }

        bool operator == (const CommitteeInfo& rhs) const {
            if (this == &rhs) {
                return true;
            }
            return accountName == rhs.accountName && pk == rhs.pk && blsPk == rhs.blsPk;
        }
    };
}
