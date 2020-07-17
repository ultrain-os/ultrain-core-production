/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/chain/types.hpp>

#include <ultrainio/net_plugin/net_plugin.hpp>
#include <ultrainio/chain/controller.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <ultrainio/chain/block.hpp>
#include <ultrainio/chain/plugin_interface.hpp>
#include <ultrainio/producer_rpos_plugin/producer_rpos_plugin.hpp>
#include <ultrainio/utilities/key_conversion.hpp>
#include <ultrainio/chain/contract_types.hpp>

namespace fc {
    extern std::unordered_map<std::string,logger>& get_logger_map();
}

namespace ultrainio { namespace net_plugin_n {
    static appbase::abstract_plugin& _net_plugin = app().register_plugin<net_plugin>();

    class connection;
    class sync_block_manager;
    class dispatch_manager;

    using connection_ptr = std::shared_ptr<connection>;
    using connection_wptr = std::weak_ptr<connection>;

    using socket_ptr = std::shared_ptr<tcp::socket>;

    using net_message_ptr = shared_ptr<net_message>;

    struct passive_peer{
        shared_ptr<tcp::acceptor>  acceptor;
        shared_ptr<tcp::resolver>  resolver;
        tcp::endpoint              listen_endpoint;
        string                     p2p_address;
    };
    struct node_msg_state{
        string signature;
        connection_ptr conn;
    };
    struct by_sig;
    typedef multi_index_container<
        node_msg_state,
        indexed_by<
            ordered_unique<
            tag< by_sig >,
            member < node_msg_state,
                     string,
                     &node_msg_state::signature > > >
    > node_msg_index;
    update_in_flight  incr_in_flight(1), decr_in_flight(-1);

    class net_plugin_impl {
    public:
        passive_peer                     rpos_listener;
        passive_peer                     trx_listener;
        uint32_t                         max_static_clients = 0;
        uint32_t                         max_dynamic_clients = 0;
        uint32_t                         max_passive_out_count = 4;
        uint32_t                         min_connections = 0;
        uint32_t                         max_nodes_per_host = 4;
        uint32_t                         max_retry_count = 3;
        uint32_t                         max_grey_list_size = 10;
        uint32_t                         num_clients = 0;
        uint32_t                         num_passive_out = 0;


        vector<string>                   rpos_active_peers;
        vector<string>                   trx_active_peers;
        vector<chain::public_key_type>   allowed_peers; ///< peer keys allowed to connect
        vector<chain::public_key_type>   allowed_tcp_peers; ///< peer keys allowed to connect
        std::map<chain::public_key_type,
                 chain::private_key_type> private_keys; ///< overlapping with producer keys, also authenticating non-producing nodes
        vector<string>                   udp_seed_ip;
        bool                             use_node_table = false;
        enum possible_connections : char {
            None = 0,
            Producers = 1 << 0,
            Specified = 1 << 1,
            Any = 1 << 2
        };
        possible_connections             allowed_connections{None};

        connection_ptr find_connection(const string& host) const;
        bool is_grey_connection(const string& host) const;
        bool is_connection_to_seed(connection_ptr con) const;
        bool is_static_connection(connection_ptr con) const;
        void del_connection_with_node_id(const fc::sha256& node_id,connection_direction dir,string addr);

        std::set< connection_ptr >       connections;
        std::list< string >              peer_addr_grey_list;
        bool                             done = false;
        unique_ptr< dispatch_manager >   dispatcher;
        unique_ptr< sync_block_manager > sync_block_master;
        shared_ptr< LightClient >        light_client;
        int                        max_waitblocknum_seconds;
        int                        max_waitblock_seconds ;
        unique_ptr<boost::asio::steady_timer> connector_check;
        unique_ptr<boost::asio::steady_timer> transaction_check;
        unique_ptr<boost::asio::steady_timer> keepalive_timer;
        unique_ptr<boost::asio::steady_timer> sizeprint_timer;
        unique_ptr<boost::asio::steady_timer> speedmonitor_timer;
        unique_ptr<boost::asio::steady_timer> block_handler_check;

        boost::asio::steady_timer::duration   connector_period;
        boost::asio::steady_timer::duration   txn_exp_period;
        boost::asio::steady_timer::duration   resp_expected_period;
        boost::asio::steady_timer::duration   keepalive_interval{std::chrono::seconds{32}};
        boost::asio::steady_timer::duration   speedmonitor_period;
        boost::asio::steady_timer::duration   block_handler_period{std::chrono::seconds{1}};
        const std::chrono::system_clock::duration peer_authentication_interval{std::chrono::seconds{1}}; ///< Peer clock may be no more than 1 second skewed from our clock, including network latency.
        boost::asio::steady_timer::duration   producerslist_update_interval{std::chrono::seconds{32}};
        unique_ptr<boost::asio::steady_timer> producerslist_update_timer;
        void start_producerslist_update_timer();
        void reset_producerslist();
        bool                          network_version_match = false;
        chain_id_type                 chain_id;
        fc::sha256                    node_id;

        string                        user_agent_name;
        chain_plugin*                 chain_plug;
        int                           started_sessions = 0;

        node_transaction_index        local_txns;
        node_msg_index                local_msgs;

        bool                          use_socket_read_watermark = false;
        bool                          kcp_transport = false;
        channels::transaction_ack::channel_type::handle  incoming_transaction_ack_subscription;

        string connect( const string& endpoint, msg_priority p = msg_priority_trx,
                        connection_direction dir = direction_out, const fc::sha256& node_id = fc::sha256() );
        void connect( connection_ptr c );
        void connect( connection_ptr c, tcp::resolver::iterator endpoint_itr );
        uint32_t connect_to_endpoint( const p2p::NodeIPEndpoint& ep, connection_direction dir, const fc::sha256& node_id = fc::sha256() );
        bool start_session( connection_ptr c );
        void start_listen_loop(shared_ptr<tcp::acceptor> acceptor, msg_priority p);
        void start_read_message( connection_ptr c);
        void reset_speedlimit_monitor( );
        void start_speedlimit_monitor_timer();
        void close( connection_ptr c );
        size_t count_open_sockets() const;

        void onNodeTableDropEvent(const p2p::NodeIPEndpoint& _n);
        void onNodeTableTcpConnectEvent(const p2p::NodeIPEndpoint& _n);
        template<typename VerifierFunc> void send_all( const net_message &msg, VerifierFunc verify );

        void transaction_ack(const std::tuple<const fc::exception_ptr, const transaction_trace_ptr, const packed_transaction_ptr>&);
        bool is_valid( const handshake_message &msg);

        shared_ptr<tcp::resolver> get_resolver(msg_priority p) const;
        shared_ptr<tcp::acceptor> get_acceptor(msg_priority p) const;
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
        void handle_message( connection_ptr c, const notice_message &msg);
        void handle_message( connection_ptr c, const request_message &msg);
        void handle_message( connection_ptr c, const packed_transaction &msg);

        void handle_message( connection_ptr c, const ultrainio::EchoMsg &msg);
        void handle_message( connection_ptr c, const ultrainio::ProposeMsg& msg);
        void handle_message( connection_ptr c, const ultrainio::ReqBlockNumRangeMsg& msg);
        void handle_message( connection_ptr c, const ultrainio::RspBlockNumRangeMsg& msg);
        void handle_message( connection_ptr c, const ultrainio::ReqSyncMsg& msg);
        void handle_message( connection_ptr c, const ultrainio::SyncBlockMsg& msg);
        void handle_message( connection_ptr c, const ultrainio::SyncStopMsg& msg);

        void start_broadcast(const net_message& msg, msg_priority p);
        void start_broadcast(const SignedTransaction& trx);
        void send_block(const fc::sha256& node_id, const net_message& msg);
        bool send_req_sync(const ultrainio::ReqSyncMsg& msg);
        void send_block_num_range(const fc::sha256& node_id, const net_message& msg);
        void stop_sync_block();

        void start_conn_timer( );
        void start_txn_timer( );
        void start_block_handler_timer( );
        void start_monitors( );

        void expire_txns( );
        void connection_monitor( );
        void connection_nosymm_monitor( );

        /** \brief Peer heartbeat ticker.
         */
        void ticker();
        bool is_genesis_finished = false;
        bool is_genesis_finish();
        std::vector<string> producers_account;
        bool is_producer_account_pk(chain::account_name const& account);
        bool is_pk_signature_match(chain::public_key_type const& pk,fc::sha256 const& hash,chain::signature_type const& sig);
        bool authen_whitelist_and_producer(const fc::sha256& hash,const chain::public_key_type& pk,const chain::signature_type& sig,chain::account_name const& account);
        bool is_account_pk_match(chain::public_key_type const& pk,chain::account_name const& account);
        bool is_account_commitee_pk_match(fc::sha256 const& hash,chain::account_name const& account,std::string sig);
        bool is_account_bls_pk_match(fc::sha256 const& hash,chain::account_name const& account,std::string sig);
        /** \brief Determine if a peer is allowed to connect.
         *
         * Checks current connection mode and key authentication.
         *
         * \return False if the peer should not connect, true otherwise.
         */
        bool authenticate_peer(const handshake_message& msg);
        /** \brief Retrieve public key used to authenticate with peers.
         *
         * Finds a key to use for authentication.  If this node is a producer, use
         * the front of the producer key map.  If the node is not a producer but has
         * a configured private key, use it.  If the node is neither a producer nor has
         * a private key, returns an empty key.
         *
         * \note On a node with multiple private keys configured, the key with the first
         *       numerically smaller byte will always be used.
         */
        chain::public_key_type get_authentication_key() const;

        uint16_t to_protocol_version(uint16_t v);

        void promote_private_address(vector<string>& peers);
    };

    const fc::string logger_name("net_plugin_impl");
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

    static net_plugin_impl *my_impl;

    /**
     * default value initializers
     */
    constexpr auto     def_send_buffer_size_mb = 4;
    constexpr auto     def_send_buffer_size = 1024*1024*def_send_buffer_size_mb;
    constexpr auto     def_txn_expire_wait = std::chrono::seconds(12);
    constexpr auto     def_resp_expected_wait = std::chrono::seconds(5);
    constexpr auto     def_sync_fetch_span = 100;
    constexpr uint32_t  def_max_just_send = 1500; // roughly 1 "mtu"
    constexpr bool     large_msg_notify = false;

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
    constexpr uint16_t proto_base = 0;
    constexpr uint16_t proto_explicit_sync = 1;

    constexpr uint16_t net_version = proto_explicit_sync;

    update_known_by_peer set_is_known;
    update_request_time  set_request_time;

    struct handshake_initializer {
        static void populate(handshake_message &hello, msg_priority p, char style);
    };

    class connection : public std::enable_shared_from_this<connection> {
    public:
        explicit connection(string endpoint, msg_priority pri, connection_direction dir);

        explicit connection(socket_ptr s, msg_priority pri, connection_direction dir);
        ~connection();
        void initialize();

        static const uint32_t MAX_OUT_QUEUE = 1000;
        static const uint32_t MAX_WRITE_QUEUE = 100000;

        peer_block_state_index  blk_state;
        transaction_state_index trx_state;
        socket_ptr              socket;

        fc::message_buffer<4*1024*1024>    pending_message_buffer;
        fc::optional<std::size_t>        outstanding_read_bytes;
        vector<char>            blk_buffer;

        struct queued_write {
            std::shared_ptr<vector<char>> buff;
            std::function<void(boost::system::error_code, std::size_t)> callback;
        };
        msg_priority            priority;
        connection_direction    direct = direction_in;
        deque<queued_write>     write_queue;
        deque<queued_write>     out_queue;
        fc::sha256              node_id;
        handshake_message       last_handshake_recv;
        handshake_message       last_handshake_sent;
        int16_t                 sent_handshake_count = 0;
        bool                    ticker_rcv = false;
        uint32_t                ticker_no_rcv_count = 0;
        bool                    connecting = false;
        uint16_t                protocol_version  = 0;
        string                  peer_addr;
        chain::public_key_type  peer_pk;
        chain::account_name peer_account;
        unique_ptr<boost::asio::steady_timer> response_expected;
        optional<request_message> pending_fetch;
        go_away_reason          no_retry = no_reason;
        block_id_type           fork_head;
        uint32_t                fork_head_num = 0;
        optional<request_message> last_req;
        RspBlockNumRangeMsg     block_num_range;
        uint32_t pack_count_rcv = 0;
        uint32_t pack_count_drop = 0;
        uint32_t retry_connect_count = 0;
        uint32_t wait_handshake_count = 0;
        std::string get_conn_directstring(connection_direction dir)const{
            switch(dir)
            {
                case direction_in:
                    return "in";
                    break;
                case direction_out:
                    return "out";
                    break;
                case direction_passive_out:
                    return "passive";
                    break;
                default:
                    break;
            }
            return "in";
        }
        connection_status get_status()const {
            connection_status stat;
            boost::system::error_code ec;
            auto peer_str = socket->remote_endpoint(ec).address().to_string();
            auto port =  to_string(socket->remote_endpoint(ec).port());
            stat.peer = peer_str + ":" + port + ","
                + get_conn_directstring(direct)+","
                + (priority==msg_priority_rpos ? "rpos":"trx")+","
                + last_handshake_recv.account.to_string()+",tcp";
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

        void txn_send_pending(const vector<transaction_id_type> &ids);
        void txn_send(const vector<transaction_id_type> &txn_lis);

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
        bool process_next_message(net_plugin_impl& impl, uint32_t message_length);
        bool check_pkt_limit_exceed();
        bool add_peer_block(const peer_block_state &pbs);

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
        net_plugin_impl &impl;
        connection_ptr c;
        msgHandler( net_plugin_impl &imp, connection_ptr conn) : impl(imp), c(conn) {}

        template <typename T> void operator()(const T &msg) const
        {
            impl.handle_message( c, msg);
        }
    };

    class sync_block_manager {
    public:
        uint32_t                         seq_num;
        uint32_t                         last_received_block;
        uint32_t                         last_checked_block;
        uint32_t                         sync_src_count;
        std::set<connection_ptr>         conns_without_block;
        std::vector<connection_ptr>      rsp_conns;
        connection_ptr                   sync_conn;
        ultrainio::ReqSyncMsg            sync_block_msg;
        uint32_t                         end_block_num;
        uint32_t                         first_safe_block_num;
        uint32_t                         last_safe_block_num;
        std::list<SyncBlockMsg>          block_msg_queue;
        bool                             selecting_src = false;
        boost::asio::steady_timer::duration   src_block_period;
        unique_ptr<boost::asio::steady_timer> src_block_check;
        boost::asio::steady_timer::duration   conn_timeout;
        unique_ptr<boost::asio::steady_timer> conn_check;
        std::default_random_engine            rand_engine;

        sync_block_manager(int src_check_timeout, int conn_check_timeout) {
            seq_num = 0;
            reset();
            src_block_period = {std::chrono::seconds{src_check_timeout}};
            src_block_check.reset(new boost::asio::steady_timer(app().get_io_service()));
            conn_timeout = {std::chrono::seconds{conn_check_timeout}};
            conn_check.reset(new boost::asio::steady_timer(app().get_io_service()));
            std::random_device rd;
            rand_engine.seed(rd());
        }

        void reset() {
            last_received_block = 0;
            last_checked_block = 0;
            sync_src_count = 0;
            conns_without_block.clear();
            rsp_conns.clear();
            sync_conn = nullptr;
            memset(&sync_block_msg, 0, sizeof(sync_block_msg));
            end_block_num = 0;
            first_safe_block_num = 0;
            last_safe_block_num = 0;
            selecting_src = false;
            block_msg_queue.clear();

            if (src_block_check) {
                src_block_check->cancel();
            }

            if (conn_check) {
                conn_check->cancel();
            }
        }

        void set_src_block_period(const boost::asio::steady_timer::duration& d) {
            src_block_period = d;
        }

        void start_conn_check_timer() {
            conn_check->expires_from_now(conn_timeout);
            conn_check->async_wait([this](boost::system::error_code ec) {
                if (ec.value() == boost::asio::error::operation_aborted) {
                    ilog("receive block conn check canceled, will not wait for next block. last received:${rcv} last checked:${chk}",
                         ("rcv", last_received_block)("chk", last_checked_block));
                    app().get_plugin<producer_rpos_plugin>().sync_cancel();
                    reset();
                }else if (last_received_block <= last_checked_block || ec.value() != 0) {
                    ilog("no block received in last period or error occur. last received:${rcv} last checked:${chk} ec:${ec}",
                         ("rcv", last_received_block)("chk", last_checked_block)("ec", ec.value()));
                    app().get_plugin<producer_rpos_plugin>().sync_fail(sync_block_msg);
                    reset();
                }else {
                    last_checked_block = last_received_block;
                    start_conn_check_timer();
                }
            });
        }

        connection_ptr select_longest_sync_src() {
            std::vector<connection_ptr> longest_conns;
            uint32_t block_num = 0;
            for (auto& con : rsp_conns) {
                if (con->block_num_range.firstNum == 0 || con->block_num_range.firstNum > sync_block_msg.startBlockNum) { // can't provide all the blocks
                    ilog("${p} can't provide all the blocks", ("p", con->peer_name()));
                    conns_without_block.insert(con);
                    if (conns_without_block.size() == sync_src_count && seq_num >= 15) {
                        ULTRAIN_ASSERT(false, chain::unlinkable_block_exception, "No block in neighbors. Help!!!");
                    }
                } else if (con->block_num_range.lastNum > block_num) {
                    longest_conns.clear();
                    longest_conns.emplace_back(con);
                    block_num = con->block_num_range.lastNum;
                } else if (con->block_num_range.lastNum == block_num) {
                    longest_conns.emplace_back(con);
                }
            }

            if (longest_conns.empty()) {
                return nullptr;
            }
            if (longest_conns.front()->block_num_range.lastNum < sync_block_msg.startBlockNum) {
                // We have a longest chain. Reset end block num, so can transfer to producing block in consensus module.
                sync_block_msg.endBlockNum = sync_block_msg.startBlockNum;
                return nullptr;
            }

            uint32_t r = rand_engine()%longest_conns.size();
            ilog("select random ${r}th longest connection to sync block. peer:${p}", ("r", r)("p", longest_conns[r]->peer_name()));
            return longest_conns[r];
        }

        void sync_block(connection_ptr con) {
            ilog("start sync block");
            selecting_src = false;
            last_received_block = 0;
            last_checked_block = 0;
            sync_src_count = 0;
            conns_without_block.clear();
            first_safe_block_num = 0;
            last_safe_block_num = 0;

            if (sync_block_msg.endBlockNum > con->block_num_range.lastNum) {
                if (sync_block_msg.endBlockNum == std::numeric_limits<uint32_t>::max()) {
                    sync_block_msg.endBlockNum = con->block_num_range.lastNum + 1;
                }else {
                    sync_block_msg.endBlockNum = con->block_num_range.lastNum;
                }
            }
            end_block_num = sync_block_msg.endBlockNum;
            con->enqueue(sync_block_msg);
            sync_conn = con;
            rsp_conns.clear();
            start_conn_check_timer();
        }

        void handle_block() {
            if (block_msg_queue.empty()) {
                return;
            }

            fc::time_point dead_line = fc::time_point::now() + fc::microseconds(1000000);
            ilog("start handle blocks.");
            while(!block_msg_queue.empty()) {
                auto& b = block_msg_queue.front().block;
                bool is_safe = b.block_num() <= last_safe_block_num;

                // We can handle the block in 2 cases: 1. the block is confirmed 2. the block is last one and the chain is bax
                if (!(is_safe || (sync_block_msg.startBlockNum == sync_conn->block_num_range.lastNum && sync_block_msg.startBlockNum == b.block_num()))) {
                    break;
                }

                bool is_last_block = (b.block_num() == end_block_num);
                if (!app().get_plugin<producer_rpos_plugin>().handle_message(block_msg_queue.front(), is_last_block, is_safe)) {
                    return;
                }

                if (is_last_block) {
                    ilog("is last block reset");
                    reset();
                } else {
                    block_msg_queue.pop_front();
                }

                if (fc::time_point::now() > dead_line) {
                    ilog("It gets dead line.");
                    break;
                }
            }
        }

        void confirm_safe_blocks(const std::list<BlockHeader>& safe_blocks) {
            ilog("Blocks from ${s} to ${e} have been checked. first safe: ${fs} last safe: ${ls}",
                 ("s", safe_blocks.front().block_num())
                 ("e", safe_blocks.back().block_num())
                 ("fs", first_safe_block_num)
                 ("ls", last_safe_block_num));

            if (last_safe_block_num == 0) {
                first_safe_block_num = safe_blocks.front().block_num();
                last_safe_block_num = safe_blocks.back().block_num();
            } else if (last_safe_block_num + 1 == safe_blocks.front().block_num()) {
                last_safe_block_num = safe_blocks.back().block_num();
            } else {
                elog("Error!!! Confirmed block nums are not continuous.");
            }
        }
    };

    class CheckBlockCallback : public LightClientCallback {
    public:
        CheckBlockCallback(sync_block_manager& sbm) : m_sbm{sbm} {

        }

        ~CheckBlockCallback() {
        }

        virtual void onConfirmed(const std::list<BlockHeader>& checkedBlocks) {
            if (checkedBlocks.empty()) {
                elog("Error!!! No block checked.");
                return;
            }
            m_sbm.confirm_safe_blocks(checkedBlocks);
        }
    private:
        sync_block_manager& m_sbm;
    };

    class dispatch_manager {
    public:
        uint32_t just_send_it_max = 0;

        vector<transaction_id_type> req_trx;

        std::multimap<transaction_id_type, connection_ptr> received_transactions;

        void bcast_transaction (const packed_transaction& msg);
        void rejected_transaction (const transaction_id_type& msg);

        void recv_transaction(connection_ptr conn, const transaction_id_type& id);
        void recv_notice (connection_ptr conn, const notice_message& msg, bool generated);

        void retry_fetch (connection_ptr conn);
    };

    //---------------------------------------------------------------------------
connection::connection(string endpoint, msg_priority pri, connection_direction dir)
        : blk_state(),
        trx_state(),
        socket( std::make_shared<tcp::socket>( std::ref( app().get_io_service() ))),
        priority(pri),
        direct(dir),
        node_id(),
        last_handshake_recv(),
        last_handshake_sent(),
        sent_handshake_count(0),
        connecting(false),
        protocol_version(0),
        peer_addr(endpoint),
        response_expected(),
        pending_fetch(),
        no_retry(no_reason),
        fork_head(),
        fork_head_num(0),
        last_req(),
        block_num_range()
    {
        wlog( "created connection to ${n}", ("n", endpoint) );
        initialize();
    }

    connection::connection(socket_ptr s, msg_priority pri, connection_direction dir)
        : blk_state(),
        trx_state(),
        socket(s),
        priority(pri),
        direct(dir),
        node_id(),
        last_handshake_recv(),
        last_handshake_sent(),
        sent_handshake_count(0),
        connecting(true),
        protocol_version(0),
        peer_addr(),
        response_expected(),
        pending_fetch(),
        no_retry(no_reason),
        fork_head(),
        fork_head_num(0),
        last_req(),
        block_num_range()
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
        blk_state.clear();
        trx_state.clear();
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
        if( last_req ) {
            my_impl->dispatcher->retry_fetch (shared_from_this());
        }
        reset();
        ticker_rcv = false;
        ticker_no_rcv_count = 0;
        sent_handshake_count = 0;
        last_handshake_recv = handshake_message();
        last_handshake_sent = handshake_message();
        fc_dlog(logger, "canceling wait on ${p}", ("p",peer_name()));
        cancel_wait();
        pending_message_buffer.reset();
    }

    void connection::txn_send_pending(const vector<transaction_id_type> &ids) {
        for(auto tx = my_impl->local_txns.begin(); tx != my_impl->local_txns.end(); ++tx ){
            if(tx->serialized_txn.size() && tx->block_num == 0) {
                bool found = false;
                for(auto known : ids) {
                    if( known == tx->id) {
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    my_impl->local_txns.modify(tx,incr_in_flight);
                    queue_write(std::make_shared<vector<char>>(tx->serialized_txn),
                                true,
                                [tx_id=tx->id](boost::system::error_code ec, std::size_t ) {
                                    auto& local_txns = my_impl->local_txns;
                                    auto tx = local_txns.get<by_id>().find(tx_id);
                                    if (tx != local_txns.end()) {
                                        local_txns.modify(tx, decr_in_flight);
                                    } else {
                                        fc_wlog(logger, "Local pending TX erased before queued_write called callback");
                                    }
                                });
                }
            }
        }
    }

    void connection::txn_send(const vector<transaction_id_type> &ids) {
        for(auto t : ids) {
            auto tx = my_impl->local_txns.get<by_id>().find(t);
            if( tx != my_impl->local_txns.end() && tx->serialized_txn.size()) {
                my_impl->local_txns.modify( tx,incr_in_flight);
                queue_write(std::make_shared<vector<char>>(tx->serialized_txn),
                            true,
                            [t](boost::system::error_code ec, std::size_t ) {
                                auto& local_txns = my_impl->local_txns;
                                auto tx = local_txns.get<by_id>().find(t);
                                if (tx != local_txns.end()) {
                                    local_txns.modify(tx, decr_in_flight);
                                } else {
                                    fc_wlog(logger, "Local TX erased before queued_write called callback");
                                }
                            });
            }
        }
    }

    void connection::send_handshake( ) {
        ilog("send_handshake to peer : ${peer}", ("peer", this->peer_name()));
        char style = 'n';
        if (peer_addr.length() > 0) {
            style = 'd';
            if (priority == msg_priority_trx) {
                if (std::find(my_impl->trx_active_peers.begin(), my_impl->trx_active_peers.end(), peer_addr) != my_impl->trx_active_peers.end()) {
                    style = 's';
                }
            } else {
                if (std::find(my_impl->rpos_active_peers.begin(), my_impl->rpos_active_peers.end(), peer_addr) != my_impl->rpos_active_peers.end()) {
                    style = 's';
                }
            }
        }
        handshake_initializer::populate(last_handshake_sent, this->priority, style);
        last_handshake_sent.generation = ++sent_handshake_count;
        fc_dlog(logger, "Sending handshake generation ${g} to ${ep}",
                ("g",last_handshake_sent.generation)("ep", peer_name()));
        enqueue(last_handshake_sent);
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
        if (write_queue.size() >= MAX_WRITE_QUEUE) {
            elog("write queue is full. peer: ${p}", ("p", peer_name()));
            return;
        } else {
            write_queue.push_back({buff, callback});
        }

        if(out_queue.empty() && trigger_send)
            do_queue_write();
    }

    void connection::do_queue_write() {
        if(write_queue.empty() || !out_queue.empty())
            return;
        connection_wptr c(shared_from_this());
        if(!socket->is_open()) {
            elog("socket not open to ${p}",("p",peer_name()));
            my_impl->close(c.lock());
            return;
        }
        std::vector<boost::asio::const_buffer> bufs;
        while (write_queue.size() > 0 && out_queue.size() < MAX_OUT_QUEUE) {
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

                while (conn->out_queue.size()>0) {
                    auto m = conn->out_queue.front();
                    conn->out_queue.pop_front();
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
            if( pending_fetch.valid() && !( pending_fetch->req_trx.empty( ) || pending_fetch->req_blocks.empty( ) ) ) {
                my_impl->dispatcher->retry_fetch (shared_from_this() );
            }
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

    bool connection::check_pkt_limit_exceed() {
        static int count_rpos_threhold =  2000;
        static int count_trx_threhold =  20000;

        pack_count_rcv ++;
        if(((priority == msg_priority_rpos) && (pack_count_rcv > count_rpos_threhold))
			||((priority == msg_priority_trx)&&(pack_count_rcv >count_trx_threhold)))
        {
            pack_count_drop++;
            return true;
        }
        return false;
    }
    bool connection::process_next_message(net_plugin_impl& impl, uint32_t message_length) {
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

            if (which == uint64_t(net_message::tag<SyncBlockMsg>::value)) {
                blk_buffer.resize(message_length);
                auto index = pending_message_buffer.read_index();
                pending_message_buffer.peek(blk_buffer.data(), message_length, index);
            }
            auto ds = pending_message_buffer.create_datastream();
            net_message msg;
            fc::raw::unpack(ds, msg);
            ticker_rcv = true;
            bool isexceed = check_pkt_limit_exceed();
            if(isexceed)
            {
                return true;
            }
            msgHandler m(impl, shared_from_this() );
            msg.visit(m);
        } catch(  const fc::exception& e ) {
            edump((e.to_detail_string() ));
            impl.close( shared_from_this() );
            return false;
        }
        return true;
    }

    bool connection::add_peer_block(const peer_block_state &entry) {
        auto bptr = blk_state.get<by_id>().find(entry.id);
        bool added = (bptr == blk_state.end());
        if (added){
            blk_state.insert(entry);
        }
        else {
            blk_state.modify(bptr,set_is_known);
            if (entry.block_num == 0) {
                blk_state.modify(bptr,update_block_num(entry.block_num));
            }
            else {
                blk_state.modify(bptr,set_request_time);
            }
        }
        return added;
    }

    void dispatch_manager::bcast_transaction (const packed_transaction& trx) {
        std::set<connection_ptr> skips;
        transaction_id_type id = trx.id();

        auto range = received_transactions.equal_range(id);
        for (auto org = range.first; org != range.second; ++org) {
           skips.insert(org->second);
        }
        received_transactions.erase(range.first, range.second);

        for (auto ref = req_trx.begin(); ref != req_trx.end(); ++ref) {
            if (*ref == id) {
                req_trx.erase(ref);
                break;
            }
        }

        if( my_impl->local_txns.get<by_id>().find( id ) != my_impl->local_txns.end( ) ) { //found
            fc_dlog(logger, "found trxid in local_trxs" );
            return;
        }
        uint32_t packsiz = 0;
        uint32_t bufsiz = 0;

        time_point_sec trx_expiration = trx.expiration();

        net_message msg(trx);
        packsiz = fc::raw::pack_size(msg);
        bufsiz = packsiz + sizeof(packsiz);
        vector<char> buff(bufsiz);
        fc::datastream<char*> ds( buff.data(), bufsiz);
        ds.write( reinterpret_cast<char*>(&packsiz), sizeof(packsiz) );
        fc::raw::pack( ds, msg );
        node_transaction_state nts = {id,
                                    trx_expiration,
                                    trx,
                                    std::move(buff),
                                    0, 0, 0};
        my_impl->local_txns.insert(std::move(nts));

        if( !large_msg_notify || bufsiz <= just_send_it_max) {
            my_impl->send_all( trx, [id, &skips, trx_expiration](connection_ptr c) -> bool {
                if( skips.find(c) != skips.end() ) {
                    return false;
                }
                const auto& bs = c->trx_state.find(id);
                bool unknown = bs == c->trx_state.end();
                if( unknown) {
                    c->trx_state.insert(transaction_state({id,true,true,0,trx_expiration,time_point() }));
                    fc_dlog(logger, "sending whole trx to ${n}", ("n",c->peer_name() ) );
                } else {
                    update_txn_expiry ute(trx_expiration);
                    c->trx_state.modify(bs, ute);
                }
                return unknown;
            });
        }
        else {
            notice_message pending_notify;
            pending_notify.known_trx.mode = normal;
            pending_notify.known_trx.ids.push_back( id );
            pending_notify.known_blocks.mode = none;
            my_impl->send_all(pending_notify, [id, &skips, trx_expiration](connection_ptr c) -> bool {
                if (skips.find(c) != skips.end() ) {
                    return false;
                }
                const auto& bs = c->trx_state.find(id);
                bool unknown = bs == c->trx_state.end();
                if( unknown) {
                    fc_dlog(logger, "sending notice to ${n}", ("n",c->peer_name() ) );
                    c->trx_state.insert(transaction_state({id,false,true,0,trx_expiration,time_point() }));
                } else {
                    update_txn_expiry ute(trx_expiration);
                    c->trx_state.modify(bs, ute);
                }
                return unknown;
            });
        }
    }

    void dispatch_manager::recv_transaction (connection_ptr c, const transaction_id_type& id) {
        received_transactions.insert(std::make_pair(id, c));
        if (c &&
            c->last_req &&
            c->last_req->req_trx.mode != none &&
            !c->last_req->req_trx.ids.empty() &&
            c->last_req->req_trx.ids.back() == id) {
            c->last_req.reset();
        }

        fc_dlog(logger, "canceling wait on ${p}", ("p",c->peer_name()));
        c->cancel_wait();
    }

    void dispatch_manager::rejected_transaction (const transaction_id_type& id) {
        fc_dlog(logger,"not sending rejected transaction ${tid}",("tid",id));
        auto range = received_transactions.equal_range(id);
        received_transactions.erase(range.first, range.second);
    }

    void dispatch_manager::recv_notice (connection_ptr c, const notice_message& msg, bool generated) {
        request_message req;
        req.req_trx.mode = none;
        req.req_blocks.mode = none;
        bool send_req = false;
        controller &cc = my_impl->chain_plug->chain();
        if (msg.known_trx.mode == normal) {
            req.req_trx.mode = normal;
            req.req_trx.pending = 0;
            for( const auto& t : msg.known_trx.ids ) {
                const auto &tx = my_impl->local_txns.get<by_id>( ).find( t );

                if( tx == my_impl->local_txns.end( ) ) {
                    fc_dlog(logger,"did not find ${id}",("id",t));

                    //At this point the details of the txn are not known, just its id. This
                    //effectively gives 120 seconds to learn of the details of the txn which
                    //will update the expiry in bcast_transaction
                    c->trx_state.insert( (transaction_state){t,true,true,0,time_point_sec(time_point::now()) + 120,
                        time_point()} );

                    req.req_trx.ids.push_back( t );
                    req_trx.push_back( t );
                }
                else {
                    fc_dlog(logger,"big msg manager found txn id in table, ${id}",("id", t));
                }
            }
            send_req = !req.req_trx.ids.empty();
            fc_dlog(logger,"big msg manager send_req ids list has ${ids} entries", ("ids", req.req_trx.ids.size()));
        }
        else if (msg.known_trx.mode != none) {
            elog ("passed a notice_message with something other than a normal on none known_trx");
            return;
        }
        if (msg.known_blocks.mode == normal) {
            req.req_blocks.mode = normal;
            for( const auto& blkid : msg.known_blocks.ids) {
                signed_block_ptr b;
                peer_block_state entry = {blkid,0,true,true,fc::time_point()};
                try {
                    b = cc.fetch_block_by_id(blkid);
                    if(b)
                        entry.block_num = b->block_num();
                } catch (const assert_exception &ex) {
                    ilog( "caught assert on fetch_block_by_id, ${ex}",("ex",ex.what()));
                    // keep going, client can ask another peer
                } catch (...) {
                    elog( "failed to retrieve block for id");
                }
                if (!b) {
                    send_req = true;
                    req.req_blocks.ids.push_back( blkid );
                    entry.requested_time = fc::time_point::now();
                }
                c->add_peer_block(entry);
            }
        }
        else if (msg.known_blocks.mode != none) {
            elog ("passed a notice_message with something other than a normal on none known_blocks");
            return;
        }
        fc_dlog( logger, "send req = ${sr}", ("sr",send_req));
        if( send_req) {
            c->enqueue(req);
            c->fetch_wait();
            c->last_req = std::move(req);
        }
    }

    void dispatch_manager::retry_fetch( connection_ptr c ) {
        if (!c->last_req) {
            return;
        }
        fc_wlog( logger, "failed to fetch from ${p}",("p",c->peer_name()));
        transaction_id_type tid;
        block_id_type bid;
        bool is_txn = false;
        if( c->last_req->req_trx.mode == normal && !c->last_req->req_trx.ids.empty() ) {
            is_txn = true;
            tid = c->last_req->req_trx.ids.back();
        }
        else if( c->last_req->req_blocks.mode == normal && !c->last_req->req_blocks.ids.empty() ) {
            bid = c->last_req->req_blocks.ids.back();
        }
        else {
            fc_wlog( logger,"no retry, block mpde = ${b} trx mode = ${t}",
                    ("b",modes_str(c->last_req->req_blocks.mode))("t",modes_str(c->last_req->req_trx.mode)));
            return;
        }
        for (auto conn : my_impl->connections) {
            if (conn == c || conn->last_req) {
                continue;
            }
            bool sendit = false;
            if (is_txn) {
                auto trx = conn->trx_state.get<by_id>().find(tid);
                sendit = trx != conn->trx_state.end() && trx->is_known_by_peer;
            }
            else {
                auto blk = conn->blk_state.get<by_id>().find(bid);
                sendit = blk != conn->blk_state.end() && blk->is_known;
            }
            if (sendit) {
                conn->enqueue(*c->last_req);
                conn->fetch_wait();
                conn->last_req = c->last_req;
                return;
            }
        }

        // at this point no other peer has it, re-request or do nothing?
        if( c->connected() ) {
            c->enqueue(*c->last_req);
            c->fetch_wait();
        }
    }

    //------------------------------------------------------------------------

    string net_plugin_impl::connect(const string& host, msg_priority p, connection_direction dir, const fc::sha256& node_id) {
        if (find_connection(host))
            return "already connected";

        connection_ptr c = std::make_shared<connection>(host, p, dir);
        if (node_id != fc::sha256()) {
            c->node_id = node_id;
        }
        connections.insert( c );
        connect( c );
        return "added connection";
    }

    void net_plugin_impl::connect( connection_ptr c ) {
        if (c->no_retry != go_away_reason::no_reason && c->no_retry != go_away_reason::authentication) {
            ilog("Skipping connect due to go_away reason ${r}",("r", reason_str( c->no_retry )));
            if(c->no_retry == go_away_reason::duplicate)
            {
                connections.erase(c);
            }
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
        tcp::resolver::query query( tcp::v4(), host.c_str(), port.c_str() );
        connection_wptr weak_conn = c;
        // Note: need to add support for IPv6 too

        get_resolver(c->priority)->async_resolve(query,
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

    void net_plugin_impl::connect( connection_ptr c, tcp::resolver::iterator endpoint_itr ) {
        if (c->no_retry != go_away_reason::no_reason && c->no_retry != go_away_reason::authentication) {
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

    uint32_t net_plugin_impl::connect_to_endpoint( const p2p::NodeIPEndpoint& ep, connection_direction dir, const fc::sha256& node_id ) {
        uint32_t i = 0;
        string host = ep.address() + ":" + std::to_string(ep.listenPort(msg_priority_trx));
        if (!find_connection(host) && !is_grey_connection(host)) {
            ilog("connect to node: ${a}", ("a", host));
            connect(host, msg_priority_trx, dir, node_id);
            i++;
        }

        host = ep.address() + ":" + std::to_string(ep.listenPort(msg_priority_rpos));
        if (!find_connection(host) && !is_grey_connection(host)) {
            ilog("connect to node: ${a}", ("a", host));
            connect(host, msg_priority_rpos, dir, node_id);
            i++;
        }

        return i;
    }

    bool net_plugin_impl::start_session( connection_ptr con ) {
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

    void net_plugin_impl::start_listen_loop(shared_ptr<tcp::acceptor> acceptor, msg_priority p) {
        auto socket = std::make_shared<tcp::socket>( std::ref( app().get_io_service() ) );
        acceptor->async_accept( *socket, [acceptor,socket,this,p]( boost::system::error_code ec ) {
            if( !ec ) {
               uint32_t visitors = 0;
               uint32_t from_addr = 0;
               auto paddr = socket->remote_endpoint(ec).address();
               if (ec) {
                  elog("Error getting remote endpoint: ${m}",("m", ec.message()));
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
                  if( (from_addr < max_nodes_per_host && num_clients < (max_static_clients +  max_dynamic_clients))
                      || std::find(udp_seed_ip.begin(), udp_seed_ip.end(), paddr.to_string()) != udp_seed_ip.end() ) { // always accept seed connection
                     ++num_clients;
                     connection_ptr c = std::make_shared<connection>(socket, p, direction_in);
                     connections.insert( c );
                     start_session( c );

                  }
                  else {
                     if (from_addr >= max_nodes_per_host) {
                        elog("Number of connections (${n}) from ${ra} exceeds limit",
                                ("n", from_addr+1)("ra",paddr.to_string()));
                     }
                     else {
                        elog("Error max_client_count ${m} exceeded",
                                ( "m", max_static_clients +  max_dynamic_clients) );
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
            start_listen_loop(acceptor, p);
         });
   }

    void net_plugin_impl::start_broadcast(const net_message& msg, msg_priority p) {
        std::string peers_str;
        for(auto &c : connections) {
            if (c->current() && p == c->priority) {
                peers_str += c->peer_name() + ";";
                c->enqueue(msg);
            }
        }
        if (!peers_str.empty()) {
            ilog ("enqueue send to peers : ${peer_address}", ("peer_address", peers_str));
        }

    }

    void net_plugin_impl::start_broadcast(const SignedTransaction& trx) {
        dispatcher->bcast_transaction(packed_transaction(trx, packed_transaction::zlib));
    }

    void net_plugin_impl::send_block(const fc::sha256& node_id, const net_message& msg) {
        for(auto &c : connections) {
            if (c->priority == msg_priority_trx && c->current()) {
                if (c->node_id == node_id) {
                    ilog ("send block to peer : ${peer_name}, enqueue", ("peer_name", c->peer_name()));
                    c->enqueue(msg);
                    break;
                }
            }
        }
    }

    void net_plugin_impl::send_block_num_range(const fc::sha256& node_id, const net_message& msg) {
        for (auto &c : connections) {
            if (c->priority == msg_priority_trx && c->current()) {
                if (c->node_id == node_id) {
                    ilog("send block num range to peer: ${p}", ("p", c->peer_name()));
                    c->enqueue(msg);
                    break;
                }
            }
        }
    }

    void net_plugin_impl::stop_sync_block() {
        if (sync_block_master->sync_conn) {
            SyncStopMsg stop_msg;
            stop_msg.seqNum = sync_block_master->seq_num;
            sync_block_master->sync_conn->enqueue(stop_msg);
        }
        sync_block_master->reset();
    }

    bool net_plugin_impl::send_req_sync(const ultrainio::ReqSyncMsg& msg) {
        sync_block_master->rsp_conns.clear();
        std::vector<connection_ptr> conn_list;
        conn_list.reserve(connections.size());
        for (auto& c:connections) {
            if (c->priority == msg_priority_trx && c->current()) {
                c->block_num_range.firstNum = 0;
                c->block_num_range.lastNum = 0;
                c->block_num_range.blockHash = "";
                c->block_num_range.prevBlockHash = "";
                conn_list.emplace_back(c);
            }
        }

        ReqBlockNumRangeMsg block_num_range_msg;
        block_num_range_msg.seqNum = ++sync_block_master->seq_num;
        sync_block_master->sync_src_count = conn_list.size();
        sync_block_master->conns_without_block.clear();
        for (auto c:conn_list) {
            if(c->current()) {
                ilog ("send req block num range to peer : ${peer_address}, enqueue", ("peer_address", c->peer_name()));
                c->enqueue(net_message(block_num_range_msg));
            }
        }

        sync_block_master->end_block_num = 0;
        sync_block_master->sync_block_msg = msg;
        sync_block_master->sync_block_msg.seqNum = block_num_range_msg.seqNum;
        sync_block_master->selecting_src = true;

        sync_block_master->src_block_check->expires_from_now(sync_block_master->src_block_period);
        sync_block_master->src_block_check->async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("select sync source canceled");
                if (!sync_block_master->sync_conn) {
                    ilog("producer_rpos_plugin sync cancel");
                    app().get_plugin<producer_rpos_plugin>().sync_cancel();
                }
            }else if (sync_block_master->selecting_src && sync_block_master->end_block_num == 0) {
                connection_ptr wc = sync_block_master->select_longest_sync_src();
                if (wc) {
                    sync_block_master->sync_block(wc);
                } else {
                    ilog("select sync source timeout");
                    app().get_plugin<producer_rpos_plugin>().sync_fail(sync_block_master->sync_block_msg);
                }
            }
        });

        return true;
    }

   void net_plugin_impl::start_read_message( connection_ptr conn ) {

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
                     ULTRAIN_ASSERT(bytes_transferred <= conn->pending_message_buffer.bytes_to_write(), plugin_exception, "");
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

    size_t net_plugin_impl::count_open_sockets() const
    {
        size_t count = 0;
        for( auto &c : connections) {
            if(c->socket->is_open())
                ++count;
        }
        return count;
    }

    template<typename VerifierFunc>
    void net_plugin_impl::send_all( const net_message &msg, VerifierFunc verify) {
        for( auto &c : connections) {
            if( c->priority == msg_priority_trx && c->current() && verify( c)) {
                c->enqueue( msg );
            }
        }
    }

    bool net_plugin_impl::is_valid( const handshake_message &msg) {
        // Do some basic validation of an incoming handshake_message, so things
        // that really aren't handshake messages can be quickly discarded without
        // affecting state.
        bool valid = true;
        if (msg.last_irreversible_block_num > msg.head_num) {
            wlog("Handshake message validation: last irreversible block (${i}) is greater than head block (${h})",
                 ("i", msg.last_irreversible_block_num)("h", msg.head_num));
            valid = false;
        }
        if (msg.p2p_address.empty()) {
            wlog("Handshake message validation: p2p_address is null string");
            valid = false;
        }
        if (msg.os.empty()) {
            wlog("Handshake message validation: os field is null string");
            valid = false;
        }
        if ((msg.sig != chain::signature_type() || msg.token != sha256()) && (msg.token != fc::sha256::hash(msg.time))) {
            wlog("Handshake message validation: token field invalid");
            valid = false;
        }
        return valid;
    }

    shared_ptr<tcp::resolver> net_plugin_impl::get_resolver(msg_priority p) const {
        if (p == msg_priority_rpos) {
            return rpos_listener.resolver;
        } else {
            return trx_listener.resolver;
        }
    }

    shared_ptr<tcp::acceptor> net_plugin_impl::get_acceptor(msg_priority p) const {
        if (p == msg_priority_rpos) {
            return rpos_listener.acceptor;
        } else {
            return trx_listener.acceptor;
        }
    }

   void net_plugin_impl::handle_message( connection_ptr c, const handshake_message &msg) {
      ilog("got a handshake_message from ${p}, ${h}, ${nod}", ("p",c->peer_addr)("h",msg.p2p_address)("nod", msg.node_id));
      if (!is_valid(msg)) {
         elog("bad handshake message");
         c->enqueue( go_away_message( fatal_other ));
         return;
      }
      controller& cc = chain_plug->chain();
      uint32_t lib_num = cc.last_irreversible_block_num( );
      uint32_t peer_lib = msg.last_irreversible_block_num;
      if( c->connecting ) {
         c->connecting = false;
      }

      if( msg.node_id == node_id) {
         elog( "Self connection detected. Closing connection");
         c->enqueue( go_away_message( self ) );
         return;
      }

      if (c->direct == direction_in) {
         string style;
         for (auto& ext : msg.ext) {
            if (ext.key == handshake_ext::connect_style) {
               style = ext.value;
            }
         }

         if (!style.empty()) {
            if (style[0] != 's' && style[0] != 'd') {
               elog("bad handshake message");
               c->enqueue( go_away_message( fatal_other ));
               return;
            }

            uint32_t client_count = 0;
            for (auto& check : connections) {
               if (check == c) {
                  continue;
               }
               for (auto& ext : check->last_handshake_recv.ext) {
                  if (ext.key == handshake_ext::connect_style && ext.value.length() > 0 && ext.value[0] == style[0]) {
                     client_count++;
	             break;
                  }
               }
            }

            if ((style[0] == 's' && client_count >= max_static_clients) || (style[0] == 'd' && client_count >= max_dynamic_clients)) {
               elog("${style} clients = ${cc}, exceeds limit.", ("style", style)("cc", client_count));
               c->enqueue(go_away_message(too_many_connections));
               return;
            }
         }
      }

      if( c->peer_addr.empty() || c->last_handshake_recv.node_id == fc::sha256()) {
         dlog("checking for duplicate" );
         for(const auto &check : connections) {
            ilog("peer: ${peer}, node id: ${nd}, connected: ${cond}", ("peer", check->peer_name())("nd", check->last_handshake_recv.node_id)("cond", check->connected()));
            if(check == c)
               continue;
            if(check->connected() && check->peer_name() == msg.p2p_address) {
               // It's possible that both peers could arrive here at relatively the same time, so
               // we need to avoid the case where they would both tell a different connection to go away.
               // Using the sum of the initial handshake times of the two connections, we will
               // arbitrarily (but consistently between the two peers) keep one of them.
               if (msg.time + c->last_handshake_sent.time <= check->last_handshake_sent.time + check->last_handshake_recv.time)
                  continue;

               dlog("sending go_away duplicate to ${ep}", ("ep",msg.p2p_address) );
               go_away_message gam(duplicate);
               gam.node_id = node_id;
               c->enqueue(gam);
               c->no_retry = duplicate;
               return;
            }
         }
      }
      else {
         dlog("skipping duplicate check, addr == ${pa}, id = ${ni}",("pa",c->peer_addr)("ni",c->last_handshake_recv.node_id));
      }

      if( msg.chain_id != chain_id) {
         elog( "Peer on a different chain. Closing connection");
         c->enqueue( go_away_message(go_away_reason::wrong_chain) );
         return;
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

      if(!authenticate_peer(msg)) {
         elog("Peer not authenticated.  Closing connection.");
         c->enqueue(go_away_message(authentication));
         return;
      }

      if((c->priority != msg_priority_rpos)&&(c->priority != msg_priority_trx)) {
         elog("wrong priority  Closing connection.");
         c->enqueue(go_away_message(authentication));
         return;
      }
      //check for duplicate key access
      for(auto &it : connections) {
         if(it->connected() && (it->peer_account == msg.account) && (it->priority == c->priority)) {
            boost::system::error_code ec;
            if(it->socket->remote_endpoint(ec).address() != c->socket->remote_endpoint(ec).address()) {

               ilog("duplicate pk");
               go_away_message gam(no_reason);
               gam.node_id = node_id;
               c->enqueue(gam);
               return;
            }
         }
      }
      bool on_fork = false;
      fc_dlog(logger, "lib_num = ${ln} peer_lib = ${pl}",("ln",lib_num)("pl",peer_lib));
      uint32_t first_in_local = cc.first_block_num();
      if( peer_lib <= lib_num && peer_lib > 0 && peer_lib >= first_in_local) {
         try {
            block_id_type peer_lib_id =  cc.get_block_id_for_num( peer_lib);
            on_fork =( msg.last_irreversible_block_id != peer_lib_id);
         }
         catch( const unknown_block_exception &ex) {
            wlog( "peer last irreversible block ${pl} is unknown", ("pl", peer_lib));
            on_fork = true;
         }
         catch( ...) {
            wlog( "caught an exception getting block id for ${pl}",("pl",peer_lib));
            on_fork = true;
         }
         if( on_fork) {
            elog( "Peer chain is forked");
            c->enqueue( go_away_message( forked ));
            return;
         }
      }

      if (c->sent_handshake_count == 0) {
         c->send_handshake();
      }

      c->peer_pk = msg.key;
      c->peer_account = msg.account;
      c->last_handshake_recv = msg;
      c->_logger_variant.reset();
   }

   void net_plugin_impl::handle_message( connection_ptr c, const go_away_message &msg ) {
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
      if (msg.reason == duplicate) {
          boost::system::error_code ec;
          del_connection_with_node_id(msg.node_id,c->direct,c->socket->remote_endpoint(ec).address().to_string()); /// warning !!!
      }
   }

   void net_plugin_impl::handle_message(connection_ptr c, const time_message &msg) {
      peer_ilog(c, "received time_message");
      /* We've already lost however many microseconds it took to dispatch
       * the message, but it can't be helped.
       */
      ilog("received time ${peer}",("peer",c->peer_name()));
      if(c->current())
      {
         c->ticker_rcv = true;
      }
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

   void net_plugin_impl::handle_message( connection_ptr c, const notice_message &msg) {
      // peer tells us about one or more blocks or txns. When done syncing, forward on
      // notices of previously unknown blocks or txns,
      //
      return;
      peer_ilog(c, "received notice_message");
      c->connecting = false;
      request_message req;
      bool send_req = false;
      if (msg.known_trx.mode != none) {
         fc_dlog(logger,"this is a ${m} notice with ${n} blocks", ("m",modes_str(msg.known_trx.mode))("n",msg.known_trx.pending));
      }
      switch (msg.known_trx.mode) {
      case none:
         break;
      case last_irr_catch_up: {
         c->last_handshake_recv.head_num = msg.known_trx.pending;
         req.req_trx.mode = none;
         break;
      }
      case catch_up : {
         if( msg.known_trx.pending > 0) {
            // plan to get all except what we already know about.
            req.req_trx.mode = catch_up;
            send_req = true;
            size_t known_sum = local_txns.size();
            if( known_sum ) {
               for( const auto& t : local_txns.get<by_id>( ) ) {
                  req.req_trx.ids.push_back( t.id );
               }
            }
         }
         break;
      }
      case normal: {
         dispatcher->recv_notice (c, msg, false);
      }
      }

      if (msg.known_blocks.mode != none) {
         fc_dlog(logger,"this is a ${m} notice with ${n} blocks", ("m",modes_str(msg.known_blocks.mode))("n",msg.known_blocks.pending));
      }
      switch (msg.known_blocks.mode) {
      case none : {
         if (msg.known_trx.mode != normal) {
            return;
         }
         break;
      }
      case last_irr_catch_up:
      case catch_up: {
         break;
      }
      case normal : {
         dispatcher->recv_notice (c, msg, false);
         break;
      }
      default: {
         peer_elog(c, "bad notice_message : invalid known_blocks.mode ${m}",("m",static_cast<uint32_t>(msg.known_blocks.mode)));
      }
      }
      fc_dlog(logger, "send req = ${sr}", ("sr",send_req));
      if( send_req) {
         c->enqueue(req);
      }
   }

    void net_plugin_impl::handle_message( connection_ptr c, const request_message &msg) {
        return;
        switch (msg.req_blocks.mode) {
        case catch_up :
            peer_elog(c,  "received request_message:catch_up");
            break;
        case normal :
            peer_elog(c, "received request_message:normal");
            break;
        default:;
        }

        switch (msg.req_trx.mode) {
        case catch_up :
            c->txn_send_pending(msg.req_trx.ids);
            break;
        case normal :
            c->txn_send(msg.req_trx.ids);
            break;
        case none :
            if(msg.req_blocks.mode == none)
                peer_elog(c, "received request_message:none");
            break;
        default:;
        }
   }

   void net_plugin_impl::handle_message( connection_ptr c, const EchoMsg &msg) {
//       ilog("echo from ${p} block_id: ${id} num: ${num} phase: ${phase} baxcount: ${baxcount} account: ${account} sig: ${sig}",
//            ("p", c->peer_name())("id", short_hash(msg.blockId))("num", BlockHeader::num_from_id(msg.blockId))
//            ("phase", (uint32_t)msg.phase)("baxcount",msg.baxCount)("account", std::string(msg.account))("sig", short_sig(msg.signature)));
       string  sig = msg.signature;
       if( my_impl->local_msgs.get<by_sig>().find( sig ) == my_impl->local_msgs.end( ) ) { //no found
           node_msg_state nms= {sig,c};
           my_impl->local_msgs.insert(std::move(nms));
       }
       app().get_plugin<producer_rpos_plugin>().handle_message(msg);
   }

   void net_plugin_impl::handle_message( connection_ptr c, const ProposeMsg& msg) {
       ilog("propose from ${p} block id: ${id} block num: ${num}",
            ("p", c->peer_name())("id", short_hash(msg.block.id()))("num", msg.block.block_num()));
       string  sig = msg.signature;
       if( my_impl->local_msgs.get<by_sig>().find( sig ) == my_impl->local_msgs.end( ) ) { //no found
           node_msg_state nms= {sig,c};
           my_impl->local_msgs.insert(std::move(nms));
       }
       app().get_plugin<producer_rpos_plugin>().handle_message(msg);
   }

    void net_plugin_impl::handle_message( connection_ptr c, const ultrainio::ReqBlockNumRangeMsg& msg) {
        ilog("receive req block num range msg!!! from peer ${p} seq: ${s}", ("p", c->peer_name())("s", msg.seqNum));
        app().get_plugin<producer_rpos_plugin>().handle_message(c->node_id, msg);
    }

    void net_plugin_impl::handle_message( connection_ptr c, const ultrainio::RspBlockNumRangeMsg& msg) {
        ilog("receive rsp block num range msg!!! from peer ${p} seq: ${s} block num: ${f} - ${l}", ("p", c->peer_name())("s", msg.seqNum)("f", msg.firstNum)("l", msg.lastNum));
        ilog("sync block master selecting src:${s} seq:${seq}", ("s", sync_block_master->selecting_src)("seq", sync_block_master->seq_num));
        if (!sync_block_master->selecting_src || msg.seqNum != sync_block_master->seq_num) {
            return;
        }

        for (auto& con : sync_block_master->rsp_conns) {
            if (con == c) {
                elog("duplicate rsp block num range msg from peer ${p}", ("p", c->peer_name()));
                return;
            }
        }
        c->block_num_range = msg;
        sync_block_master->rsp_conns.emplace_back(c);

        if (sync_block_master->rsp_conns.size() < connections.size()) {
            return;
        }
        connection_ptr sc = sync_block_master->select_longest_sync_src();
        if (sc) {
            sync_block_master->src_block_check->cancel();
            sync_block_master->sync_block(sc);
        }
    }

    void net_plugin_impl::handle_message( connection_ptr c, const ultrainio::ReqSyncMsg& msg) {
        ilog("receive req sync msg!!! message from ${p} addr:${addr} blockNum = ${blockNum}",
             ("p", c->peer_name())("addr",c->peer_addr)("blockNum", msg.endBlockNum));
        app().get_plugin<producer_rpos_plugin>().handle_message(c->node_id, msg);
    }

    void net_plugin_impl::handle_message( connection_ptr c, const ultrainio::SyncBlockMsg& msg) {
        ilog("receive block msg!!! message from ${p} blockNum = ${blockNum} end block num:${eb}",
             ("p", c->peer_name())("blockNum", msg.block.block_num())("eb", sync_block_master->end_block_num));

        if (c != sync_block_master->sync_conn || msg.seqNum != sync_block_master->seq_num) {
            wlog("receive old block msg!!! Discard. seq num in msg: ${snm}, seq num in master: ${sns}", ("snm", msg.seqNum)("sns", sync_block_master->seq_num));
            SyncStopMsg stop_msg;
            stop_msg.seqNum = msg.seqNum;
            c->enqueue(stop_msg);
            return;
        }

        if (sync_block_master->end_block_num > 0) {
            if (sync_block_master->last_received_block == 0 || sync_block_master->last_received_block + 1 == msg.block.block_num()) {
                sync_block_master->last_received_block++;
            } else {
                elog("receive block with wrong number: ${num}, but last received block num: ${last}", ("num", msg.block.block_num())("last", sync_block_master->last_received_block));
            }

            sync_block_master->last_received_block = msg.block.block_num();
            if (sync_block_master->last_received_block == sync_block_master->sync_block_msg.startBlockNum) {
                controller &cc = chain_plug->chain();
                std::shared_ptr<StakeVoteBase> stake = MsgMgr::getInstance()->getStakeVote(cc.head_block_num() + 1);
                StartPoint sp(stake->getCommitteeSet(), cc.head_block_id());
                if (EpochEndPoint::isEpochEndPoint(cc.head_block_header())) {
                    EpochEndPoint eep(cc.head_block_header());
                    sp.nextCommitteeMroot = eep.nextCommitteeMroot();
                }
                sp.genesisPk = Genesis::s_genesisPk;
                light_client->setStartPoint(sp);
            }
            sync_block_master->block_msg_queue.emplace_back(msg);
            BlsVoterSet blsVoterSet(msg.proof);
            light_client->accept(msg.block, msg.block.signature, blsVoterSet);
        }
    }

    void net_plugin_impl::handle_message( connection_ptr c, const ultrainio::SyncStopMsg &msg) {
        ilog("receive sync stop msg!!! message from ${p} addr:${addr} seqNum = ${sn}",
             ("p", c->peer_name())("addr", c->peer_addr)("sn", msg.seqNum));
        app().get_plugin<producer_rpos_plugin>().handle_message(c->node_id, msg);
    }

   void net_plugin_impl::handle_message( connection_ptr c, const packed_transaction &msg) {
      fc_dlog(logger, "got a packed transaction, cancel wait");
      peer_ilog(c, "received packed_transaction");
      transaction_id_type tid = msg.id();
      c->cancel_wait();
      if(local_txns.get<by_id>().find(tid) != local_txns.end()) {
         fc_dlog(logger, "got a duplicate transaction - dropping");
         return;
      }
      dispatcher->recv_transaction(c, tid);
      //uint64_t code = 0;
      chain_plug->accept_transaction(msg, true, [=](const static_variant<fc::exception_ptr, transaction_trace_ptr>& result) {
          if (result.contains<fc::exception_ptr>()) {
              auto e_ptr = result.get<fc::exception_ptr>();
              // Non-producing node does not pre-run network borne trx but still broadcasts it.
              if (e_ptr->code() == discard_network_trx_for_non_producing_node::code_value) {
                  dispatcher->bcast_transaction(msg);
                  //elog("discard_network_trx_for_non_producing_node::code_value)");
                  return;
              } else if (e_ptr->code() != tx_duplicate::code_value &&
                         e_ptr->code() != expired_tx_exception::code_value &&
                         e_ptr->code() != node_is_syncing::code_value) {
                  elog("accept txn threw  ${m}",("m",result.get<fc::exception_ptr>()->to_detail_string()));
              }
          } else {
              auto trace = result.get<transaction_trace_ptr>();
              if (!trace->except && trace->ability != action::PureView) {
                  fc_dlog(logger, "chain accepted transaction");
                  dispatcher->bcast_transaction(msg);
                  return;
              }
          }

          dispatcher->rejected_transaction(tid);
      });
   }

    void net_plugin_impl::reset_speedlimit_monitor( )
    {
        for(auto &c : connections) {
            if(c->current()){
                ilog("${p} peer ${peer} count_rcv ${counnt_rcv} count_drop ${count_drop} ",
                        ("p",c->priority==msg_priority_rpos ? "rpos":"trx")
                        ("peer",c->peer_name())
                        ("counnt_rcv",c->pack_count_rcv)
                        ("count_drop",c->pack_count_drop));
                c->pack_count_rcv=0;
                c->pack_count_drop = 0;
            }
        }
    }
    void net_plugin_impl::start_speedlimit_monitor_timer( )
    {
        speedmonitor_timer->expires_from_now( speedmonitor_period);
        speedmonitor_timer->async_wait( [this](boost::system::error_code ec) {
	       reset_speedlimit_monitor();
	       start_speedlimit_monitor_timer();
        });
    }
    void net_plugin_impl::start_producerslist_update_timer( )
    {
        producerslist_update_timer->expires_from_now(producerslist_update_interval);
        producerslist_update_timer->async_wait( [this](boost::system::error_code ec) {
            reset_producerslist();
	    start_producerslist_update_timer();
        });
    }
    void net_plugin_impl::start_conn_timer( ) {
        connector_check->expires_from_now( connector_period);
        connector_check->async_wait( [this](boost::system::error_code ec) {
            if( !ec) {
               connection_monitor( );
               connection_nosymm_monitor();
            }
            else {
                elog( "Error from connection check monitor: ${m}",( "m", ec.message()));
                start_conn_timer( );
            }
        });
    }

    void net_plugin_impl::onNodeTableDropEvent(p2p::NodeIPEndpoint const& _n) {
        //TODO:P2P udp linkage p2p
        ilog("onNodeTableDropEven addr ${addr}",("addr",_n.address()));
        boost::system::error_code ec;
        auto it = connections.begin();
        while(it != connections.end()) {
            if((*it)->socket->remote_endpoint(ec).address().to_string() == _n.m_address) {
                if(!is_static_connection(*it)){
                    close(*it);
                    it = connections.erase(it);
                }else{
                    ++it;
                }
            } else {
                ++it;
            }
        }
    }

    void net_plugin_impl::onNodeTableTcpConnectEvent(p2p::NodeIPEndpoint const& _n) {
        ilog("TcpConnectEvent from: ${addr}", ("addr", _n.address()));
        if (num_passive_out < max_passive_out_count) {
            num_passive_out += connect_to_endpoint(_n, direction_passive_out);
        } else {
            wlog("num_passive_out: ${num} exceeds limit: ${max}, discard to connect to ${addr}",
                 ("num", num_passive_out)("max", max_passive_out_count)("addr", _n.address()));
        }
    }
    void net_plugin_impl::start_block_handler_timer( ) {
        block_handler_check->expires_from_now(block_handler_period);
        block_handler_check->async_wait( [this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("block handler timer is canceled.");
            } else {
                sync_block_master->handle_block();
                start_block_handler_timer(); // We must restart timer after handle_block()
            }
        });
    }

   void net_plugin_impl::start_txn_timer() {
      transaction_check->expires_from_now( txn_exp_period);
      transaction_check->async_wait( [this](boost::system::error_code ec) {
            if( !ec) {
               expire_txns( );
            }
            else {
               elog( "Error from transaction check monitor: ${m}",( "m", ec.message()));
               start_txn_timer( );
            }
         });
   }

   void net_plugin_impl::ticker() {
      keepalive_timer->expires_from_now (keepalive_interval);
      keepalive_timer->async_wait ([this](boost::system::error_code ec) {
            ticker ();
            if (ec) {
               wlog ("Peer keepalive ticked sooner than expected: ${m}", ("m", ec.message()));
            }
            for (auto &c : connections ) {
               if (c->current())
               {
                  if(!c->ticker_rcv)
                  {
                      ilog("ticker no rcv times ${no_rcv_count} ${peer}",("no_rcv_count",c->ticker_no_rcv_count)("peer",c->peer_name()));
                      c->ticker_no_rcv_count++;
                      if(c->ticker_no_rcv_count >= 5)
                      {
                         ilog("ticker checked");
                         c->ticker_rcv = false;
                         c->ticker_no_rcv_count = 0;
                         close(c);
                      }
                   }
                   else
                   {
                       c->ticker_rcv = false;
                       c->ticker_no_rcv_count = 0;
                   }
               }
               if (c->socket->is_open()) {
                  c->send_time();
               }
            }
         });
   }

   void net_plugin_impl::start_monitors() {
      connector_check.reset(new boost::asio::steady_timer( app().get_io_service()));
      transaction_check.reset(new boost::asio::steady_timer( app().get_io_service()));
      sizeprint_timer.reset(new boost::asio::steady_timer( app().get_io_service()));
      speedmonitor_timer.reset(new boost::asio::steady_timer( app().get_io_service()));
      block_handler_check.reset(new boost::asio::steady_timer( app().get_io_service()));
      producerslist_update_timer.reset(new boost::asio::steady_timer( app().get_io_service()));
      start_conn_timer();
      start_txn_timer();
      int round_interval = app().get_plugin<producer_rpos_plugin>().get_round_interval();
      speedmonitor_period = {std::chrono::seconds{round_interval}};
      start_speedlimit_monitor_timer();
      start_block_handler_timer();
   }

   void net_plugin_impl::expire_txns() {
      start_txn_timer( );

      // Lets keep watch on the size of received_transactions.
      ilog("received_trx size ${s}", ("s", dispatcher->received_transactions.size()));

      auto &old = local_txns.get<by_expiry>();
      auto ex_up = old.upper_bound( time_point::now());
      auto ex_lo = old.lower_bound( fc::time_point_sec( 0));
      old.erase( ex_lo, ex_up);

      auto &stale = local_txns.get<by_block_num>();
      controller &cc = chain_plug->chain();
      uint32_t bn = cc.last_irreversible_block_num();
      stale.erase( stale.lower_bound(1), stale.upper_bound(bn) );
      for ( auto &c : connections ) {
         auto &stale_txn = c->trx_state.get<by_block_num>();
         stale_txn.erase( stale_txn.lower_bound(1), stale_txn.upper_bound(bn) );
         auto &stale_txn_e = c->trx_state.get<by_expiry>();
         stale_txn_e.erase(stale_txn_e.lower_bound(time_point_sec()), stale_txn_e.upper_bound(time_point::now()));
         auto &stale_blk = c->blk_state.get<by_block_num>();
         stale_blk.erase( stale_blk.lower_bound(1), stale_blk.upper_bound(bn) );
      }

#if defined( __linux__ )
      static uint32_t malloc_trim_count = 0;
      malloc_trim_count++;
      if (malloc_trim_count % 25 == 0) { // transaction expire timer = 12s, malloc timer = 3min
        malloc_trim(0);
      }
#endif
   }

   void net_plugin_impl::connection_nosymm_monitor( ) {
       int count_pri_trx = 0,count_pri_rpos = 0;
       static std::default_random_engine rg(time(0));
       if(!use_node_table){
           return ;
       }
       for (auto& c:connections) {
           if(c->priority == msg_priority_rpos){
               count_pri_rpos ++;
           }
           else if(c->priority == msg_priority_trx){
               count_pri_trx ++;
           }
       }
       if(count_pri_rpos < 2 || count_pri_trx < 2){
           if(peer_addr_grey_list.size()>0){
               ilog("pop all grey list");
               peer_addr_grey_list.clear();
           }
           if(connections.size() >= min_connections + num_passive_out){
               std::list<p2p::NodeEntry> nodes = p2p::NodeTable::getInstance()->getNodes();
               uint32_t i = 0;
               for (std::list<p2p::NodeEntry>::iterator it = nodes.begin(); it != nodes.end() && i < 4; ++it) {
                   if (rg() % 2 == 0) {
                       continue;
                   }
                   i += connect_to_endpoint(it->endPoint(), direction_out, it->id());
               }
           }
       }
   }
   void net_plugin_impl::connection_monitor( ) {
      static std::default_random_engine rg(time(0));

      start_conn_timer();
      auto it = connections.begin();
      while(it != connections.end()) {
         if (!(*it)->socket->is_open() && !(*it)->connecting) {
            if (use_node_table && (*it)->retry_connect_count > max_retry_count) { // if use_node_table == false, we should always reconnect peers
               if (peer_addr_grey_list.size() >= max_grey_list_size) {
                  peer_addr_grey_list.pop_front();
               }

               if (!is_grey_connection((*it)->peer_addr)) {
                   if(!is_connection_to_seed(*it))
                   {
                       peer_addr_grey_list.emplace_back((*it)->peer_addr);
                       ilog("put ${a} to grey list", ("a", (*it)->peer_addr));
                   }
                   if ((*it)->node_id != fc::sha256()) {
                     p2p::NodeTable::getInstance()->send_request_connect((*it)->node_id);
                     ilog("send_request_connect to ${p}.", ("p", (*it)->peer_addr));
                  }
               }
               ilog("erase ${a} ", ("a", (*it)->peer_addr));
               it = connections.erase(it);
               continue;
            } else if ((*it)->peer_addr.length() <= 0 || is_grey_connection((*it)->peer_addr)) {
               it = connections.erase(it);
               continue;
            } else {
               if (!is_static_connection(*it)) { // skip the addr of seeds and static connections, so always reconnect
                  (*it)->retry_connect_count++;
               }
               connect(*it);
            }
         } else if ((*it)->last_handshake_recv.generation == 0) { // no handshake received
            if ((*it)->wait_handshake_count > 0 && (*it)->socket->is_open()) {
               boost::system::error_code ec;
               ilog("no handshake received from: peer: ${peer} ${addr}",
                    ("peer", (*it)->peer_name())
                    ("addr", (*it)->socket->remote_endpoint(ec).address().to_string()));
               if ((*it)->node_id != fc::sha256()) {
                  p2p::NodeTable::getInstance()->send_request_connect((*it)->node_id);
                  ilog("send_request_connect to ${p}.", ("p", (*it)->peer_addr));
               }
               close(*it);
               if (!is_static_connection(*it)) {
                  elog("the non static connection will be erased");
                  it = connections.erase(it);
               }
               continue;
            }
            (*it)->wait_handshake_count++;
         }
         ++it;
      }

      ilog("connections size: ${s} min_connections: ${mc} num_clients: ${nc} grey list: ${gls} num_passive_out: ${npo}",
           ("s", connections.size())("mc", min_connections)("nc", num_clients)("gls", peer_addr_grey_list.size())("npo", num_passive_out));
      if (!kcp_transport && use_node_table && connections.size() < min_connections + num_passive_out) { // if use_node_table == false, we can't get any valid node ip from node table
         uint32_t count = min_connections - (connections.size() - num_passive_out);
         std::list<p2p::NodeEntry> nodes = p2p::NodeTable::getInstance()->getNodes();
         uint32_t i = 0;
         for (std::list<p2p::NodeEntry>::iterator it = nodes.begin(); it != nodes.end() && i < count; ++it) {
            if (rg() % 2 == 0) {
               continue;
            }

            i += connect_to_endpoint(it->endPoint(), direction_out, it->id());
         }
      }
   }

   void net_plugin_impl::close( connection_ptr c ) {
      if (c->socket->is_open()) {
         if (c->direct == direction_passive_out) {
            if (num_passive_out == 0) {
               wlog("num_passive_out already at 0");
            } else {
               --num_passive_out;
            }
         } else if ( c->peer_addr.empty() ) {
            if (num_clients == 0) {
               wlog("num_clients already at 0");
            }
            else {
               --num_clients;
            }
         }
      }

      c->close();
   }

   void net_plugin_impl::transaction_ack(const std::tuple<const fc::exception_ptr, const transaction_trace_ptr, const packed_transaction_ptr>& results) {
      transaction_id_type id = std::get<2>(results)->id();
      if (std::get<0>(results)) {
         fc_ilog(logger,"signaled NACK, trx-id = ${id} : ${why}",("id", id)("why", std::get<0>(results)->to_detail_string()));
         dispatcher->rejected_transaction(id);
      } else if (std::get<1>(results) && std::get<1>(results)->ability == action::PureView) {
          fc_ilog(logger, "PureView action, reject to broadcast.");
         // Do not broadcast pureview action.
         dispatcher->rejected_transaction(id);
      } else {
         fc_ilog(logger,"signaled ACK, trx-id = ${id}",("id", id));
         const auto& ptx = std::get<2>(results);
         dispatcher->bcast_transaction(*ptx);
      }
   }
   bool net_plugin_impl::is_producer_account_pk(chain::account_name const& account)
   {

       auto found_producer_account = std::find(producers_account.begin(), producers_account.end(), account.to_string());
       if(found_producer_account == producers_account.end())
       {
	       return false;
       }
       return true;
   }

   void net_plugin_impl::reset_producerslist()
   {
        const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
        struct chain_apis::read_only::get_producers_params params;
        params.json=true;
        params.lower_bound="";
        params.chain_name = chain::self_chain_name;
        params.all_chain = false;
        producers_account.clear();
	try {
            auto result = ro_api.get_producers(params);
            if(!result.rows.empty()) {
                for( const auto& r : result.rows ) {
                    producers_account.push_back(r.prod_detail["owner"].as_string());
                }

            }
        }
       catch (fc::exception& e) {
            ilog("there may be no producer registered: ${e}", ("e", e.to_string()));
        }
   }
   bool net_plugin_impl::is_genesis_finish()
   {
       if(!is_genesis_finished)
       {
           const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
           is_genesis_finished  = ro_api.is_genesis_finished();
           if(!is_genesis_finished)
           {
               return false;
           }
           else
           {
               reset_producerslist();
               start_producerslist_update_timer();
           }
       }
       return true;

   }
   bool net_plugin_impl::authen_whitelist_and_producer(fc::sha256 const& hash,chain::public_key_type const& pk,chain::signature_type const& sig,chain::account_name const& account)
   {
        bool is_genesis_fin = is_genesis_finish();
        if(!is_genesis_fin)
        {
            return true;
        }
       /*pk match the signature*/
        bool is_pk_signature_matched = is_pk_signature_match(pk,hash,sig);
        if(!is_pk_signature_matched)
        {
           return false;
        }
        /*pk match the account*/
        bool is_account_pk_matched = is_account_pk_match(pk,account);
        if(!is_account_pk_matched)
        {
            return false;
        }
        /*pk or username in whitelist or producers*/
        auto allowed_it = std::find(allowed_peers.begin(), allowed_peers.end(), pk);
        bool is_producer_pk = is_producer_account_pk(account);
       if( allowed_it == allowed_peers.end() && (!is_producer_pk))
       {
           elog("an unauthorized key");
           return false;
       }
       return true;
   }
bool net_plugin_impl::is_pk_signature_match(chain::public_key_type const& pk,fc::sha256 const& hash,chain::signature_type const& sig)
{
    /*pk match the signature*/
    try {
#ifdef ULTRAIN_TRX_SUPPORT_GM
        return pk.verify(hash.data(), hash.data_size(), sig);
#else
        chain::public_key_type peer_key = chain::public_key_type(sig, hash, true);
        return peer_key == pk;
#endif
    }
    catch (fc::exception& /*e*/) {
        elog("unrecover key error");
        return false;
    }
    elog("unauthenticated key");
    return false;
}
bool net_plugin_impl::is_account_commitee_pk_match(fc::sha256 const& hash,chain::account_name const& account,std::string sig)
{
    controller& cc = chain_plug->chain();
    uint32_t blockNum = cc.head_block_num()+1;
    std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(blockNum);
    consensus::PublicKeyType publicKey = stakeVotePtr->getPublicKey(account);
    if (!Validator::verify<fc::sha256>(consensus::SignatureType(sig), hash, publicKey)) {
        elog("validate error");
        return false;
    }
    return true;
}
bool net_plugin_impl::is_account_bls_pk_match(fc::sha256 const& hash,chain::account_name const& account,std::string sig)
{
    controller& cc = chain_plug->chain();
    uint32_t blockNum = cc.head_block_num()+1;
    std::shared_ptr<StakeVoteBase> voterSysPtr = MsgMgr::getInstance()->getStakeVote(blockNum);
    unsigned char blsPk[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
    bool res = voterSysPtr->getCommitteeBlsPublicKey(account, blsPk, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);
    if (!res) {
        elog("account ${account} is not in committee", ("account", account));
        return false;
    }
    return Validator::verify<fc::sha256>(sig, hash, blsPk);
}
bool net_plugin_impl::is_account_pk_match(chain::public_key_type const& pk,chain::account_name const& account)
{
/*pk match the account*/
    const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
    struct chain_apis::read_only::get_account_info_params get_account_para;
    std::vector<chain::public_key_type> producers_pk;
    get_account_para.account_name =account;
    try {
        auto result = ro_api.get_account_info(get_account_para);
        for ( auto& perm : result.permissions )
        {
            for(auto& key_wei: perm.required_auth.keys)
            {
                producers_pk.push_back(key_wei.key);
            }
        }
    }
    catch (fc::exception& e) {
        ilog("there may be no producer registered: ${e}", ("e", e.to_string()));
        return false;
    }
    catch(const std::exception& e)
    {
          elog( "there is no this name: ${e}", ("e",e.what()));
          return false;
    }
    catch(...)
    {
        ilog("there is no this name,unkown");
        return false;
    }
    auto found_producer_key = std::find(producers_pk.begin(), producers_pk.end(), pk);
    if(found_producer_key == producers_pk.end())
    {
        return false;
    }
    return true;

}
bool net_plugin_impl::authenticate_peer(const handshake_message& msg) {
	if(allowed_connections == None)
		return false;

	if(allowed_connections == Any)
		return true;
	namespace sc = std::chrono;
	sc::system_clock::duration msg_time(msg.time);
	auto time = sc::system_clock::now().time_since_epoch();
	if(time - msg_time > peer_authentication_interval) {
		elog( "Peer ${peer} sent a handshake with a timestamp skewed by more than ${time}.",
				("peer", msg.p2p_address)("time", "1 second")); // TODO Add to_variant for std::chrono::system_clock::duration
		return false;
	}
	if(msg.sig != chain::signature_type() && msg.token != sha256()) {
		sha256 hash = fc::sha256::hash(msg.time);
		if(hash != msg.token) {
			elog( "Peer ${peer} sent a handshake with an invalid token.",
					("peer", msg.p2p_address));
			return false;
		}
		bool is_genesis_fin = is_genesis_finish();
		if(!is_genesis_fin)
		{
			return true;
		}
		/*pk match the signature*/
		bool is_pk_signature_matched = is_pk_signature_match(msg.key,hash,msg.sig);
		if(!is_pk_signature_matched)
		{
			return false;
		}
		/*pk match the account*/
		bool is_account_pk_matched = is_account_pk_match(msg.key,msg.account);
		if(!is_account_pk_matched)
		{
			return false;
		}
		auto allowed_tcp_white = std::find(allowed_tcp_peers.begin(), allowed_tcp_peers.end(),msg.key);
		if(allowed_tcp_white != allowed_tcp_peers.end())
		{
			return true;
		}
		/*pk or username in whitelist or producers*/
		auto allowed_p2p_white = std::find(allowed_peers.begin(), allowed_peers.end(), msg.key);
		if(allowed_p2p_white != allowed_peers.end())
		{
			return true;
		}
		bool is_producer_pk = is_producer_account_pk(msg.account);
		if(!is_producer_pk)
		{
			elog("an unauthorized key");
			return false;
		}
		bool commiteekey_nochecked = true;
		bool blskey_nochecked = true;
		for (auto& ext : msg.ext)
		{
			if(ext.key == handshake_ext::sig_commiteekey)
			{
				string sig_commitee = ext.value;
				bool is_account_commitee_pk_matched = is_account_commitee_pk_match(hash,msg.account,sig_commitee);
				if(!is_account_commitee_pk_matched)
				{
					elog("account_commitee_pk no match");
					return false;
				}

				commiteekey_nochecked = false;
			}
			else if(ext.key == handshake_ext::sig_blskey)
			{
				string sig_blk = ext.value;
				bool is_account_blk_pk_matched = is_account_bls_pk_match(hash,msg.account,sig_blk);
				if(!is_account_blk_pk_matched)
				{
					elog("account blk_pk no  match");
					return false;
				}
				blskey_nochecked = false;
			}
		}
		if(blskey_nochecked || commiteekey_nochecked )
		{
			elog("leak blskey check or commitee check");
			return false;
		}
		return true;
	}
	else
	{
		elog( "Peer sent a handshake with blank signature and token, but this node accepts only authenticated connections.");
		return false;
	}
    return true;
}
   chain::public_key_type net_plugin_impl::get_authentication_key() const {
      if(!private_keys.empty())
         return private_keys.begin()->first;
      /*producer_rpos_plugin* pp = app().find_plugin<producer_rpos_plugin>();
      if(pp != nullptr && pp->get_state() == abstract_plugin::started)
         return pp->first_producer_public_key();*/
      ilog("get_authentication_key empty");
       return chain::public_key_type();
   }

   void
   handshake_initializer::populate( handshake_message &hello, msg_priority p, char style) {
      hello.network_version = net_version_base + net_version;
      hello.chain_id = my_impl->chain_id;
      hello.node_id = my_impl->node_id;
      auto sk_account = private_key_type(app().get_plugin<producer_rpos_plugin>().get_account_sk());
      hello.key = sk_account.get_public_key();
      auto name_account = app().get_plugin<producer_rpos_plugin>().get_account_name();
      hello.account = name_account;
      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> tp = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now());
      hello.time = tp.time_since_epoch().count();
      hello.token = fc::sha256::hash(hello.time);
      hello.sig = sk_account.sign(hello.token);
      // If we couldn't sign, don't send a token.
      if(hello.sig == chain::signature_type())
         hello.token = sha256();
      string sig_commitee = std::string(Signer::sign<fc::sha256>(hello.token, NodeInfo::getMainPrivateKey()));
      hello.ext.push_back({handshake_ext::sig_commiteekey,sig_commitee});
      unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
      NodeInfo::getMainBlsPriKey(sk, Bls::BLS_PRI_KEY_LENGTH);
      string sig_blk = std::string(Signer::sign<fc::sha256>(hello.token, sk));
      hello.ext.push_back({handshake_ext::sig_blskey,sig_blk});
      hello.ext.push_back({handshake_ext::connect_style, std::string(1, style)});
      if (p == msg_priority_rpos) {
         hello.p2p_address = my_impl->rpos_listener.p2p_address + " - " + hello.node_id.str().substr(0,7);
      }
      else {
         hello.p2p_address = my_impl->trx_listener.p2p_address + " - " + hello.node_id.str().substr(0,7);
      }

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


      controller& cc = my_impl->chain_plug->chain();
      hello.head_id = fc::sha256();
      hello.last_irreversible_block_id = fc::sha256();
      hello.head_num = cc.fork_db_head_block_num();
      hello.last_irreversible_block_num = cc.last_irreversible_block_num();
      if( hello.last_irreversible_block_num ) {
         try {
            hello.last_irreversible_block_id = cc.get_block_id_for_num(hello.last_irreversible_block_num);
         }
         catch( const unknown_block_exception &ex) {
            ilog("caught unkown_block");
            hello.last_irreversible_block_num = 0;
         }
      }
      if( hello.head_num ) {
         try {
            hello.head_id = cc.get_block_id_for_num( hello.head_num );
         }
         catch( const unknown_block_exception &ex) {
           hello.head_num = 0;
         }
      }
   }

   net_plugin::net_plugin()
      :my( new net_plugin_impl ) {
      my_impl = my.get();
   }

   net_plugin::~net_plugin() {
   }

   void net_plugin::set_program_options( options_description& /*cli*/, options_description& cfg )
   {
      cfg.add_options()
         ( "p2p-listen-endpoint", bpo::value<string>()->default_value( "0.0.0.0:20122" ), "The actual host:port used to listen for incoming p2p connections.")
         ( "p2p-server-address", bpo::value<string>(), "An externally accessible host:port for identifying this node. Defaults to p2p-listen-endpoint.")
         ( "p2p-peer-address", bpo::value< vector<string> >()->composing(), "The public endpoint of a peer node to connect to. Use multiple p2p-peer-address options as needed to compose a network.")
         ( "rpos-p2p-listen-endpoint", bpo::value<string>()->default_value( "0.0.0.0:20123" ), "The actual host:port used to listen for incoming rpos p2p connections.")
         ( "rpos-p2p-server-address", bpo::value<string>(), "An externally accessible host:port for identifying this node. Defaults to rpos-p2p-listen-endpoint.")
         ( "rpos-p2p-peer-address", bpo::value< vector<string> >()->composing(), "The public endpoint of a peer node to connect to. Use multiple rpos-p2p-peer-address options as needed to compose a network.")
         ( "udp-listen-port", bpo::value<uint16_t>()->default_value(20124), "The udp port used by p2p search.")
         ( "udp-seed", bpo::value< vector<string> >()->composing(), "The udp seed ip to build p2p nodes table. If this option was not set, we would get nothing from p2p nodes table.")
         ( "listen-ip", bpo::value<string>()->default_value( "0.0.0.0" ), "IP that really works,public address first if the node has one.")
         ( "p2p-max-nodes-per-host", bpo::value<int>()->default_value(4), "Maximum number of client nodes from any single IP address")
         ( "agent-name", bpo::value<string>()->default_value("\"ULTRAIN Test Agent\""), "The name supplied to identify this node amongst the peers.")
         ( "allowed-connection", bpo::value<vector<string>>()->multitoken()->default_value({"any"}, "any"), "Can be 'any' or 'producers' or 'specified' or 'none'. If 'specified', peer-key must be specified at least once. If only 'producers', peer-key is not required. 'producers' and 'specified' may be combined.")
         ( "peer-key", bpo::value<vector<string>>()->composing()->multitoken(), "Optional public key of peer allowed to connect.  May be used multiple times.")
         ( "tcp-peer-key", bpo::value<vector<string>>()->composing()->multitoken(), "Optional public key of peer only tcp allowed to connect.  May be used multiple times.")
         ( "peer-private-key", boost::program_options::value<vector<string>>()->composing()->multitoken(),
           "Tuple of [PublicKey, WIF private key] (may specify multiple times)")
         ( "max-static-clients", bpo::value<int>()->default_value(10), "Maximum number of static clients from which connections are accepted.")
         ( "max-dynamic-clients", bpo::value<int>()->default_value(20), "Maximum number of dynamic(address from p2p table) clients from which connections are accepted.")
         ( "max-passive-out-count", bpo::value<uint32_t>()->default_value(10), "Maximum number of passive out connections are accepted.")
         ( "min-connections", bpo::value<int>()->default_value(8), "Minimum number of connections the programme need create, including active and subjective connections")
         ( "max-retry-count", bpo::value<uint32_t>()->default_value(3), "Maximum number of reconnecting to listen endpoint")
         ( "max-grey-list-size", bpo::value<uint32_t>()->default_value(10), "Maximum size of grey list")
         ( "connection-cleanup-period", bpo::value<int>()->default_value(15), "number of seconds to wait before cleaning up dead connections")
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
        ("max-waitblocknum-seconds", bpo::value<int>()->default_value(3), "Time duration for selecting sync block source.")
        ("max-waitblock-seconds",bpo::value<int>()->default_value(12), "Check period for connecting during syncing block.")
        ("nat-mapping", bpo::value<bool>()->default_value(false),"True to enable traverse Nat using upnp.")

        ;
   }

   template<typename T>
   T dejsonify(const string& s) {
      return fc::json::from_string(s).as<T>();
   }

   void net_plugin::plugin_initialize( const variables_map& options ) {
      ilog("Initialize net plugin");
      try {
         peer_log_format = options.at( "peer-log-format" ).as<string>();

         my->network_version_match = options.at( "network-version-match" ).as<bool>();
         my->max_waitblocknum_seconds = options.at( "max-waitblocknum-seconds" ).as<int>();
         my->max_waitblock_seconds = options.at( "max-waitblock-seconds" ).as<int>();
         my->dispatcher.reset( new dispatch_manager );
         my->sync_block_master.reset( new sync_block_manager(my->max_waitblocknum_seconds, my->max_waitblock_seconds) );
         my->light_client = LightClientMgr::getInstance()->getLightClient(0);
         my->light_client->addCallback(std::make_shared<CheckBlockCallback>(*my->sync_block_master));

         my->connector_period = std::chrono::seconds( options.at( "connection-cleanup-period" ).as<int>());
         my->txn_exp_period = def_txn_expire_wait;
         my->resp_expected_period = def_resp_expected_wait;
         my->dispatcher->just_send_it_max = options.at( "max-implicit-request" ).as<uint32_t>();
         my->max_static_clients = options.at( "max-static-clients" ).as<int>();
         my->max_dynamic_clients = options.at( "max-dynamic-clients" ).as<int>();
         my->min_connections = options.at( "min-connections" ).as<int>();
         ULTRAIN_ASSERT( my->max_static_clients + my->max_dynamic_clients > my->min_connections, plugin_config_exception, "max_client_count must be > min_connections");

         my->max_nodes_per_host = options.at( "p2p-max-nodes-per-host" ).as<int>();

         if (options.count( "max-passive-out-count" )) {
            my->max_passive_out_count = options.at( "max-passive-out-count" ).as<uint32_t>();
         }

         if (options.count( "max-retry-count" )) {
            my->max_retry_count = options.at( "max-retry-count" ).as<uint32_t>();
         }

         if (options.count( "max-grey-list-size" )) {
            my->max_grey_list_size = options.at( "max-grey-list-size" ).as<uint32_t>();
         }

         my->num_clients = 0;
         my->num_passive_out = 0;
         my->started_sessions = 0;

         ilog(" max_static_clients: ${msc} max_dynamic_clients: ${mdc} \nmax_passive_out_count: ${mpoc} max_retry_count: ${mrc} max_grey_list_size: ${mgls}",
              ("msc", my->max_static_clients)("mdc", my->max_dynamic_clients)
              ("mpoc", my->max_passive_out_count)("mrc", my->max_retry_count)("mgls", my->max_grey_list_size));

         my->trx_listener.resolver = std::make_shared<tcp::resolver>( std::ref( app().get_io_service()));
         string& trx_p2p_address = my->trx_listener.p2p_address;
         if( options.count( "p2p-listen-endpoint" )) {
            trx_p2p_address = options.at( "p2p-listen-endpoint" ).as<string>();
            auto host = trx_p2p_address.substr( 0, trx_p2p_address.find( ':' ));
            auto port = trx_p2p_address.substr( host.size() + 1, trx_p2p_address.size());
            idump((host)( port ));
            tcp::resolver::query query( tcp::v4(), host.c_str(), port.c_str());
            // Note: need to add support for IPv6 too?

            my->trx_listener.listen_endpoint = *my->trx_listener.resolver->resolve( query );
            my->trx_listener.acceptor.reset( new tcp::acceptor( app().get_io_service()));
         }

         if( options.count( "p2p-server-address" )) {
            trx_p2p_address = options.at( "p2p-server-address" ).as<string>();
         } else {
            if( my->trx_listener.listen_endpoint.address().to_v4() == address_v4::any()) {
               boost::system::error_code ec;
               auto host = host_name( ec );
               if( ec.value() != boost::system::errc::success ) {

                  FC_THROW_EXCEPTION( fc::invalid_arg_exception,
                                      "Unable to retrieve host_name. ${msg}", ("msg", ec.message()));

               }
               auto port = trx_p2p_address.substr(trx_p2p_address.find( ':' ), trx_p2p_address.size());
               trx_p2p_address = host + port;
            }
         }

         my->rpos_listener.resolver = std::make_shared<tcp::resolver>( std::ref( app().get_io_service()));
         string& rpos_p2p_address = my->rpos_listener.p2p_address;
         if( options.count( "rpos-p2p-listen-endpoint" )) {
            rpos_p2p_address = options.at( "rpos-p2p-listen-endpoint" ).as<string>();
            auto host = rpos_p2p_address.substr( 0, rpos_p2p_address.find( ':' ));
            auto port = rpos_p2p_address.substr( host.size() + 1, rpos_p2p_address.size());
            idump((host)( port ));
            tcp::resolver::query query( tcp::v4(), host.c_str(), port.c_str());
            // Note: need to add support for IPv6 too?

            my->rpos_listener.listen_endpoint = *my->rpos_listener.resolver->resolve( query );
            my->rpos_listener.acceptor.reset( new tcp::acceptor( app().get_io_service()));
         }

         if( options.count( "rpos-p2p-server-address" )) {
            rpos_p2p_address = options.at( "rpos-p2p-server-address" ).as<string>();
         } else {
            if( my->rpos_listener.listen_endpoint.address().to_v4() == address_v4::any()) {
               boost::system::error_code ec;
               auto host = host_name( ec );
               if( ec.value() != boost::system::errc::success ) {

                  FC_THROW_EXCEPTION( fc::invalid_arg_exception,
                                      "Unable to retrieve host_name. ${msg}", ("msg", ec.message()));

               }
               auto port = rpos_p2p_address.substr(rpos_p2p_address.find( ':' ), rpos_p2p_address.size());
               rpos_p2p_address = host + port;
            }
         }

         if( options.count( "p2p-peer-address" )) {
            my->trx_active_peers = options.at( "p2p-peer-address" ).as<vector<string> >();
            my->promote_private_address(my->trx_active_peers);
         }
         if( options.count( "rpos-p2p-peer-address" )) {
            my->rpos_active_peers = options.at( "rpos-p2p-peer-address" ).as<vector<string> >();
            my->promote_private_address(my->rpos_active_peers);
         }

         if( options.count( "agent-name" )) {
            my->user_agent_name = options.at( "agent-name" ).as<string>();
         }

         if( options.count( "allowed-connection" )) {
            const std::vector<std::string> allowed_remotes = options["allowed-connection"].as<std::vector<std::string>>();
            for( const std::string& allowed_remote : allowed_remotes ) {
               if( allowed_remote == "any" )
                  my->allowed_connections |= net_plugin_impl::Any;
               else if( allowed_remote == "producers" )
                  my->allowed_connections |= net_plugin_impl::Producers;
               else if( allowed_remote == "specified" )
                  my->allowed_connections |= net_plugin_impl::Specified;
               else if( allowed_remote == "none" )
                  my->allowed_connections = net_plugin_impl::None;
            }
         }

         if( my->allowed_connections & net_plugin_impl::Specified )
            ULTRAIN_ASSERT( options.count( "peer-key" ),
                        plugin_config_exception,
                       "At least one peer-key must accompany 'allowed-connection=specified'" );

         if( options.count( "peer-key" )) {
            const std::vector<std::string> key_strings = options["peer-key"].as<std::vector<std::string>>();
            for( const std::string& key_string : key_strings ) {
               my->allowed_peers.push_back( chain::public_key_type( key_string ));
            }
         }
         if( options.count( "tcp-peer-key" )) {
            const std::vector<std::string> tcp_key_strings = options["tcp-peer-key"].as<std::vector<std::string>>();
            for( const std::string& tcp_key_string : tcp_key_strings ) {
               my->allowed_tcp_peers.push_back( chain::public_key_type( tcp_key_string ));
            }
         }
         for(auto peer: my->allowed_peers)
         {
            ilog("peer key ${key}",("key",peer));
         }

         if( options.count( "peer-private-key" )) {
            ilog("has peer-private-key");
	    const std::vector<std::string> key_id_to_wif_pair_strings = options["peer-private-key"].as<std::vector<std::string>>();
            for( const std::string& key_id_to_wif_pair_string : key_id_to_wif_pair_strings ) {
               auto key_id_to_wif_pair = dejsonify<std::pair<chain::public_key_type, std::string>>(
                     key_id_to_wif_pair_string );
               my->private_keys[key_id_to_wif_pair.first] = chain::private_key_type( key_id_to_wif_pair.second );
            }
         }
         my->chain_plug = app().find_plugin<chain_plugin>();
         my->chain_id = app().get_plugin<chain_plugin>().get_chain_id();
         my->node_id = app().get_plugin<chain_plugin>().get_node_id();
         ilog( "my node_id is ${id} ${chainid}", ("id", my->node_id)("chainid",my->chain_id));

         uint16_t udp_port = 20124;
         if (options.count("udp-listen-port")) {
            udp_port = options.at( "udp-listen-port" ).as<uint16_t>();
         }
         string listen_ip;
         if(options.count("listen-ip")) {
            listen_ip = options.at( "listen-ip" ).as<string>();
         }
         bool traverseNat = false;
        if(options.count("nat-mapping")) {
            traverseNat = options.at( "nat-mapping" ).as<bool>();
        }
         my->kcp_transport = options.at( "kcp-transport" ).as<bool>();

         p2p::NodeIPEndpoint local;
         local.setAddress("0.0.0.0");
         local.setUdpPort(udp_port);
         local.setListenPort(msg_priority_trx, my->trx_listener.listen_endpoint.port());
         local.setListenPort(msg_priority_rpos, my->rpos_listener.listen_endpoint.port());
         p2p::NodeTable::initInstance(std::ref(app().get_io_service()), local, my->node_id, my->chain_id.str(), listen_ip, traverseNat);
	 if (options.count("udp-seed")) {
            my->udp_seed_ip = options.at( "udp-seed" ).as<vector<string> >();
         }
         if (!my->udp_seed_ip.empty()) {
            my->use_node_table = true;
         }
         my->keepalive_timer.reset( new boost::asio::steady_timer( app().get_io_service()));
         my->ticker();
      } FC_LOG_AND_RETHROW()
   }

   void net_plugin::plugin_startup() {
      if (my->trx_listener.acceptor) {
         my->trx_listener.acceptor->open(my->trx_listener.listen_endpoint.protocol());
         my->trx_listener.acceptor->set_option(tcp::acceptor::reuse_address(true));
         my->trx_listener.acceptor->bind(my->trx_listener.listen_endpoint);
         my->trx_listener.acceptor->listen();
         my->start_listen_loop(my->trx_listener.acceptor, msg_priority_trx);
      }

      if (my->rpos_listener.acceptor) {
         my->rpos_listener.acceptor->open(my->rpos_listener.listen_endpoint.protocol());
         my->rpos_listener.acceptor->set_option(tcp::acceptor::reuse_address(true));
         my->rpos_listener.acceptor->bind(my->rpos_listener.listen_endpoint);
         my->rpos_listener.acceptor->listen();
         my->start_listen_loop(my->rpos_listener.acceptor, msg_priority_rpos);
      }
      p2p::NodeTable* nodeTblPtr =   p2p::NodeTable::getInstance();
      {
        if(!my->kcp_transport)
        {
           ilog("kcp_transport");
            nodeTblPtr->needtcpevent.connect( boost::bind(&net_plugin_impl::onNodeTableTcpConnectEvent, my.get(), _1));
        }
	    nodeTblPtr->nodedropevent.connect( boost::bind(&net_plugin_impl::onNodeTableDropEvent, my.get(), _1));
    //nodeTblPtr->pktcheckevent.connect(boost::bind(&net_plugin_impl::authen_whitelist_and_producer,my.get(),_1,_2,_3,_4));
      }
      my->incoming_transaction_ack_subscription = app().get_channel<channels::transaction_ack>().subscribe(boost::bind(&net_plugin_impl::transaction_ack, my.get(), _1));

      my->start_monitors();
      if(!my->kcp_transport){
	  for (auto active_peer : my->trx_active_peers) {
              my->connect(active_peer, msg_priority_trx);
	  }

	  for (auto active_peer : my->rpos_active_peers) {
	      my->connect(active_peer, msg_priority_rpos);
	  }
      }
      auto sk_account = private_key_type(app().get_plugin<producer_rpos_plugin>().get_account_sk());
      nodeTblPtr->set_nodetable_sk(sk_account);
      nodeTblPtr->set_nodetable_pk(sk_account.get_public_key());
      auto name_account = app().get_plugin<producer_rpos_plugin>().get_account_name();
      nodeTblPtr->set_nodetable_account(chain::account_name(name_account));
      nodeTblPtr->doSocketInit();
      if (!my->udp_seed_ip.empty())
      {
         nodeTblPtr->init(my->udp_seed_ip,std::ref(app().get_io_service()));
      }
      if(fc::get_logger_map().find(logger_name) != fc::get_logger_map().end())
         logger = fc::get_logger_map()[logger_name];
   }

    void net_plugin::plugin_shutdown() {
        try {
            ilog( "shutdown.." );
            my->done = true;
            if (my->rpos_listener.acceptor) {
                ilog("close rpos acceptor");
                my->rpos_listener.acceptor->close();
                auto cons = my->connections;
                for (auto con : cons) {
                    if (con->priority == msg_priority_rpos) {
                        my->close(con);
                    }
                }

                my->rpos_listener.acceptor = nullptr;
            }

            if (my->trx_listener.acceptor) {
                ilog("trx rpos acceptor");
                my->trx_listener.acceptor->close();
                auto cons = my->connections;
                for (auto con : cons) {
                    if (con->priority == msg_priority_trx) {
                        my->close(con);
                    }
                }

                my->trx_listener.acceptor = nullptr;
            }
            ilog( "exit shutdown" );
        }
        FC_CAPTURE_AND_RETHROW()
    }

   void net_plugin::broadcast(const ProposeMsg& propose) {
      ilog("broadcast propose msg. blockHash : ${blockHash}", ("blockHash", short_hash(propose.block.id())));
      my->start_broadcast(net_message(propose), msg_priority_rpos);
   }

   void net_plugin::broadcast(const EchoMsg& echo) {
      my->start_broadcast(net_message(echo), msg_priority_rpos);
   }
   void net_plugin::partial_broadcast(const ProposeMsg& msg,bool rtn){
       string sig = msg.signature;
       if(!rtn)
       {
          my->local_msgs.erase(sig);
          return ;
       }
       auto msg_save = my->local_msgs.get<by_sig>().find(sig);
       if(msg_save != my->local_msgs.end()){
          connection_ptr c = msg_save->conn;
          for (auto &conn : my->connections) {
               if (conn != c && conn->priority == msg_priority_rpos && conn->current()) {
                   conn->enqueue(net_message(msg));
               }
           }
          my->local_msgs.erase(sig);
       }
       else{
           broadcast(msg);
       }
   }
   void net_plugin::partial_broadcast(const EchoMsg& msg,bool rtn){
       string sig = msg.signature;
       if(!rtn)
       {
          my->local_msgs.erase(sig);
          return ;
       }
       auto msg_save = my->local_msgs.get<by_sig>().find(sig);
       if(msg_save != my->local_msgs.end()){
          connection_ptr c = msg_save->conn;
          for (auto &conn : my->connections) {
               if (conn != c && conn->priority == msg_priority_rpos && conn->current()) {
                   conn->enqueue(net_message(msg));
               }
           }
          my->local_msgs.erase(sig);
       }
       else{
           broadcast(msg);
       }
   }
   void net_plugin::broadcast(const SignedTransaction& trx) {
      my->start_broadcast(trx);
   }

   void net_plugin::send_block(const fc::sha256& node_id, const ultrainio::SyncBlockMsg& msg) {
       ilog("send block msg to node:${node} block num:${n} seq num:${sn}", ("node", node_id)("n", msg.block.block_num())("sn", msg.seqNum));
       my->send_block(node_id, net_message(msg));
   }

   bool net_plugin::send_req_sync(const ultrainio::ReqSyncMsg& msg) {
       ilog("send req sync msg");
       return my->send_req_sync(msg);
   }

   void net_plugin::send_block_num_range(const fc::sha256& node_id, const ultrainio::RspBlockNumRangeMsg& msg) {
      ilog("send block num range:${f} - ${l} hash:${h} prev hash:${ph}", ("f", msg.firstNum)("l", msg.lastNum)("h", msg.blockHash)("ph", msg.prevBlockHash));
      my->send_block_num_range(node_id, net_message(msg));
   }

   void net_plugin::stop_sync_block() {
       ilog("stop sync block");
       my->stop_sync_block();
   }

   size_t net_plugin::num_peers() const {
      return my->count_open_sockets();
   }

   /**
    *  Used to trigger a new connection from RPC API
    */
   string net_plugin::connect(const string& host) {
      return my->connect(host);
   }

   string net_plugin::disconnect( const string& host ) {
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
#if 0
    optional<connection_status> net_plugin::status( const string& host )const {
        auto con = my->find_connection( host );
        if( con )
            return con->get_status();
        return optional<connection_status>();
    }

    vector<connection_status> net_plugin::connections()const {
        vector<connection_status> result;
        result.reserve( my->connections.size() );
        for( const auto& c : my->connections ) {
            result.push_back( c->get_status() );
        }
        return result;
    }
#endif
    vector<connection_status> net_plugin::get_connected_connections()const {
        vector<connection_status> result;
        result.reserve( my->connections.size() );
        for( const auto& c : my->connections ) {
            if(c->current())
            {
                result.push_back( c->get_status() );
            }
        }
        return result;
    }
    int net_plugin::is_netplugin_prime()const{
        if(my->kcp_transport){
            return 0;
        }
        return 1;
    }
    int net_plugin::get_sync_waitblock_interval(){
        return my->max_waitblock_seconds;
    }
    int net_plugin::get_sync_waitblocknum_interval(){
        return my->max_waitblocknum_seconds;
    }

    connection_ptr net_plugin_impl::find_connection(const string& host )const {
        for( const auto& c : connections )
            if( c->peer_addr == host ) return c;
        return connection_ptr();
    }

    bool net_plugin_impl::is_grey_connection(const string& host) const {
        for (const auto& addr : peer_addr_grey_list) {
          if (addr == host) {
            return true;
          }
        }

        return false;
    }

    bool net_plugin_impl::is_connection_to_seed(connection_ptr con) const {
        auto colon = con->peer_addr.find(':');
        if (colon != std::string::npos) {
            auto host = con->peer_addr.substr(0, colon);
            return std::find(udp_seed_ip.begin(), udp_seed_ip.end(), host) != udp_seed_ip.end();
        }

        boost::system::error_code ec;
        std::string addr{con->socket->remote_endpoint(ec).address().to_string()};
        return std::find(udp_seed_ip.begin(), udp_seed_ip.end(), addr) != udp_seed_ip.end();
    }

    bool net_plugin_impl::is_static_connection(connection_ptr con) const {
        if (con->priority == msg_priority_trx) {
            return std::find(trx_active_peers.begin(), trx_active_peers.end(), con->peer_addr) != trx_active_peers.end();
        } else {
            return std::find(rpos_active_peers.begin(), rpos_active_peers.end(), con->peer_addr) != rpos_active_peers.end();
        }
    }

    void net_plugin_impl::del_connection_with_node_id(const fc::sha256& node_id, connection_direction dir,string addr) {
	boost::system::error_code ec;
        auto it = connections.begin();
        ilog("connections size: ${s}", ("s", connections.size()));
        while (it != connections.end()) {
            if ((*it)->node_id == node_id && (*it)->direct == dir && (*it)
                    && !strcmp((*it)->socket->remote_endpoint(ec).address().to_string().c_str(),addr.c_str())) {
                ilog("del connection to ${peer}, node id: ${id}", ("peer", (*it)->peer_addr)("id", node_id));
                if ((*it)->socket && (*it)->socket->is_open()) {
                    close(*it);
                }
                it = connections.erase(it);
            } else {
                ++it;
            }
        }
    }

    uint16_t net_plugin_impl::to_protocol_version (uint16_t v) {
        if (v >= net_version_base) {
            v -= net_version_base;
            return (v > net_version_range) ? 0 : v;
        }
        return 0;
    }

    void net_plugin_impl::promote_private_address(vector<string>& peers) {
        fc::ip::endpoint ep;
        vector<string> private_peers;
        vector<string> public_peers;

        for (auto& p : peers) {
            ep = fc::ip::endpoint::from_string(p);
            if (ep.get_address().is_private_address()) {
                private_peers.push_back(p);
            } else {
                public_peers.push_back(p);
            }
        }
        private_peers.insert(private_peers.end(), public_peers.begin(), public_peers.end());
        peers.swap(private_peers);
    }
}
}
