/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <set>
#include <string>
#include <memory>
#include <thread>

struct UPNPUrls;
struct IGDdatas;

namespace ultrainio {
    namespace p2p {

        class UPnP {
        public:
            UPnP();

            ~UPnP();

            std::string externalIP();

            int addRedirect(char const *addr, int port);

            void removeRedirect(int port);

            bool isValid() const { return m_ok; }

        private:
            std::set<int> m_reg;
            bool m_ok;
            std::shared_ptr<UPNPUrls> m_urls;
            std::shared_ptr<IGDdatas> m_data;
        };
    }
}
