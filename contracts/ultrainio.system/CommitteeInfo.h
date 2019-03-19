#pragma once

#include <string>
#include <sstream>
#include <ultrainiolib/types.hpp>

namespace ultrainiosystem {
    struct CommitteeInfo {
        account_name owner;
        std::string  producer_key;
        std::string  bls_key;

        ULTRAINLIB_SERIALIZE(CommitteeInfo, (owner)(producer_key)(bls_key) )

        CommitteeInfo() {}
        CommitteeInfo(account_name an, const std::string& key, const std::string& bk): owner(an),
                      producer_key(key), bls_key(bk) {}

        bool isEmpty() const {
            std::string accountStr = ultrainio::name{owner}.to_string();
            return accountStr.empty() || producer_key.empty() || bls_key.empty();
        }

        void toStrStream(std::stringstream& ss) const {
            if(isEmpty()) {
                return;
            }
            std::string accountStr = ultrainio::name{owner}.to_string(); //to remove
            ss << accountStr << " ";    //to remove
            //ss << owner << " ";
            ss << producer_key << " ";
            ss << bls_key;
        }

        bool fromStrStream(std::stringstream& ss) {
            std::string accountStr;
            if(!(ss >> accountStr)) {
                return false;
            }
            owner = ultrainio::string_to_name(accountStr.c_str());
            //to remove above
            /*
            if(!(ss >> owner)) {
                return false;
            }*/
            if(!(ss >> producer_key)) {
                return false;
            }
            if(!(ss >> bls_key)) {
                return false;
            }

            return true;
        }

        bool operator == (const CommitteeInfo& rhs) const {
            if (this == &rhs) {
                return true;
            }
            return owner == rhs.owner && producer_key == rhs.producer_key && bls_key == rhs.bls_key;
        }
    };
}
