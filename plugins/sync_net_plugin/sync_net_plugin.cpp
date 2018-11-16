/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include <ultrainio/sync_net_plugin/sync_net_plugin.hpp>
#include <ultrainio/sync_net_plugin/protocol.hpp>

#include <fc/network/message_buffer.hpp>
#include <fc/network/ip.hpp>
#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>
#include <fc/log/appender.hpp>
#include <fc/container/flat.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/crypto/rand.hpp>
#include <fc/exception/exception.hpp>

#include <ultrainio/chain/exceptions.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/intrusive/set.hpp>

#include <random>

namespace fc {
    extern std::unordered_map<std::string,logger>& get_logger_map();
}

namespace ultrainio {
    static appbase::abstract_plugin& _sync_net_plugin = app().register_plugin<sync_net_plugin>();

    using namespace wss;
    using std::vector;

    using boost::asio::ip::tcp;
    using boost::asio::ip::address_v4;
    using boost::asio::ip::host_name;
    using boost::intrusive::rbtree;
    using boost::multi_index_container;

    using fc::time_point;
    using fc::time_point_sec;

    namespace bip = boost::interprocess;

    class connection;

    using connection_ptr = std::shared_ptr<connection>;
    using connection_wptr = std::weak_ptr<connection>;

    using socket_ptr = std::shared_ptr<tcp::socket>;

    using net_message_ptr = std::shared_ptr<net_message>;

    template<typename I> std::string itoh(I n, size_t hlen = sizeof(I)<<1) {
        static const char* digits = "0123456789abcdef";
        std::string r(hlen, '0');
        for(size_t i = 0, j = (hlen - 1) * 4 ; i < hlen; ++i, j -= 4)
            r[i] = digits[(n>>j) & 0x0f];
        return r;
    }

    class sync_net_plugin_impl {
    public:
        std::unique_ptr<tcp::acceptor>        acceptor;
        tcp::endpoint                    listen_endpoint;
        string                           p2p_address;
        uint32_t                         max_client_count = 0;
        uint32_t                         max_nodes_per_host = 1;
        uint32_t                         num_clients = 0;

        vector<string>                   supplied_peers;

        enum possible_connections : char {
            None = 0,
            Producers = 1 << 0,
            Specified = 1 << 1,
            Any = 1 << 2
        };
        possible_connections             allowed_connections{None};

        connection_ptr find_connection( string host )const;

        std::set< connection_ptr >       connections;
        bool                             done = false;

        std::unique_ptr<boost::asio::steady_timer> connector_check;
        std::unique_ptr<boost::asio::steady_timer> keepalive_timer;

        boost::asio::steady_timer::duration   connector_period;
        boost::asio::steady_timer::duration   resp_expected_period;
        boost::asio::steady_timer::duration   keepalive_interval{std::chrono::seconds{32}};

        bool                          network_version_match = false;
        fc::sha256                    node_id;

        string                        user_agent_name;
        int                           started_sessions = 0;

        std::shared_ptr<tcp::resolver>     resolver;

        bool                          use_socket_read_watermark = false;

        void connect( connection_ptr c );
        void connect( connection_ptr c, tcp::resolver::iterator endpoint_itr );
        bool start_session( connection_ptr c );
        void start_listen_loop( );
        void start_read_message( connection_ptr c);

        void close( connection_ptr c );
        size_t count_open_sockets() const;

        template<typename VerifierFunc>
        void send_all( const net_message &msg, VerifierFunc verify );

        bool is_valid( const handshake_message &msg);

        void handle_message( connection_ptr c, const handshake_message &msg);
        void handle_message( connection_ptr c, const go_away_message &msg);

        /** \brief Process time_message
         *
         * Calculate offset, delay and dispersion.  Note carefully the
         * implied processing.  The first-order difference is done
         * directly in 64-bit arithmetic, then the result is converted
         * to floating double.  All further processing is in
         * floating-double arithmetic with rounding done by the hardware.
         * This is necessary in order to avoid overflow and preserve precision.
         */
        void handle_message( connection_ptr c, const time_message &msg);

        void handle_message( connection_ptr c, const FileInfo &msg);
        void handle_message( connection_ptr c, const FileTransferPacket &msg);

        void start_broadcast(const net_message& msg);

        void start_conn_timer( );
        void start_monitors( );

        void connection_monitor( );

        /** \brief Peer heartbeat ticker.
         */
        void ticker();
        /** \brief Determine if a peer is allowed to connect.
         *
         * Checks current connection mode and key authentication.
         *
         * \return False if the peer should not connect, true otherwise.
         */

        uint16_t to_protocol_version(uint16_t v);
    };

    const fc::string logger_name("sync_net_plugin_impl");
    fc::logger logger;
    std::string peer_log_format;

#define peer_dlog( PEER, FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( logger.is_enabled( fc::log_level::debug ) ) \
      logger.log( FC_LOG_MESSAGE( debug, peer_log_format + FORMAT, __VA_ARGS__ (PEER->get_logger_variant()) ) ); \
  FC_MULTILINE_MACRO_END

#define peer_ilog( PEER, FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( logger.is_enabled( fc::log_level::info ) ) \
      logger.log( FC_LOG_MESSAGE( info, peer_log_format + FORMAT, __VA_ARGS__ (PEER->get_logger_variant()) ) ); \
  FC_MULTILINE_MACRO_END

#define peer_wlog( PEER, FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( logger.is_enabled( fc::log_level::warn ) ) \
      logger.log( FC_LOG_MESSAGE( warn, peer_log_format + FORMAT, __VA_ARGS__ (PEER->get_logger_variant()) ) ); \
  FC_MULTILINE_MACRO_END

#define peer_elog( PEER, FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( logger.is_enabled( fc::log_level::error ) ) \
      logger.log( FC_LOG_MESSAGE( error, peer_log_format + FORMAT, __VA_ARGS__ (PEER->get_logger_variant())) ); \
  FC_MULTILINE_MACRO_END


    template<class enum_type, class=typename std::enable_if<std::is_enum<enum_type>::value>::type>
    inline enum_type& operator|=(enum_type& lhs, const enum_type& rhs)
    {
        using T = std::underlying_type_t <enum_type>;
        return lhs = static_cast<enum_type>(static_cast<T>(lhs) | static_cast<T>(rhs));
    }

    static sync_net_plugin_impl *my_impl;

    /**
     * default value initializers
     */
    constexpr auto     def_send_buffer_size_mb = 4;
    constexpr auto     def_send_buffer_size = 1024*1024*def_send_buffer_size_mb;
    constexpr auto     def_max_clients = 25; // 0 for unlimited clients
    constexpr auto     def_max_nodes_per_host = 1;
    constexpr auto     def_conn_retry_wait = 30;

    constexpr auto     def_resp_expected_wait = std::chrono::seconds(5);
    constexpr auto     def_sync_fetch_span = 100;
    constexpr uint32_t  def_max_just_send = 1500; // roughly 1 "mtu"


    constexpr auto     message_header_size = 4;

    /**
     *  For a while, network version was a 16 bit value equal to the second set of 16 bits
     *  of the current build's git commit id. We are now replacing that with an integer protocol
     *  identifier. Based on historical analysis of all git commit identifiers, the larges gap
     *  between ajacent commit id values is shown below.
     *  these numbers were found with the following commands on the master branch:
     *
     *  git log | grep "^commit" | awk '{print substr($2,5,4)}' | sort -u > sorted.txt
     *  rm -f gap.txt; prev=0; for a in $(cat sorted.txt); do echo $prev $((0x$a - 0x$prev)) $a >> gap.txt; prev=$a; done; sort -k2 -n gap.txt | tail
     *
     *  DO NOT EDIT net_version_base OR net_version_range!
     */
    constexpr uint16_t net_version_base = 0x04b5;
    constexpr uint16_t net_version_range = 106;
    /**
     *  If there is a change to network protocol or behavior, increment net version to identify
     *  the need for compatibility hooks
     */
    constexpr uint16_t proto_explicit_sync = 1;

    constexpr uint16_t net_version = proto_explicit_sync;

    struct handshake_initializer {
        static void populate(handshake_message &hello);
    };

    class connection : public std::enable_shared_from_this<connection> {
    public:
        explicit connection( string endpoint );

        explicit connection( socket_ptr s );
        ~connection();
        void initialize();

        socket_ptr              socket;

        fc::message_buffer<1024*1024>    pending_message_buffer;
        fc::optional<std::size_t>        outstanding_read_bytes;
        vector<char>            blk_buffer;

        struct queued_write {
            std::shared_ptr<vector<char>> buff;
            std::function<void(boost::system::error_code, std::size_t)> callback;
        };
        deque<queued_write>     write_queue;
        deque<queued_write>     out_queue;
        fc::sha256              node_id;
        handshake_message       last_handshake_recv;
        handshake_message       last_handshake_sent;
        int16_t                 sent_handshake_count = 0;
        bool                    connecting = false;
        uint16_t                protocol_version  = 0;
        string                  peer_addr;
        std::unique_ptr<boost::asio::steady_timer> response_expected;

        go_away_reason         no_retry = no_reason;

        uint32_t               fork_head_num = 0;


        connection_status get_status()const {
            connection_status stat;
            stat.peer = peer_addr;
            stat.connecting = connecting;
            stat.last_handshake = last_handshake_recv;
            return stat;
        }

        // Members set from network data
        tstamp                         org{0};          //!< originate timestamp
        tstamp                         rec{0};          //!< receive timestamp
        tstamp                         dst{0};          //!< destination timestamp
        tstamp                         xmt{0};          //!< transmit timestamp

        // Computed data
        double                         offset{0};       //!< peer offset

        static const size_t            ts_buffer_size{32};
        char                           ts[ts_buffer_size];          //!< working buffer for making human readable timestamps

        bool connected();
        bool current();
        void reset();
        void close();
        void send_handshake();

        /** \brief Convert an std::chrono nanosecond rep to a human readable string
         */
        char* convert_tstamp(const tstamp& t);
        /**  \brief Populate and queue time_message
         */
        void send_time();
        /** \brief Populate and queue time_message immediately using incoming time_message
         */
        void send_time(const time_message& msg);
        /** \brief Read system time and convert to a 64 bit integer.
         *
         * There are only two calls on this routine in the program.  One
         * when a packet arrives from the network and the other when a
         * packet is placed on the send queue.  Calls the kernel time of
         * day routine and converts to a (at least) 64 bit integer.
         */
        tstamp get_time()
        {
            return std::chrono::system_clock::now().time_since_epoch().count();
        }

        const string peer_name();


        void enqueue( const net_message &msg, bool trigger_send = true );
        void flush_queues();

        void cancel_wait();
        void fetch_wait();
        void fetch_timeout(boost::system::error_code ec);

        void queue_write(std::shared_ptr<vector<char>> buff,
                         bool trigger_send,
                         std::function<void(boost::system::error_code, std::size_t)> callback);
        void do_queue_write();

        /** \brief Process the next message from the pending message buffer
         *
         * Process the next message from the pending_message_buffer.
         * message_length is the already determined length of the data
         * part of the message and impl in the net plugin implementation
         * that will handle the message.
         * Returns true is successful. Returns false if an error was
         * encountered unpacking or processing the message.
         */
        bool process_next_message(sync_net_plugin_impl& impl, uint32_t message_length);

        fc::optional<fc::variant_object> _logger_variant;
        const fc::variant_object& get_logger_variant()  {
            if (!_logger_variant) {
                boost::system::error_code ec;
                auto rep = socket->remote_endpoint(ec);
                string ip = ec ? "<unknown>" : rep.address().to_string();
                string port = ec ? "<unknown>" : std::to_string(rep.port());

                auto lep = socket->local_endpoint(ec);
                string lip = ec ? "<unknown>" : lep.address().to_string();
                string lport = ec ? "<unknown>" : std::to_string(lep.port());

                _logger_variant.emplace(fc::mutable_variant_object()
                    ("_name", peer_name())
                    ("_id", node_id)
                    ("_sid", ((string)node_id).substr(0, 7))
                    ("_ip", ip)
                    ("_port", port)
                    ("_lip", lip)
                    ("_lport", lport)
                );
            }
            return *_logger_variant;
        }
    };

    struct msgHandler : public fc::visitor<void> {
        sync_net_plugin_impl &impl;
        connection_ptr c;
        msgHandler( sync_net_plugin_impl &imp, connection_ptr conn) : impl(imp), c(conn) {}

        template <typename T> void operator()(const T &msg) const
        {
            impl.handle_message( c, msg);
        }
    };

    //---------------------------------------------------------------------------

    connection::connection( string endpoint )
        : socket( std::make_shared<tcp::socket>( std::ref( app().get_io_service() ))),
        node_id(),
        last_handshake_recv(),
        last_handshake_sent(),
        sent_handshake_count(0),
        connecting(false),
        protocol_version(0),
        peer_addr(endpoint),
        response_expected(),
        no_retry(no_reason),
        fork_head_num(0)
    {
        wlog( "created connection to ${n}", ("n", endpoint) );
        initialize();
    }

    connection::connection( socket_ptr s )
        : socket( s ),
        node_id(),
        last_handshake_recv(),
        last_handshake_sent(),
        sent_handshake_count(0),
        connecting(true),
        protocol_version(0),
        peer_addr(),
        response_expected(),
        no_retry(no_reason),
        fork_head_num(0)
    {
        wlog( "accepted network connection" );
        initialize();
    }

    connection::~connection() {}

    void connection::initialize() {
        auto *rnd = node_id.data();
        rnd[0] = 0;
        response_expected.reset(new boost::asio::steady_timer(app().get_io_service()));
    }

    bool connection::connected() {
        return (socket && socket->is_open() && !connecting);
    }

    bool connection::current() {
        return connected();
    }

    void connection::reset() {

     }

    void connection::flush_queues() {
        write_queue.clear();
        out_queue.clear();
    }

    void connection::close() {
        if(socket) {
            socket->close();
        }
        else {
            wlog("no socket to close!");
        }
        flush_queues();
        connecting = false;
        reset();
        sent_handshake_count = 0;
        last_handshake_recv = handshake_message();
        last_handshake_sent = handshake_message();
        fc_dlog(logger, "canceling wait on ${p}", ("p",peer_name()));
        cancel_wait();
        pending_message_buffer.reset();
    }

    void connection::send_handshake( ) {
        ilog("send_handshake to peer : ${peer}", ("peer", this->peer_name()));
        handshake_initializer::populate(last_handshake_sent);
        last_handshake_sent.generation = ++sent_handshake_count;
        fc_dlog(logger, "Sending handshake generation ${g} to ${ep}",
                ("g",last_handshake_sent.generation)("ep", peer_name()));
        enqueue(last_handshake_sent);
    }

    char* connection::convert_tstamp(const tstamp& t) {
        const long long NsecPerSec{1000000000};
        time_t seconds = t / NsecPerSec;
        strftime(ts, ts_buffer_size, "%F %T", localtime(&seconds));
        snprintf(ts+19, ts_buffer_size-19, ".%lld", t % NsecPerSec);
        return ts;
    }

    void connection::send_time() {
        time_message xpkt;
        xpkt.org = rec;
        xpkt.rec = dst;
        xpkt.xmt = get_time();
        org = xpkt.xmt;
        enqueue(xpkt);
    }

    void connection::send_time(const time_message& msg) {
        time_message xpkt;
        xpkt.org = msg.xmt;
        xpkt.rec = msg.dst;
        xpkt.xmt = get_time();
        enqueue(xpkt);
    }

    void connection::queue_write(std::shared_ptr<vector<char>> buff,
                                 bool trigger_send,
                                 std::function<void(boost::system::error_code, std::size_t)> callback) {
        write_queue.push_back({buff, callback});
        if(out_queue.empty() && trigger_send)
            do_queue_write();
    }

    void connection::do_queue_write() {
        if(write_queue.empty() || !out_queue.empty())
            return;
        connection_wptr c(shared_from_this());
        if(!socket->is_open()) {
            fc_elog(logger,"socket not open to ${p}",("p",peer_name()));
            my_impl->close(c.lock());
            return;
        }
        std::vector<boost::asio::const_buffer> bufs;
        while (write_queue.size() > 0) {
            auto& m = write_queue.front();
            bufs.push_back(boost::asio::buffer(*m.buff));
            out_queue.push_back(m);
            write_queue.pop_front();
        }
        boost::asio::async_write(*socket, bufs, [c](boost::system::error_code ec, std::size_t w) {
            try {
                auto conn = c.lock();
                if(!conn)
                    return;

                for (auto& m: conn->out_queue) {
                    m.callback(ec, w);
                }

                if(ec) {
                    string pname = conn ? conn->peer_name() : "no connection name";
                    if( ec.value() != boost::asio::error::eof) {
                        elog("Error sending to peer ${p}: ${i}", ("p",pname)("i", ec.message()));
                    }
                    else {
                        ilog("connection closure detected on write to ${p}",("p",pname));
                    }
                    my_impl->close(conn);
                    return;
                }
                while (conn->out_queue.size() > 0) {
                    conn->out_queue.pop_front();
                }
                conn->do_queue_write();
            }
            catch(const std::exception &ex) {
                auto conn = c.lock();
                string pname = conn ? conn->peer_name() : "no connection name";
                elog("Exception in do_queue_write to ${p} ${s}", ("p",pname)("s",ex.what()));
            }
            catch(const fc::exception &ex) {
                auto conn = c.lock();
                string pname = conn ? conn->peer_name() : "no connection name";
                elog("Exception in do_queue_write to ${p} ${s}", ("p",pname)("s",ex.to_string()));
            }
            catch(...) {
                auto conn = c.lock();
                string pname = conn ? conn->peer_name() : "no connection name";
                elog("Exception in do_queue_write to ${p}", ("p",pname) );
            }
        });
    }

    void connection::enqueue( const net_message &m, bool trigger_send ) {
        go_away_reason close_after_send = no_reason;
        if (m.contains<go_away_message>()) {
            close_after_send = m.get<go_away_message>().reason;
        }

        uint32_t payload_size = fc::raw::pack_size( m );
        char * header = reinterpret_cast<char*>(&payload_size);
        size_t header_size = sizeof(payload_size);

        size_t buffer_size = header_size + payload_size;

        auto send_buffer = std::make_shared<vector<char>>(buffer_size);
        fc::datastream<char*> ds( send_buffer->data(), buffer_size);
        ds.write( header, header_size );
        fc::raw::pack( ds, m );
        connection_wptr weak_this = shared_from_this();

        queue_write(send_buffer,trigger_send,
                    [weak_this, close_after_send](boost::system::error_code ec, std::size_t ) {
                        connection_ptr conn = weak_this.lock();
                        if (conn) {
                            if (close_after_send != no_reason) {
                                elog ("sent a go away message: ${r}, closing connection to ${p}",("r", reason_str(close_after_send))("p", conn->peer_name()));
                                my_impl->close(conn);
                                return;
                            }
                        } else {
                            fc_wlog(logger, "connection expired before enqueued net_message called callback!");
                        }
                    });
    }

    void connection::cancel_wait() {
        if (response_expected)
            response_expected->cancel();
    }

    void connection::fetch_wait( ) {
        response_expected->expires_from_now( my_impl->resp_expected_period);
        connection_wptr c(shared_from_this());
        response_expected->async_wait( [c]( boost::system::error_code ec){
            connection_ptr conn = c.lock();
            if (!conn) {
                // connection was destroyed before this lambda was delivered
                return;
            }

            conn->fetch_timeout(ec);
        } );
    }

    const string connection::peer_name() {
        if( !last_handshake_recv.p2p_address.empty() ) {
            return last_handshake_recv.p2p_address;
        }
        if( !peer_addr.empty() ) {
            return peer_addr;
        }
        return "connecting client";
    }

    void connection::fetch_timeout( boost::system::error_code ec ) {
        if( !ec ) {
//            if( pending_fetch.valid() && !( pending_fetch->req_trx.empty( ) || pending_fetch->req_blocks.empty( ) ) ) {
//                my_impl->dispatcher->retry_fetch (shared_from_this() );
//            }
        }
        else if( ec == boost::asio::error::operation_aborted ) {
            if( !connected( ) ) {
                fc_dlog(logger, "fetch timeout was cancelled due to dead connection");
            }
        }
        else {
            elog( "setting timer for fetch request got error ${ec}", ("ec", ec.message( ) ) );
        }
    }

    bool connection::process_next_message(sync_net_plugin_impl& impl, uint32_t message_length) {
        try {
            // If it is a SyncBlockMsg, then save the raw message for the cache
            // This must be done before we unpack the message.
            // This code is copied from fc::io::unpack(..., unsigned_int)
            auto index = pending_message_buffer.read_index();
            uint64_t which = 0; char b = 0; uint8_t by = 0;
            do {
                pending_message_buffer.peek(&b, 1, index);
                which |= uint32_t(uint8_t(b) & 0x7f) << by;
                by += 7;
            } while( uint8_t(b) & 0x80 && by < 32);

            auto ds = pending_message_buffer.create_datastream();
            net_message msg;
            fc::raw::unpack(ds, msg);
            msgHandler m(impl, shared_from_this() );
            msg.visit(m);
        } catch(  const fc::exception& e ) {
            edump((e.to_detail_string() ));
            impl.close( shared_from_this() );
            return false;
        }
        return true;
    }
    //------------------------------------------------------------------------


    //------------------------------------------------------------------------

    void sync_net_plugin_impl::connect( connection_ptr c ) {
        if( c->no_retry != go_away_reason::no_reason) {
            fc_dlog( logger, "Skipping connect due to go_away reason ${r}",("r", reason_str( c->no_retry )));
            return;
        }

        auto colon = c->peer_addr.find(':');

        if (colon == std::string::npos || colon == 0) {
            elog ("Invalid peer address. must be \"host:port\": ${p}", ("p",c->peer_addr));
            for ( auto itr : connections ) {
                if((*itr).peer_addr == c->peer_addr) {
                    (*itr).reset();
                    close(itr);
                    connections.erase(itr);
                    break;
                }
            }
            return;
        }

        auto host = c->peer_addr.substr( 0, colon );
        auto port = c->peer_addr.substr( colon + 1);
        idump((host)(port));
        tcp::resolver::query query( tcp::v4(), host.c_str(), port.c_str() );
        connection_wptr weak_conn = c;
        // Note: need to add support for IPv6 too

        resolver->async_resolve(query,
                                [weak_conn, this]( const boost::system::error_code& err,
                                tcp::resolver::iterator endpoint_itr ){
                                    auto c = weak_conn.lock();
                                    if (!c) return;
                                    if( !err ) {
                                        connect( c, endpoint_itr );
                                    } else {
                                        elog( "Unable to resolve ${peer_addr}: ${error}",
                                             (  "peer_addr", c->peer_name() )("error", err.message() ) );
                                    }
                                });
    }

    void sync_net_plugin_impl::connect( connection_ptr c, tcp::resolver::iterator endpoint_itr ) {
        if( c->no_retry != go_away_reason::no_reason) {
            string rsn = reason_str(c->no_retry);
            return;
        }
        auto current_endpoint = *endpoint_itr;
        ++endpoint_itr;
        c->connecting = true;
        connection_wptr weak_conn = c;
        c->socket->async_connect( current_endpoint, [weak_conn, endpoint_itr, this] ( const boost::system::error_code& err ) {
            auto c = weak_conn.lock();
            if (!c) return;
            if( !err && c->socket->is_open() ) {
               if (start_session( c )) {
                  c->send_handshake ();
               }
            } else {
               if( endpoint_itr != tcp::resolver::iterator() ) {
                  close(c);
                  connect( c, endpoint_itr );
               }
               else {
                  elog( "connection failed to ${peer}: ${error}",
                        ( "peer", c->peer_name())("error",err.message()));
                  c->connecting = false;
                  my_impl->close(c);
               }
            }
        } );
    }

    bool sync_net_plugin_impl::start_session( connection_ptr con ) {
        boost::asio::ip::tcp::no_delay nodelay( true );
        boost::system::error_code ec;
        con->socket->set_option( nodelay, ec );
        if (ec) {
            elog( "connection failed to ${peer}: ${error}",
                 ( "peer", con->peer_name())("error",ec.message()));
            con->connecting = false;
            close(con);
            return false;
        }
        else {
            start_read_message( con );
            ++started_sessions;
            return true;
            // for now, we can just use the application main loop.
            //     con->readloop_complete  = bf::async( [=](){ read_loop( con ); } );
            //     con->writeloop_complete = bf::async( [=](){ write_loop con ); } );
        }
    }


    void sync_net_plugin_impl::start_listen_loop( ) {
        auto socket = std::make_shared<tcp::socket>( std::ref( app().get_io_service() ) );
        acceptor->async_accept( *socket, [socket,this]( boost::system::error_code ec ) {
            if( !ec ) {
               uint32_t visitors = 0;
               uint32_t from_addr = 0;
               auto paddr = socket->remote_endpoint(ec).address();
               if (ec) {
                  fc_elog(logger,"Error getting remote endpoint: ${m}",("m", ec.message()));
               }
               else {
                  for (auto &conn : connections) {
                     if(conn->socket->is_open()) {
                        if (conn->peer_addr.empty()) {
                           visitors++;
                           boost::system::error_code ec;
                           if (paddr == conn->socket->remote_endpoint(ec).address()) {
                              from_addr++;
                           }
                        }
                     }
                  }
                  if (num_clients != visitors) {
                     ilog ("checking max client, visitors = ${v} num clients ${n}",("v",visitors)("n",num_clients));
                     num_clients = visitors;
                  }
                  if( from_addr < max_nodes_per_host && (max_client_count == 0 || num_clients < max_client_count )) {
                     ++num_clients;
                     connection_ptr c = std::make_shared<connection>( socket );
                     connections.insert( c );
                     start_session( c );

                  }
                  else {
                     if (from_addr >= max_nodes_per_host) {
                        fc_elog(logger, "Number of connections (${n}) from ${ra} exceeds limit",
                                ("n", from_addr+1)("ra",paddr.to_string()));
                     }
                     else {
                        fc_elog(logger, "Error max_client_count ${m} exceeded",
                                ( "m", max_client_count) );
                     }
                     socket->close( );
                  }
               }
            } else {
               elog( "Error accepting connection: ${m}",( "m", ec.message() ) );
               // For the listed error codes below, recall start_listen_loop()
               switch (ec.value()) {
                  case ECONNABORTED:
                  case EMFILE:
                  case ENFILE:
                  case ENOBUFS:
                  case ENOMEM:
                  case EPROTO:
                     break;
                  default:
                     return;
               }
            }
            start_listen_loop();
         });
   }

   void sync_net_plugin_impl::start_broadcast(const net_message& msg) {
       for(auto &c : connections) {
           if(c->current()) {
               ilog ("send to peer : ${peer_address}, enqueue", ("peer_address", c->peer_name()));
               c->enqueue(msg);
           }
       }
   }

   void sync_net_plugin_impl::start_read_message( connection_ptr conn ) {

      try {
         if(!conn->socket) {
            return;
         }
         connection_wptr weak_conn = conn;

         std::size_t minimum_read = conn->outstanding_read_bytes ? *conn->outstanding_read_bytes : message_header_size;

         if (use_socket_read_watermark) {
            const size_t max_socket_read_watermark = 4096;
            std::size_t socket_read_watermark = std::min<std::size_t>(minimum_read, max_socket_read_watermark);
            boost::asio::socket_base::receive_low_watermark read_watermark_opt(socket_read_watermark);
            conn->socket->set_option(read_watermark_opt);
         }

         auto completion_handler = [minimum_read](boost::system::error_code ec, std::size_t bytes_transferred) -> std::size_t {
            if (ec || bytes_transferred >= minimum_read ) {
               return 0;
            } else {
               return minimum_read - bytes_transferred;
            }
         };

         boost::asio::async_read(*conn->socket,
            conn->pending_message_buffer.get_buffer_sequence_for_boost_async_read(), completion_handler,
            [this,weak_conn]( boost::system::error_code ec, std::size_t bytes_transferred ) {
               auto conn = weak_conn.lock();
               if (!conn) {
                  return;
               }

               conn->outstanding_read_bytes.reset();

               try {
                  if( !ec ) {
                     if (bytes_transferred > conn->pending_message_buffer.bytes_to_write()) {
                        elog("async_read_some callback: bytes_transfered = ${bt}, buffer.bytes_to_write = ${btw}",
                             ("bt",bytes_transferred)("btw",conn->pending_message_buffer.bytes_to_write()));
                     }
                     ULTRAIN_ASSERT(bytes_transferred <= conn->pending_message_buffer.bytes_to_write(), ultrainio::chain::plugin_exception, "");
                     conn->pending_message_buffer.advance_write_ptr(bytes_transferred);
                     while (conn->pending_message_buffer.bytes_to_read() > 0) {
                        uint32_t bytes_in_buffer = conn->pending_message_buffer.bytes_to_read();
                        if (bytes_in_buffer < message_header_size) {
                           conn->outstanding_read_bytes.emplace(message_header_size - bytes_in_buffer);
                           break;
                        } else {
                           uint32_t message_length;
                           auto index = conn->pending_message_buffer.read_index();
                           conn->pending_message_buffer.peek(&message_length, sizeof(message_length), index);

                           if(message_length > def_send_buffer_size*2 || message_length == 0) {
                              elog("incoming message length unexpected (${i})", ("i", message_length));
                              close(conn);
                              return;
                           }

                           auto total_message_bytes = message_length + message_header_size;
                           if (bytes_in_buffer >= total_message_bytes) {
                              conn->pending_message_buffer.advance_read_ptr(message_header_size);
                              if (!conn->process_next_message(*this, message_length)) {
                                 return;
                              }
                           } else {
                              auto outstanding_message_bytes = total_message_bytes - bytes_in_buffer;
                              auto available_buffer_bytes = conn->pending_message_buffer.bytes_to_write();
                              if (outstanding_message_bytes > available_buffer_bytes) {
                                 conn->pending_message_buffer.add_space( outstanding_message_bytes - available_buffer_bytes );
                              }

                              conn->outstanding_read_bytes.emplace(outstanding_message_bytes);
                              break;
                           }
                        }
                     }
                     start_read_message(conn);
                  } else {
                     auto pname = conn->peer_name();
                     if (ec.value() != boost::asio::error::eof) {
                        elog( "Error reading message from ${p}: ${m}",("p",pname)( "m", ec.message() ) );
                     } else {
                        ilog( "Peer ${p} closed connection",("p",pname) );
                     }
                     close( conn );
                  }
               }
               catch(const std::exception &ex) {
                  string pname = conn ? conn->peer_name() : "no connection name";
                  elog("Exception in handling read data from ${p} ${s}",("p",pname)("s",ex.what()));
                  close( conn );
               }
               catch(const fc::exception &ex) {
                  string pname = conn ? conn->peer_name() : "no connection name";
                  elog("Exception in handling read data ${s}", ("p",pname)("s",ex.to_string()));
                  close( conn );
               }
               catch (...) {
                  string pname = conn ? conn->peer_name() : "no connection name";
                  elog( "Undefined exception hanlding the read data from connection ${p}",( "p",pname));
                  close( conn );
               }
            } );
      } catch (...) {
         string pname = conn ? conn->peer_name() : "no connection name";
         elog( "Undefined exception handling reading ${p}",("p",pname) );
         close( conn );
      }
   }

   size_t sync_net_plugin_impl::count_open_sockets() const
   {
      size_t count = 0;
      for( auto &c : connections) {
         if(c->socket->is_open())
            ++count;
      }
      return count;
   }


   template<typename VerifierFunc>
   void sync_net_plugin_impl::send_all( const net_message &msg, VerifierFunc verify) {
      for( auto &c : connections) {
         if( c->current() && verify( c)) {
            c->enqueue( msg );
         }
      }
   }

   bool sync_net_plugin_impl::is_valid( const handshake_message &msg) {
      // Do some basic validation of an incoming handshake_message, so things
      // that really aren't handshake messages can be quickly discarded without
      // affecting state.
      bool valid = true;
      if (msg.p2p_address.empty()) {
         wlog("Handshake message validation: p2p_address is null string");
         valid = false;
      }
      if (msg.os.empty()) {
         wlog("Handshake message validation: os field is null string");
         valid = false;
      }
      return valid;
   }

   void sync_net_plugin_impl::handle_message( connection_ptr c, const handshake_message &msg) {
      peer_ilog(c, "received handshake_message");
      ilog("got a handshake_message from ${p} ${h}", ("p",c->peer_addr)("h",msg.p2p_address));
      if (!is_valid(msg)) {
         peer_elog( c, "bad handshake message");
         c->enqueue( go_away_message( fatal_other ));
         return;
      }

      if( c->connecting ) {
         c->connecting = false;
      }
      if (msg.generation == 1) {
         if( msg.node_id == node_id) {
            elog( "Self connection detected. Closing connection");
            c->enqueue( go_away_message( self ) );
            return;
         }

         if( c->peer_addr.empty() || c->last_handshake_recv.node_id == fc::sha256()) {
            fc_dlog(logger, "checking for duplicate" );
            for(const auto &check : connections) {
               if(check == c)
                  continue;
               if(check->connected() && check->peer_name() == msg.p2p_address) {
                  // It's possible that both peers could arrive here at relatively the same time, so
                  // we need to avoid the case where they would both tell a different connection to go away.
                  // Using the sum of the initial handshake times of the two connections, we will
                  // arbitrarily (but consistently between the two peers) keep one of them.
                  if (msg.time + c->last_handshake_sent.time <= check->last_handshake_sent.time + check->last_handshake_recv.time)
                     continue;

                  fc_dlog( logger, "sending go_away duplicate to ${ep}", ("ep",msg.p2p_address) );
                  go_away_message gam(duplicate);
                  gam.node_id = node_id;
                  c->enqueue(gam);
                  c->no_retry = duplicate;
                  return;
               }
            }
         }
         else {
            fc_dlog(logger, "skipping duplicate check, addr == ${pa}, id = ${ni}",("pa",c->peer_addr)("ni",c->last_handshake_recv.node_id));
         }

         c->protocol_version = to_protocol_version(msg.network_version);
         if(c->protocol_version != net_version) {
            if (network_version_match) {
               elog("Peer network version does not match expected ${nv} but got ${mnv}",
                    ("nv", net_version)("mnv", c->protocol_version));
               c->enqueue(go_away_message(wrong_version));
               return;
            } else {
               ilog("Local network version: ${nv} Remote version: ${mnv}",
                    ("nv", net_version)("mnv", c->protocol_version));
            }
         }

         if(  c->node_id != msg.node_id) {
            c->node_id = msg.node_id;
         }

         if (c->sent_handshake_count == 0) {
            c->send_handshake();
         }
      }

      c->last_handshake_recv = msg;
      c->_logger_variant.reset();
   }

   void sync_net_plugin_impl::handle_message( connection_ptr c, const go_away_message &msg ) {
      string rsn = reason_str( msg.reason );
      peer_ilog(c, "received go_away_message");
      ilog( "received a go away message from ${p}, reason = ${r}",
            ("p", c->peer_name())("r",rsn));
      c->no_retry = msg.reason;
      if(msg.reason == duplicate ) {
         c->node_id = msg.node_id;
      }
      c->flush_queues();
      close (c);
   }

   void sync_net_plugin_impl::handle_message(connection_ptr c, const time_message &msg) {
      peer_ilog(c, "received time_message");
      /* We've already lost however many microseconds it took to dispatch
       * the message, but it can't be helped.
       */
      //ilog("received time");
      msg.dst = c->get_time();

      // If the transmit timestamp is zero, the peer is horribly broken.
      if(msg.xmt == 0)
         return;                 /* invalid timestamp */

      if(msg.xmt == c->xmt)
         return;                 /* duplicate packet */

      c->xmt = msg.xmt;
      c->rec = msg.rec;
      c->dst = msg.dst;

      if(msg.org == 0)
         {
            c->send_time(msg);
            return;  // We don't have enough data to perform the calculation yet.
         }

      c->offset = (double(c->rec - c->org) + double(msg.xmt - c->dst)) / 2;
      double NsecPerUsec{1000};

      if(logger.is_enabled(fc::log_level::all))
         logger.log(FC_LOG_MESSAGE(all, "Clock offset is ${o}ns (${us}us)", ("o", c->offset)("us", c->offset/NsecPerUsec)));
      c->org = 0;
      c->rec = 0;
   }

    void sync_net_plugin_impl::handle_message(connection_ptr c, const FileInfo &msg) {
        ilog("FileInfo msg: ${m}", ("m", msg.fileName));
    }

    void sync_net_plugin_impl::handle_message(connection_ptr c, const FileTransferPacket &msg) {
        ilog("FileTransferPacket msg: ${m}", ("m", msg.protoTag));
    }

    void sync_net_plugin_impl::start_monitors() {
        connector_check.reset(new boost::asio::steady_timer( app().get_io_service()));
        start_conn_timer();
    }

   void sync_net_plugin_impl::ticker() {
      keepalive_timer->expires_from_now (keepalive_interval);
      keepalive_timer->async_wait ([this](boost::system::error_code ec) {
            ticker ();
            if (ec) {
               wlog ("Peer keepalive ticked sooner than expected: ${m}", ("m", ec.message()));
            }
            for (auto &c : connections ) {
               if (c->socket->is_open()) {
                  c->send_time();
               }
            }
         });
   }

    void sync_net_plugin_impl::start_conn_timer( ) {
        connector_check->expires_from_now( connector_period);
        connector_check->async_wait( [this](boost::system::error_code ec) {
            if( !ec) {
                connection_monitor( );
            }
            else {
                elog( "Error from connection check monitor: ${m}",( "m", ec.message()));
                start_conn_timer( );
            }
        });
    }

   void sync_net_plugin_impl::connection_monitor( ) {
      start_conn_timer();
      auto it = connections.begin();
      while(it != connections.end()) {
         if( !(*it)->socket->is_open() && !(*it)->connecting) {
            if( (*it)->peer_addr.length() > 0) {
               connect(*it);
            }
            else {
               it = connections.erase(it);
               continue;
            }
         }
         ++it;
      }
   }

   void sync_net_plugin_impl::close( connection_ptr c ) {
      if( c->peer_addr.empty( ) && c->socket->is_open() ) {
         if (num_clients == 0) {
            fc_wlog( logger, "num_clients already at 0");
         }
         else {
            --num_clients;
         }
      }
      c->close();
   }

   void
   handshake_initializer::populate( handshake_message &hello) {
      hello.network_version = net_version_base + net_version;
      hello.node_id = my_impl->node_id;
      hello.time = std::chrono::system_clock::now().time_since_epoch().count();
      hello.token = fc::sha256::hash(hello.time);
      hello.p2p_address = my_impl->p2p_address + " - " + hello.node_id.str().substr(0,7);
#if defined( __APPLE__ )
      hello.os = "osx";
#elif defined( __linux__ )
      hello.os = "linux";
#elif defined( _MSC_VER )
      hello.os = "win32";
#else
      hello.os = "other";
#endif
      hello.agent = my_impl->user_agent_name;
   }

   sync_net_plugin::sync_net_plugin()
      :my( new sync_net_plugin_impl ) {
      my_impl = my.get();
   }

   sync_net_plugin::~sync_net_plugin() {
   }

   void sync_net_plugin::set_program_options( options_description& /*cli*/, options_description& cfg )
   {
      cfg.add_options()
         ( "p2p-listen-endpoint", bpo::value<string>()->default_value( "0.0.0.0:9876" ), "The actual host:port used to listen for incoming p2p connections.")
         ( "p2p-server-address", bpo::value<string>(), "An externally accessible host:port for identifying this node. Defaults to p2p-listen-endpoint.")
         ( "p2p-peer-address", bpo::value< vector<string> >()->composing(), "The public endpoint of a peer node to connect to. Use multiple p2p-peer-address options as needed to compose a network.")
         ( "p2p-max-nodes-per-host", bpo::value<int>()->default_value(def_max_nodes_per_host), "Maximum number of client nodes from any single IP address")
         ( "agent-name", bpo::value<string>()->default_value("\"ULTRAIN Test Agent\""), "The name supplied to identify this node amongst the peers.")
         ( "allowed-connection", bpo::value<vector<string>>()->multitoken()->default_value({"any"}, "any"), "Can be 'any' or 'producers' or 'specified' or 'none'. If 'specified', peer-key must be specified at least once. If only 'producers', peer-key is not required. 'producers' and 'specified' may be combined.")
         ( "peer-key", bpo::value<vector<string>>()->composing()->multitoken(), "Optional public key of peer allowed to connect.  May be used multiple times.")
         ( "peer-private-key", boost::program_options::value<vector<string>>()->composing()->multitoken(),
           "Tuple of [PublicKey, WIF private key] (may specify multiple times)")
         ( "max-clients", bpo::value<int>()->default_value(def_max_clients), "Maximum number of clients from which connections are accepted, use 0 for no limit")
         ( "connection-cleanup-period", bpo::value<int>()->default_value(def_conn_retry_wait), "number of seconds to wait before cleaning up dead connections")
         ( "network-version-match", bpo::value<bool>()->default_value(false),
           "True to require exact match of peer network version.")
         ( "sync-fetch-span", bpo::value<uint32_t>()->default_value(def_sync_fetch_span), "number of blocks to retrieve in a chunk from any individual peer during synchronization")
         ( "max-implicit-request", bpo::value<uint32_t>()->default_value(def_max_just_send), "maximum sizes of transaction or block messages that are sent without first sending a notice")
         ( "use-socket-read-watermark", bpo::value<bool>()->default_value(false), "Enable expirimental socket read watermark optimization")
         ( "peer-log-format", bpo::value<string>()->default_value( "[\"${_name}\" ${_ip}:${_port}]" ),
           "The string used to format peers when logging messages about them.  Variables are escaped with ${<variable name>}.\n"
           "Available Variables:\n"
           "   _name  \tself-reported name\n\n"
           "   _id    \tself-reported ID (64 hex characters)\n\n"
           "   _sid   \tfirst 8 characters of _peer.id\n\n"
           "   _ip    \tremote IP address of peer\n\n"
           "   _port  \tremote port number of peer\n\n"
           "   _lip   \tlocal IP address connected to peer\n\n"
           "   _lport \tlocal port number connected to peer\n\n")
        ;
   }

   template<typename T>
   T dejsonify(const string& s) {
      return fc::json::from_string(s).as<T>();
   }

   void sync_net_plugin::plugin_initialize( const variables_map& options ) {
      ilog("Initialize net plugin");
      try {
         peer_log_format = options.at( "peer-log-format" ).as<string>();

         my->network_version_match = options.at( "network-version-match" ).as<bool>();

         my->connector_period = std::chrono::seconds( options.at( "connection-cleanup-period" ).as<int>());
         my->resp_expected_period = def_resp_expected_wait;
         my->max_client_count = options.at( "max-clients" ).as<int>();
         my->max_nodes_per_host = options.at( "p2p-max-nodes-per-host" ).as<int>();
         my->num_clients = 0;
         my->started_sessions = 0;

         my->resolver = std::make_shared<tcp::resolver>( std::ref( app().get_io_service()));
         if( options.count( "p2p-listen-endpoint" )) {
            my->p2p_address = options.at( "p2p-listen-endpoint" ).as<string>();
            auto host = my->p2p_address.substr( 0, my->p2p_address.find( ':' ));
            auto port = my->p2p_address.substr( host.size() + 1, my->p2p_address.size());
            idump((host)( port ));
            tcp::resolver::query query( tcp::v4(), host.c_str(), port.c_str());
            // Note: need to add support for IPv6 too?

            my->listen_endpoint = *my->resolver->resolve( query );

            my->acceptor.reset( new tcp::acceptor( app().get_io_service()));
         }
         if( options.count( "p2p-server-address" )) {
            my->p2p_address = options.at( "p2p-server-address" ).as<string>();
         } else {
            if( my->listen_endpoint.address().to_v4() == address_v4::any()) {
               boost::system::error_code ec;
               auto host = host_name( ec );
               if( ec.value() != boost::system::errc::success ) {

                  FC_THROW_EXCEPTION( fc::invalid_arg_exception,
                                      "Unable to retrieve host_name. ${msg}", ("msg", ec.message()));

               }
               auto port = my->p2p_address.substr( my->p2p_address.find( ':' ), my->p2p_address.size());
               my->p2p_address = host + port;
            }
         }

         if( options.count( "p2p-peer-address" )) {
            my->supplied_peers = options.at( "p2p-peer-address" ).as<vector<string> >();
         }
         if( options.count( "agent-name" )) {
            my->user_agent_name = options.at( "agent-name" ).as<string>();
         }

         if( options.count( "allowed-connection" )) {
            const std::vector<std::string> allowed_remotes = options["allowed-connection"].as<std::vector<std::string>>();
            for( const std::string& allowed_remote : allowed_remotes ) {
               if( allowed_remote == "any" )
                  my->allowed_connections |= sync_net_plugin_impl::Any;
               else if( allowed_remote == "producers" )
                  my->allowed_connections |= sync_net_plugin_impl::Producers;
               else if( allowed_remote == "specified" )
                  my->allowed_connections |= sync_net_plugin_impl::Specified;
               else if( allowed_remote == "none" )
                  my->allowed_connections = sync_net_plugin_impl::None;
            }
         }

         fc::rand_pseudo_bytes( my->node_id.data(), my->node_id.data_size());
         ilog( "my node_id is ${id}", ("id", my->node_id));

         my->keepalive_timer.reset( new boost::asio::steady_timer( app().get_io_service()));
         my->ticker();
      } FC_LOG_AND_RETHROW()
   }

   void sync_net_plugin::plugin_startup() {
      if( my->acceptor ) {
         my->acceptor->open(my->listen_endpoint.protocol());
         my->acceptor->set_option(tcp::acceptor::reuse_address(true));
         my->acceptor->bind(my->listen_endpoint);
         my->acceptor->listen();
         ilog("starting listener, max clients is ${mc}",("mc",my->max_client_count));
         my->start_listen_loop();
      }

      my->start_monitors();

      for( auto seed_node : my->supplied_peers ) {
         connect( seed_node );
      }

      if(fc::get_logger_map().find(logger_name) != fc::get_logger_map().end())
         logger = fc::get_logger_map()[logger_name];
   }

   void sync_net_plugin::plugin_shutdown() {
      try {
         ilog( "shutdown.." );
         my->done = true;
         if( my->acceptor ) {
            ilog( "close acceptor" );
            my->acceptor->close();

            ilog( "close ${s} connections",( "s",my->connections.size()) );
            auto cons = my->connections;
            for( auto con : cons ) {
               my->close( con);
            }

            my->acceptor.reset(nullptr);
         }
         ilog( "exit shutdown" );
      }
      FC_CAPTURE_AND_RETHROW()
   }

   size_t sync_net_plugin::num_peers() const {
      return my->count_open_sockets();
   }

   /**
    *  Used to trigger a new connection from RPC API
    */
   string sync_net_plugin::connect( const string& host ) {
      if( my->find_connection( host ) )
         return "already connected";

      connection_ptr c = std::make_shared<connection>(host);
      fc_dlog(logger,"adding new connection to the list");
      my->connections.insert( c );
      fc_dlog(logger,"calling active connector");
      my->connect( c );
      return "added connection";
   }

   string sync_net_plugin::disconnect( const string& host ) {
      for( auto itr = my->connections.begin(); itr != my->connections.end(); ++itr ) {
         if( (*itr)->peer_addr == host ) {
            (*itr)->reset();
            my->close(*itr);
            my->connections.erase(itr);
            return "connection removed";
         }
      }
      return "no known connection for host";
   }

   optional<connection_status> sync_net_plugin::status( const string& host )const {
      auto con = my->find_connection( host );
      if( con )
         return con->get_status();
      return optional<connection_status>();
   }

   vector<connection_status> sync_net_plugin::connections()const {
      vector<connection_status> result;
      result.reserve( my->connections.size() );
      for( const auto& c : my->connections ) {
         result.push_back( c->get_status() );
      }
      return result;
   }

    string sync_net_plugin::transfer_file() {
//        for( auto itr = my->connections.begin(); itr != my->connections.end(); ++itr ) {
//            if( (*itr)->peer_addr == host ) {
//                (*itr)->reset();
//                my->close(*itr);
//                my->connections.erase(itr);
//                return "connection removed";
//            }
//        }
        std::vector<connection_ptr> conn_list;
        conn_list.reserve(my->connections.size());
        for (auto& c:my->connections) {
            if (c->current()) {
                conn_list.emplace_back(c);
            }
        }

        FileInfo file_info_msg;
        file_info_msg.fileName = "just a test file name";
        for (auto c:conn_list) {
            if(c->current()) {
                ilog ("send file_info_msg to peer : ${peer_address}, enqueue", ("peer_address", c->peer_name()));
                c->enqueue(net_message(file_info_msg));
            }
        }
        return "transfer file finished";
    }

   connection_ptr sync_net_plugin_impl::find_connection( string host )const {
      for( const auto& c : connections )
         if( c->peer_addr == host ) return c;
      return connection_ptr();
   }

   uint16_t sync_net_plugin_impl::to_protocol_version (uint16_t v) {
      if (v >= net_version_base) {
         v -= net_version_base;
         return (v > net_version_range) ? 0 : v;
      }
      return 0;
   }
}
