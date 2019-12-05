/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <atomic>
#include <string>
#include <vector>
#include <chrono>
#include <core/protocol.hpp>

namespace ba = boost::asio;
namespace bi = boost::asio::ip;
namespace ultrainio
{

    namespace p2p
    {
        using NodeID = fc::sha256;
	enum ext_udp_msg_type
        {
            need_tcp_connect = 1,
            nat_type = 2
        };
        enum nat_type{
            type_none=0,
            full_cone =1,
            ip_restrict_cone,
            port_restrict_cone,
            symmetric
        };

        class Node;

        struct NodeIPEndpoint
        {
            operator bi::udp::endpoint() const { return bi::udp::endpoint(boost::asio::ip::address::from_string(m_address), m_udpPort); }

            operator bool() const {
                return !boost::asio::ip::address::from_string(m_address).is_unspecified() && m_udpPort > 0;
            }

            // NOTE: the listen port in m_listenPorts must be in the same sequence
            bool operator==(NodeIPEndpoint const& _cmp) const {
                if (m_address == _cmp.m_address && m_udpPort == _cmp.m_udpPort && m_listenPorts.size() == _cmp.m_listenPorts.size()) {
                    for (size_t i = 0; i < m_listenPorts.size(); i++) {
                        if (m_listenPorts[i].port != _cmp.m_listenPorts[i].port || m_listenPorts[i].business_type != _cmp.m_listenPorts[i].business_type) {
                            return false;
                        }
                    }
                    return true;
                } else {
                    return false;
                }
            }

            bool operator!=(NodeIPEndpoint const& _cmp) const {
                return !operator==(_cmp);
            }

            std::string m_address;
            uint16_t m_udpPort = 0;
            std::vector<listen_port> m_listenPorts;

            std::string address() const { return m_address; }

            void setAddress(const std::string& _addr) { m_address = _addr; }

            uint16_t udpPort() const { return m_udpPort; }

            void setUdpPort(uint16_t _udp) { m_udpPort = _udp; }

            uint16_t listenPort(msg_priority businessType) const {
                for (auto& p : m_listenPorts) {
                    if (p.business_type == businessType) {
                        return p.port;
                    }
                }

                return 0;
            }

            void setListenPort(msg_priority businessType, uint16_t port) {
                for (auto& p : m_listenPorts) {
                    if (p.business_type == businessType) {
                        p.port = port;
                        return;
                    }
                }
                m_listenPorts.emplace_back(port, businessType);
            }
	    void set_ext_nat_type(uint32_t nat_type){
                if(nat_type != p2p::nat_type::type_none){
                    ext.push_back({ext_udp_msg_type::nat_type,to_string(nat_type)});
                }
            }
            uint32_t get_ext_nat_type() const{
                for(auto& ex:ext){
                    if(ex.key == ext_udp_msg_type::nat_type){
                        return std::atoi(ex.value.c_str());
                    }
                }
                return nat_type::type_none;
            }
            MsgExtension ext;
        };

        class Node
        {
        friend class NodeTable;
        public:
            Node() = default;
            virtual ~Node() = default;
            Node(NodeID _id, NodeIPEndpoint const& _ip): m_id(_id), m_endpoint(_ip) {}

            bool valid() const { return m_id != fc::sha256(); }

            NodeID id() const { return m_id; }

            void setId(const NodeID& _id) { m_id = _id; }

            NodeIPEndpoint endPoint() const { return m_endpoint; }

            void setEndPoint(const NodeIPEndpoint& _endpoint) { m_endpoint = _endpoint; }
        private:
            NodeID m_id;
            NodeIPEndpoint m_endpoint;
        };
    }

}
FC_REFLECT( ultrainio::p2p::NodeIPEndpoint, (m_address)(m_udpPort)(m_listenPorts)(ext))
#if 1
namespace std
{

template <> struct hash<bi::address>
{
    size_t operator()(bi::address const& _a) const
    {
        if (_a.is_v4())
            return std::hash<unsigned long>()(_a.to_v4().to_ulong());
        if (_a.is_v6())
        {
            auto const& range = _a.to_v6().to_bytes();
            return boost::hash_range(range.begin(), range.end());
        }
        if (_a.is_unspecified())
            return static_cast<size_t>(0x3487194039229152ull);  // Chosen by fair dice roll, guaranteed to be random
        return std::hash<std::string>()(_a.to_string());
    }
};

}
#endif
