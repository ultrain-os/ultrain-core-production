/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <deque>
#include <fc/network/message_buffer.hpp>
#include "kcp/ikcp.h"
#include "core/protocol.hpp"
namespace ba = boost::asio;
namespace bi = ba::ip;
namespace ultrainio
{
namespace p2p
{
    enum ext_udp_msg_type
    {
       need_tcp_connect = 1
    };
    struct Reserved
    {
        uint32_t key;/*reserve_1:1,reserver_2:2 ...*/
        std::string content;
    };
    struct UnsignedPingNode
    {
        uint8_t type ;
        NodeIPEndpoint source;
        NodeIPEndpoint dest;
        NodeID sourceid; // sender public key (from signature)
        NodeID destid;
        string chain_id;
        chain::public_key_type pk;/*public_key*/
        chain::account_name account;
        vector<Reserved> to_save;
    };
    struct PingNode:public UnsignedPingNode
    {
        string signature;
    };
    /**
 * Pong packet: Sent in response to ping
 */
    struct UnsignedPong
    {
        uint8_t type ;/*2*/
        NodeIPEndpoint destep;
        NodeIPEndpoint fromep;
        NodeID sourceid;
        NodeID destid;
        string chain_id;
        chain::public_key_type pk;/*public_key*/
        chain::account_name account;
        vector<Reserved> to_save;
    };
    struct Pong:public UnsignedPong
    {
        string signature;
    };
    struct UnsignedFindNode
    {
        uint8_t type;/*3*/
        NodeID fromID;
        NodeID targetID;
        NodeID destid;
        NodeIPEndpoint fromep;
        NodeIPEndpoint tartgetep;
        string chain_id;
        chain::public_key_type pk;/*public_key*/
        chain::account_name account;
        vector<Reserved> to_save;
    };
    struct FindNode:public UnsignedFindNode
    {
        string signature;
    };
    struct Neighbour
    {
        NodeIPEndpoint endpoint;
        NodeID node;
    };
    struct UnsignedNeighbours
    {
        uint8_t type ;/*4*/
        NodeID fromID;
        NodeID destid;
        NodeIPEndpoint fromep;
        NodeIPEndpoint tartgetep;
        std::vector<Neighbour> neighbours;
        string chain_id;
        chain::public_key_type pk;/*public_key*/
        chain::account_name account;
        vector<Reserved> to_save;
    };
    struct Neighbours:public  UnsignedNeighbours
    {
        string signature;
    };
       struct UnsignedConnectMsg
    {
        uint8_t type ;
        string peer;
        msg_priority pri;
	NodeID sourceid;
        string chain_id;
        chain::public_key_type pk;/*public_key*/
        chain::account_name account;
        vector<Reserved> to_save;
    };
    struct ConnectMsg:public UnsignedConnectMsg
    {
        string signature;
    };
    struct UnsignedConnectAckMsg
    {
        uint8_t type ;
        string peer;
	NodeID sourceid;
        msg_priority pri;
        string chain_id;
        chain::public_key_type pk;/*public_key*/
        chain::account_name account;
        kcp_conv_t conv;
        vector<Reserved> to_save;
    };
    struct ConnectAckMsg:public UnsignedConnectAckMsg
    {
        string signature;
    };
    struct UnsignedNewPing
    {
        uint8_t type ;
        NodeIPEndpoint source;
        NodeIPEndpoint dest;
        NodeID sourceid; // sender public key (from signature)
        NodeID destid;
        string chain_id;
        msg_priority pri;
        chain::public_key_type pk;/*public_key*/
        chain::account_name account;
        vector<Reserved> to_save;
    };
    struct NewPing:public UnsignedNewPing
    {
        string signature;
    };
        struct UnsignedSessionCloseMsg
    {
        uint8_t type ;
        NodeID sourceid;
        string chain_id;
        chain::public_key_type pk;/*public_key*/
        chain::account_name account;
        kcp_conv_t conv;
        bool todel;
        vector<Reserved> to_save;
    };
    struct SessionCloseMsg:public UnsignedSessionCloseMsg
    {
        string signature;
    };
    using udp_msg = fc::static_variant<PingNode,
            Pong,
            FindNode,
            Neighbours,
	    NewPing,
	    ConnectMsg,
	    ConnectAckMsg,
	    SessionCloseMsg>;


/**
 * @brief Interface which a UDPSocket's owner must implement.
 */
struct UDPSocketEvents
{
    virtual ~UDPSocketEvents() = default;
    virtual void onSocketDisconnected() {}
    virtual void handlemsg( bi::udp::endpoint const& _from, PingNode const& pingmsg ) = 0;
    virtual void handlemsg( bi::udp::endpoint const& _from, Pong const& pongmsg ) = 0;
    virtual void handlemsg( bi::udp::endpoint const& _from, FindNode const& findnodemsg ) = 0;
    virtual void handlemsg( bi::udp::endpoint const& _from, Neighbours const& msg ) = 0;
    virtual void handlemsg( bi::udp::endpoint const& _from, NewPing const& msg ) = 0;
    virtual void handlemsg( bi::udp::endpoint const& _from, ConnectMsg const& msg ) = 0;
    virtual void handlemsg( bi::udp::endpoint const& _from, ConnectAckMsg const& msg ) = 0;
    virtual void handlemsg( bi::udp::endpoint const& _from, SessionCloseMsg const& msg ) = 0;
    virtual void handlekcpmsg(const char *data,size_t bytes_recvd) = 0;
};
    using std::vector;

    struct udpmsgHandler : public fc::visitor<void> {
        UDPSocketEvents &impl;
      bi::udp::endpoint &loep;
      udpmsgHandler( UDPSocketEvents &imp,bi::udp::endpoint &ep) : impl(imp), loep(ep) {}

        template <typename T> void operator()(const T &msg) const
        {
            impl.handlemsg(loep, msg);
        }
        void handlekcpmsg(const char *data,size_t bytes_recvd) const
	{
	    impl.handlekcpmsg(data,bytes_recvd);
	}
    };
constexpr auto     udpmessage_header_size = 4;
constexpr auto     kcpmessage_header_size = 20;
constexpr auto     def_send_buffer_size_mb = 4;
constexpr auto     def_send_buffer_size = 1024*1024*def_send_buffer_size_mb;
/**
 * @brief UDP socket
 *
 */
template <typename Handler, unsigned MaxDatagramSize>
class UDPSocket: public std::enable_shared_from_this<UDPSocket<Handler, MaxDatagramSize>>
{
public:
    enum { maxDatagramSize = MaxDatagramSize };
    static_assert((unsigned)maxDatagramSize < 65507u, "UDP datagrams cannot be larger than 65507 bytes");

    /// Create socket for specific endpoint.
    UDPSocket(ba::io_service& _io, UDPSocketEvents& _host, bi::udp::endpoint _endpoint): m_host(_host), m_endpoint(_endpoint), m_socket(_io) { m_started.store(false); m_closed.store(true); };

    /// Create socket which listens to all ports.
    UDPSocket(ba::io_service& _io, UDPSocketEvents& _host, unsigned _port): m_host(_host), m_endpoint(bi::udp::v4(), _port), m_socket(_io) { m_started.store(false); m_closed.store(true); };
    virtual ~UDPSocket() { disconnect(); }

    /// Socket will begin listening for and delivering packets
    void connect();

    /// Send datagram.
    void send_kcp_packet(const char* msg,int len, const boost::asio::ip::udp::endpoint& endpoint);
    void send_msg(udp_msg const& m, bi::udp::endpoint const& ep);
    /// Returns if socket is open.
    bool isOpen() { return !m_closed; }

    /// Disconnect socket.
    void disconnect() { disconnectWithError(boost::asio::error::connection_reset); }
    void reset_pktlimit_monitor();

protected:
    void doRead();

    void doWrite();
    bool process_next_message(bi::udp::endpoint &m_recvEndpoint,uint32_t message_length);
    void disconnectWithError(boost::system::error_code _ec);

    std::atomic<bool> m_started;
    std::atomic<bool> m_closed;

    UDPSocketEvents& m_host;						///< Interface which owns this socket.
    bi::udp::endpoint m_endpoint;					///< Endpoint which we listen to.

    bi::udp::endpoint m_recvEndpoint;				///< Endpoint data was received from.
    bi::udp::socket m_socket;						///< Boost asio udp socket.

    boost::system::error_code m_socketError;		///< Set when shut down due to error.
    struct queued_write {
        std::shared_ptr<vector<char>> buff;
        bi::udp::endpoint endpoint;/*remote endpoint*/
    };
    std::deque<queued_write>     write_queue;
    fc::message_buffer<4*1024*1024>    pending_message_buffer;
    fc::optional<std::size_t>        outstanding_read_bytes;
    uint32_t pack_count_rcv = 0;
    uint32_t pack_count_drop = 0;
    bool check_pkt_limit_exceed();
};

template <typename Handler, unsigned MaxDatagramSize>
void UDPSocket<Handler, MaxDatagramSize>::connect()
{
    bool expect = false;
    if (!m_started.compare_exchange_strong(expect, true))
        return;
    m_socket.open(bi::udp::v4());
    try
    {
        m_socket.bind(m_endpoint);
    }
    catch (...)
    {
        m_socket.bind(bi::udp::endpoint(bi::udp::v4(), m_endpoint.port()));
    }

    // clear write queue so reconnect doesn't send stale messages
    write_queue.clear();

    m_closed = false;
    doRead();
}
/*kcp pack segment,conv is much bigger than normal udp payloadsize
0              4     5    (BYTE)
+--------------+-----|----------
| conv         | cmd | .... |
*/
template <typename Handler, unsigned MaxDatagramSize>
void UDPSocket<Handler, MaxDatagramSize>::send_kcp_packet(const char* msg,int len, const boost::asio::ip::udp::endpoint& endpoint) {
   auto send_buffer = std::make_shared<vector<char>>(len);
   fc::datastream<char*> ds( send_buffer->data(), len);
   ds.write(msg,len);
   write_queue.push_back({send_buffer,endpoint});
   if (write_queue.size() ==1)
       doWrite();
}
/*udp msg segment
0              4   (BYTE)
+--------------+-----------
| payloadsize  |  msg
*/
template <typename Handler, unsigned MaxDatagramSize>
void UDPSocket<Handler, MaxDatagramSize>::send_msg(udp_msg const& m, bi::udp::endpoint const& ep)
{
    uint32_t payload_size = fc::raw::pack_size( m );
    char * header = reinterpret_cast<char*>(&payload_size);
    size_t header_size = sizeof(payload_size);

    size_t buffer_size = header_size + payload_size;
    auto send_buffer = std::make_shared<vector<char>>(buffer_size);
    fc::datastream<char*> ds( send_buffer->data(), buffer_size);
    ds.write( header, header_size );
    fc::raw::pack( ds, m );

    write_queue.push_back({send_buffer,ep});

     if (write_queue.size() == 1)
           doWrite();
}
template <typename Handler, unsigned MaxDatagramSize>
void UDPSocket<Handler, MaxDatagramSize>::doRead()
{
   try {
    if (m_closed)
        return;
    std::weak_ptr<UDPSocket<Handler, MaxDatagramSize>> self =UDPSocket<Handler, MaxDatagramSize>::shared_from_this();
    m_socket.async_receive_from(pending_message_buffer.get_buffer_sequence_for_boost_async_read(), m_recvEndpoint, [this, self](boost::system::error_code _ec, size_t _len)
    {
        auto conn = self.lock();
        if(!conn)
        {
            elog("closed");
            return ;
        }
        if (m_closed)
            return disconnectWithError(_ec);
     try{
        if (_ec != boost::system::errc::success)
            elog("Receiving UDP message failed. ");
            outstanding_read_bytes.reset();
            if( !_ec ) {
                if(_len>pending_message_buffer.bytes_to_write())
                {
                   pending_message_buffer.reset();
                   doRead();
                   return; 
                }
                pending_message_buffer.advance_write_ptr(_len);
                while (pending_message_buffer.bytes_to_read() > 0) {
                    uint32_t bytes_in_buffer = pending_message_buffer.bytes_to_read();
                    if (bytes_in_buffer < udpmessage_header_size) {
                        outstanding_read_bytes.emplace(udpmessage_header_size - bytes_in_buffer);
                        break;
                    }
                    else {
                        bool is_kcp_pkt = false;
                        auto index = pending_message_buffer.read_index();
                        uint32_t conv;
                        uint8_t cmd;
                        pending_message_buffer.peek(&conv, sizeof(conv), index);
                        pending_message_buffer.peek(&cmd, sizeof(cmd), index);
                        if(conv > def_send_buffer_size*2
                                &&(cmd == 81 || cmd == 82 || cmd == 83 || cmd == 84)) {
                            is_kcp_pkt = true;
                        }
                        if(is_kcp_pkt){
                            index = pending_message_buffer.read_index();
                            vector<char> kcp_header;
                            kcp_header.resize(kcpmessage_header_size);
                            pending_message_buffer.peek(kcp_header.data(),kcpmessage_header_size,index);
                            uint32_t kcp_data_len;
                            pending_message_buffer.peek(&kcp_data_len, sizeof(kcp_data_len), index);
                            auto total_len = kcpmessage_header_size +4 +kcp_data_len;

                            index = pending_message_buffer.read_index();
                            kcp_header.resize(total_len);
                            pending_message_buffer.peek(kcp_header.data(),total_len,index);
                            udpmsgHandler m(m_host, m_recvEndpoint );
                            m.handlekcpmsg(kcp_header.data(),total_len);
                            pending_message_buffer.advance_read_ptr(total_len);
                        }
                        else
                        {
                            uint32_t message_length;
                            index = pending_message_buffer.read_index();
                            pending_message_buffer.peek(&message_length, sizeof(message_length), index);

                            if(message_length > def_send_buffer_size*2 || message_length == 0) {
                                elog("incoming message length unexpected (${i})", ("i", message_length));
                                pending_message_buffer.reset();
                                break;
                            }

                            auto total_message_bytes = message_length + udpmessage_header_size;
                            if (bytes_in_buffer >= total_message_bytes) {
                                pending_message_buffer.advance_read_ptr(udpmessage_header_size);
                                if (!process_next_message(m_recvEndpoint,message_length)) {
                                    pending_message_buffer.advance_read_ptr_from_index(index, message_length);
                                    break;
                                }
                            } else {
                                auto outstanding_message_bytes = total_message_bytes - bytes_in_buffer;
                                auto available_buffer_bytes = pending_message_buffer.bytes_to_write();
                                if (outstanding_message_bytes > available_buffer_bytes) {
                                    pending_message_buffer.add_space( outstanding_message_bytes - available_buffer_bytes );
                                }

                                outstanding_read_bytes.emplace(outstanding_message_bytes);
                                break;
                            }
                        }
                    }
                }
            }

        doRead();
     }
     catch(const std::exception &ex) {
        elog("exception in doRead");
        pending_message_buffer.reset();
        doRead();
     }
     catch(const fc::exception &ex) {
        elog("exception in doRead");
        pending_message_buffer.reset();
        doRead();
     }
     catch (...) {
        elog("exception in doRead");
        pending_message_buffer.reset();
        doRead();
     }
    });
   } catch (...) {
        elog("exception in doRead");
        pending_message_buffer.reset();
        doRead();
   }

}
template <typename Handler, unsigned MaxDatagramSize>
void UDPSocket<Handler, MaxDatagramSize>::reset_pktlimit_monitor( )
{

    ilog("pack_udp count_rcv ${counnt_rcv} count_drop ${count_drop}",
            ("counnt_rcv",pack_count_rcv)
            ("count_drop",pack_count_drop));
    pack_count_rcv = 0;
    pack_count_drop = 0;
}
template <typename Handler, unsigned MaxDatagramSize>
bool UDPSocket<Handler, MaxDatagramSize>::check_pkt_limit_exceed() {
    static int count_threhold = 2000;
    pack_count_rcv ++;
    if(pack_count_rcv > count_threhold)
    {
        pack_count_drop++;
        return true;
    }
    return false;
}
template <typename Handler, unsigned MaxDatagramSize>
bool UDPSocket<Handler, MaxDatagramSize>::process_next_message(bi::udp::endpoint &m_recvEndpoint,uint32_t message_length) {
    try {
        auto ds = pending_message_buffer.create_datastream();
        udp_msg msg;
        fc::raw::unpack(ds, msg);
        bool isexceed = check_pkt_limit_exceed();
        if(isexceed)
        {
            return true;
        }
        udpmsgHandler m(m_host, m_recvEndpoint );
        msg.visit(m);
    } catch(  const fc::exception& e ) {
	    edump((e.to_detail_string() ));
	    return false;
    }
    catch(const std::exception& e)
    {
	    elog( "error: ${e}", ("e",e.what()));
	    return false;
    }
    catch(...)
    {
	    elog("process_next error,unkown");
	    return false;
    }
    return true;
}
template <typename Handler, unsigned MaxDatagramSize>
void UDPSocket<Handler, MaxDatagramSize>::doWrite() {
    if(write_queue.empty() )
        return;
    if (m_closed)
        return;
    auto& m = write_queue.front();
    bi::udp::endpoint endpoint(m.endpoint);
     std::weak_ptr<UDPSocket<Handler, MaxDatagramSize>> self =UDPSocket<Handler, MaxDatagramSize>::shared_from_this();
    m_socket.async_send_to( boost::asio::buffer(*m.buff), endpoint, [this, self, endpoint](boost::system::error_code _ec, std::size_t)
    {
         auto conn = self.lock();
         if(!conn)
         {
             elog("closed");
             return ;
         }
        if (m_closed)
            return disconnectWithError(_ec);

        if (_ec != boost::system::errc::success)
        {
            elog("Failed delivering UDP message.${ec}",("ec", _ec.message()));
        }

        write_queue.pop_front();
        if (write_queue.empty())
            return;
        doWrite();
    });

}
template <typename Handler, unsigned MaxDatagramSize>
void UDPSocket<Handler, MaxDatagramSize>::disconnectWithError(boost::system::error_code _ec)
{
    // If !started and already stopped, shutdown has already occured. (EOF or Operation canceled)
    if (!m_started && m_closed && !m_socket.is_open() /* todo: veirfy this logic*/)
        return;

    assert(_ec);
    {
        if (m_socketError != boost::system::error_code())
            return;
        m_socketError = _ec;
    }

    bool expected = true;
    if (!m_started.compare_exchange_strong(expected, false))
        return;

    // set m_closed to true to prevent undeliverable egress messages
    bool wasClosed = m_closed;
    m_closed = true;

    // close sockets
    boost::system::error_code ec;
    m_socket.shutdown(bi::udp::socket::shutdown_both, ec);
    m_socket.close();
    write_queue.clear();
    pending_message_buffer.reset();
    // socket never started if it never left stopped-state (pre-handshake)
    if (wasClosed)
        return;

    m_host.onSocketDisconnected();
}

}

}
FC_REFLECT( ultrainio::p2p::Reserved, (key)(content))
FC_REFLECT( ultrainio::p2p::UnsignedPingNode, (sourceid)(destid)(type)(source)(dest)(chain_id)(pk)(account)(to_save))
FC_REFLECT_DERIVED( ultrainio::p2p::PingNode, (ultrainio::p2p::UnsignedPingNode), (signature))
FC_REFLECT( ultrainio::p2p::UnsignedPong, (type)(fromep)(destep)(sourceid)(destid)(chain_id)(pk)(account)(to_save))
FC_REFLECT_DERIVED( ultrainio::p2p::Pong, (ultrainio::p2p::UnsignedPong), (signature))
FC_REFLECT( ultrainio::p2p::UnsignedFindNode, (type)(fromID)(targetID)(destid)(fromep)(tartgetep)(chain_id)(pk)(account)(to_save))
FC_REFLECT_DERIVED( ultrainio::p2p::FindNode, (ultrainio::p2p::UnsignedFindNode), (signature))
FC_REFLECT( ultrainio::p2p::Neighbour, (node)(endpoint))
FC_REFLECT( ultrainio::p2p::UnsignedNeighbours, (type)(fromID)(destid)(fromep)(tartgetep)(neighbours)(chain_id)(pk)(account)(to_save))
FC_REFLECT_DERIVED( ultrainio::p2p::Neighbours, (ultrainio::p2p::UnsignedNeighbours), (signature))
FC_REFLECT( ultrainio::p2p::UnsignedConnectMsg, (type)(peer)(pri)(sourceid)(chain_id)(pk)(account)(to_save))
FC_REFLECT_DERIVED( ultrainio::p2p::ConnectMsg, (ultrainio::p2p::UnsignedConnectMsg), (signature))
FC_REFLECT( ultrainio::p2p::UnsignedConnectAckMsg, (type)(peer)(pri)(sourceid)(chain_id)(pk)(conv)(account)(to_save))
FC_REFLECT_DERIVED( ultrainio::p2p::ConnectAckMsg, (ultrainio::p2p::UnsignedConnectAckMsg), (signature))
FC_REFLECT( ultrainio::p2p::UnsignedNewPing, (sourceid)(destid)(type)(source)(dest)(chain_id)(pri)(pk)(account)(to_save))
FC_REFLECT_DERIVED( ultrainio::p2p::NewPing, (ultrainio::p2p::UnsignedNewPing), (signature))
FC_REFLECT( ultrainio::p2p::UnsignedSessionCloseMsg, (type)(sourceid)(chain_id)(pk)(account)(conv)(todel)(to_save))
FC_REFLECT_DERIVED( ultrainio::p2p::SessionCloseMsg, (ultrainio::p2p::UnsignedSessionCloseMsg), (signature))
