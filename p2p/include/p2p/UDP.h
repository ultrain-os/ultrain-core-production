#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <deque>
#include <fc/network/message_buffer.hpp>

namespace ba = boost::asio;
namespace bi = ba::ip;

namespace ultrainio
{
namespace p2p
{
    struct UnsignedPingNode
    {
        uint8_t type ;
        NodeIPEndpoint source;
        NodeIPEndpoint dest;
        NodeID sourceid; // sender public key (from signature)
        NodeID destid;
        string chain_id;
        chain::public_key_type pk;/*public_key*/
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
	NodeID sourceid; // sender public key (from signature)
	NodeID destid;
	string chain_id;
        chain::public_key_type pk;/*public_key*/
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
    };
    struct Neighbours:public  UnsignedNeighbours
    {
        string signature;
    };
    using udp_msg = fc::static_variant<PingNode,
            Pong,
            FindNode,
            Neighbours>;


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
    };
constexpr auto     udpmessage_header_size = 4;
constexpr auto     def_send_buffer_size_mb = 4;
constexpr auto     def_send_buffer_size = 1024*1024*def_send_buffer_size_mb;
/**
 * @brief UDP Interface
 * Handler must implement UDPSocketEvents.
 *
 * @todo multiple endpoints (we cannot advertise 0.0.0.0)
 * @todo decouple deque from UDPDatagram and add ref() to datagram for fire&forget
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
  //  bool send(UDPDatagram const& _datagram);
    void send_msg(udp_msg const& m, bi::udp::endpoint const& ep);
    /// Returns if socket is open.
    bool isOpen() { return !m_closed; }

    /// Disconnect socket.
    void disconnect() { disconnectWithError(boost::asio::error::connection_reset); }

protected:
    void doRead();

    void doWrite();
    bool process_next_message(bi::udp::endpoint &m_recvEndpoint);
    void disconnectWithError(boost::system::error_code _ec);

    std::atomic<bool> m_started;					///< Atomically ensure connection is started once. Start cannot occur unless m_started is false. Managed by start and disconnectWithError.
    std::atomic<bool> m_closed;					///< Connection availability.

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
  //  Guard l(x_sendQ);
    write_queue.clear();

    m_closed = false;
    doRead();
}

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
       // ilog("doRead from address ${address} ${port}",("address",m_recvEndpoint.address().to_string())("port",std::to_string(m_recvEndpoint.port()))); 
        if (_ec != boost::system::errc::success)
            elog("Receiving UDP message failed. ");
            outstanding_read_bytes.reset();
            if( !_ec ) {
                if(_len>pending_message_buffer.bytes_to_write())
                {
                    
                }
                pending_message_buffer.advance_write_ptr(_len);
                while (pending_message_buffer.bytes_to_read() > 0) {
                    uint32_t bytes_in_buffer = pending_message_buffer.bytes_to_read();
                    if (bytes_in_buffer < udpmessage_header_size) {
                        outstanding_read_bytes.emplace(udpmessage_header_size - bytes_in_buffer);
                        break;
                    }
                    else {
                         uint32_t message_length;
                        auto index = pending_message_buffer.read_index();
                        pending_message_buffer.peek(&message_length, sizeof(message_length), index);

                        if(message_length > def_send_buffer_size*2 || message_length == 0) {
                            elog("incoming message length unexpected (${i})", ("i", message_length));
                            return;
                        }

                        auto total_message_bytes = message_length + udpmessage_header_size;
                        if (bytes_in_buffer >= total_message_bytes) {
                            pending_message_buffer.advance_read_ptr(udpmessage_header_size);
                            if (!process_next_message(m_recvEndpoint)) {
                                return;
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

        doRead();
    });
}
template <typename Handler, unsigned MaxDatagramSize>
bool UDPSocket<Handler, MaxDatagramSize>::process_next_message(bi::udp::endpoint &m_recvEndpoint) {
    try {
        auto index = pending_message_buffer.read_index();
        uint64_t which = 0; char b = 0; uint8_t by = 0;
        do {
            pending_message_buffer.peek(&b, 1, index);
            which |= uint32_t(uint8_t(b) & 0x7f) << by;
            by += 7;
        } while( uint8_t(b) & 0x80 && by < 32);

        auto ds = pending_message_buffer.create_datastream();
        udp_msg msg;
        fc::raw::unpack(ds, msg);

        udpmsgHandler m(m_host, m_recvEndpoint );
        msg.visit(m);
    } catch(  const fc::exception& e ) {
        edump((e.to_detail_string() ));

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
    auto self(UDPSocket<Handler, MaxDatagramSize>::shared_from_this());
    m_socket.async_send_to( boost::asio::buffer(*m.buff), endpoint, [this, self, endpoint](boost::system::error_code _ec, std::size_t)
    {
        if (m_closed)
            return disconnectWithError(_ec);

        if (_ec != boost::system::errc::success)
        {
            elog("Failed delivering UDP message.");
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
        // disconnect-operation following prior non-zero errors are ignored
   //     Guard l(x_socketError);
        if (m_socketError != boost::system::error_code())
            return;
        m_socketError = _ec;
    }
    // TODO: (if non-zero error) schedule high-priority writes

    // prevent concurrent disconnect
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

    // socket never started if it never left stopped-state (pre-handshake)
    if (wasClosed)
        return;

    m_host.onSocketDisconnected();
}

}

}
//FC_REFLECT( ultrainio::p2p::UDPDatagram, (data)(locus_ip)(locus_port))
FC_REFLECT( ultrainio::p2p::UnsignedPingNode, (sourceid)(destid)(type)(source)(dest)(chain_id)(pk))
FC_REFLECT_DERIVED( ultrainio::p2p::PingNode, (ultrainio::p2p::UnsignedPingNode), (signature))
FC_REFLECT( ultrainio::p2p::UnsignedPong, (type)(fromep)(destep)(sourceid)(destid)(chain_id)(pk))
FC_REFLECT_DERIVED( ultrainio::p2p::Pong, (ultrainio::p2p::UnsignedPong), (signature))
FC_REFLECT( ultrainio::p2p::UnsignedFindNode, (type)(fromID)(targetID)(destid)(fromep)(tartgetep)(chain_id)(pk))
FC_REFLECT_DERIVED( ultrainio::p2p::FindNode, (ultrainio::p2p::UnsignedFindNode), (signature))
FC_REFLECT( ultrainio::p2p::Neighbour, (node)(endpoint))
FC_REFLECT( ultrainio::p2p::UnsignedNeighbours, (type)(fromID)(destid)(fromep)(tartgetep)(neighbours)(chain_id)(pk))
FC_REFLECT_DERIVED( ultrainio::p2p::Neighbours, (ultrainio::p2p::UnsignedNeighbours), (signature))
