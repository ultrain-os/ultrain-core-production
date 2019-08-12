#pragma once

#include <string>
#include <ultrainiolib/types.hpp>

namespace ultrainiosystem {
    struct committee_info {
        account_name owner;
        std::string  producer_key;
        std::string  bls_key;

        ULTRAINLIB_SERIALIZE(committee_info, (owner)(producer_key)(bls_key) )

        committee_info() {}
        committee_info(account_name an, const std::string& key, const std::string& bk): owner(an),
                      producer_key(key), bls_key(bk) {}

        bool is_empty() const {
            std::string accountstr = ultrainio::name{owner}.to_string();
            return accountstr.empty() || producer_key.empty() || bls_key.empty();
        }

        void to_strstream(std::string& s) const {
            std::string k_delimiters = std::string(" ");

            std::string accountstr = ultrainio::name{owner}.to_string(); //to remove

            s.append(accountstr).append(k_delimiters);
            s.append(producer_key).append(k_delimiters);
            s.append(bls_key);
        }

        bool from_strstream(const std::string& s, size_t start, size_t& next) {
            std::string k_delimiters = std::string(" ");

            std::size_t found = s.find(k_delimiters, start);
            if (found == std::string::npos) {
                return false;
            }
            std::string acc_name = s.substr(start, found - start);
            owner = ultrainio::string_to_name(acc_name.c_str());

            start = found + k_delimiters.size();

            found = s.find(k_delimiters, start);
            if (found == std::string::npos) {
                return false;
            }
            producer_key = s.substr(start, found - start);

            start = found + k_delimiters.size();

            if (start >= s.size()) {
                return false;
            }

            found = s.find(k_delimiters, start);
            if (found == std::string::npos) {
                bls_key = s.substr(start);
                next = std::string::npos;
            } else {
                bls_key = s.substr(start, found - start);
                next = found + k_delimiters.size();
            }
            return true;
        }

        bool operator == (const committee_info& rhs) const {
            if (this == &rhs) {
                return true;
            }
            return owner == rhs.owner && producer_key == rhs.producer_key && bls_key == rhs.bls_key;
        }
    };
}
