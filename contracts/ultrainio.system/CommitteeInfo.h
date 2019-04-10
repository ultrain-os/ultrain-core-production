#pragma once

#include <string>
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

        void toStrStream(std::string& s) const {
            std::string kDelimiters = std::string(" ");

            std::string accountStr = ultrainio::name{owner}.to_string(); //to remove

            s.append(accountStr).append(kDelimiters);
            s.append(producer_key).append(kDelimiters);
            s.append(bls_key);
        }

        bool fromStrStream(const std::string& s, size_t start, size_t& next) {
            std::string kDelimiters = std::string(" ");

            std::size_t found = s.find(kDelimiters, start);
            if (found == std::string::npos) {
                return false;
            }
            std::string accountName = s.substr(start, found - start);
            owner = ultrainio::string_to_name(accountName.c_str());

            start = found + kDelimiters.size();

            found = s.find(kDelimiters, start);
            if (found == std::string::npos) {
                return false;
            }
            producer_key = s.substr(start, found - start);

            start = found + kDelimiters.size();

            if (start >= s.size()) {
                return false;
            }

            found = s.find(kDelimiters, start);
            if (found == std::string::npos) {
                bls_key = s.substr(start);
                next = std::string::npos;
            } else {
                bls_key = s.substr(start, found - start);
                next = found + kDelimiters.size();
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
