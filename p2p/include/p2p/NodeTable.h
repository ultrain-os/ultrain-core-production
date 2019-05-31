#pragma once

#include <string>
#include <set>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <chrono>
#include <boost/integer/static_log2.hpp>
#include "Common.h"
#include "UDP.h"
#include <boost/signals2/signal.hpp>
#include <ultrainio/chain/types.hpp>
using namespace boost::asio;

namespace ba = boost::asio;
namespace bi = boost::asio::ip;

namespace ultrainio
{
namespace p2p
{
 using boost::signals2::signal;
    /*UDP notify tcp*/
/**
 * NodeEntry
 * @brief Entry in Node Table
 */
struct NodeEntry: public Node
{
    NodeEntry(NodeID const& localNode, NodeID const& _pubk, NodeIPEndpoint const& _gw);
    int const distance;	          ///< Node's distance (xor of _src as integer).
    bool pending = true;          ///< Node will be ignored until Pong is received
    bool hasValidEndpointProof() const
    {
        return fc::time_point::now() < pongexpiredtime ;
    }
    fc::time_point pongexpiredtime = fc::time_point::min();
};

class NodeTable;
inline std::ostream& operator<<(std::ostream& _out, NodeTable const& _nodeTable);

class NodeTable: UDPSocketEvents
{
    friend std::ostream& operator<<(std::ostream& _out, NodeTable const& _nodeTable);
    using NodeSocket = UDPSocket<NodeTable, 1280>;
    //using TimePoint = std::chrono::steady_clock::time_point;
    using NodeIdTimePoint = std::pair<NodeID, fc::time_point>;

    struct NodeValidation
    {
        fc::time_point pingSendTime;
	int sendtimes;
        boost::optional<NodeID> replacementNodeID;
    };
public:
    static constexpr uint32_t c_bondingTimeMSeconds{3000000};
 
    /// Constructor requiring host for I/O, credentials, and IP Address and port to listen on.
    NodeTable(ba::io_service& _io, NodeIPEndpoint const& _endpoint, NodeID const& nodeID, string const& chainid, bool _enabled = true);
    ~NodeTable();
    NodeID const m_hostNodeID;
    string const m_chainid;
    /// Returns distance based on xor metric two node ids. Used by NodeEntry and NodeTable.
    static int distance(NodeID const& _a, NodeID const& _b)
    {
       NodeID  d = (_a) ^ (_b); 
       unsigned ret;
       for (ret = 0; ((d=d >> 1) != fc::sha256()); ++ret) {}; 
       return ret;
    }

    void init( const std::vector <std::string> &seeds,ba::io_service &_io);

    /// Add node. Node will be pinged and empty shared_ptr is returned if node has never been seen or NodeID is empty.
    void addNode(Node const& _node);
    void printallbucket();
    void addNodePkList(NodeID const& _id,chain::public_key_type const& _pk,chain::account_name const& _account);

    /// Returns list of node ids active in node table.
    std::list<NodeID> nodes() const;

    std::list<NodeIPEndpoint> getNodes();
/// Returns the Node to the corresponding node id or the empty Node if that id is not found.
    Node node(NodeID const& _id);
    signal<void(const NodeIPEndpoint&)> nodeaddevent;
    signal<void(const NodeIPEndpoint&)> nodedropevent;
    signal<bool(const fc::sha256&,const chain::public_key_type&,const chain::signature_type&,chain::account_name const& _account)> pktcheckevent;
    chain::public_key_type m_pk;
    chain::private_key_type m_sk;
    void set_nodetable_pk(chain::public_key_type pk){m_pk = pk;}
    void set_nodetable_sk(chain::private_key_type sk){m_sk = sk;}
    chain::account_name m_account;
    void set_nodetable_account(chain::account_name _account){m_account = _account ;}
#if defined(BOOST_AUTO_TEST_SUITE) || defined(_MSC_VER) // MSVC includes access specifier in symbol name
protected:
#else
private:
#endif

    /// Constants for Kademlia, derived from address space.

    //  static unsigned const s_addressByteSize = h256::size;            ///< Size of address type in bytes.
    static unsigned const s_bits = 256;	                                 ///< Denoted by n in [Kademlia].
    static unsigned const s_bins = s_bits - 1;                           ///< Size of m_state (excludes root, which is us).
    static unsigned const s_maxSteps = boost::static_log2<s_bits>::value;///< Max iterations of discovery. (discover)

    /// Chosen constants
    static unsigned const s_bucketSize = 50;			///< Denoted by k in [Kademlia]. Number of nodes stored in each bucket.
    static unsigned const s_alpha = 3;				///< Denoted by \alpha in [Kademlia]. Number of concurrent FindNode requests.
 

    struct NodeBucket
    {
        unsigned distance;
        std::list<std::weak_ptr<NodeEntry>> nodes;
    };

    /// Used to ping endpoint.
    void ping(NodeIPEndpoint _to,NodeID id);

    /// Used ping known node. Used by node table when refreshing buckets and as part of eviction process (see evict).
    void ping(NodeEntry const&  _nodeEntry,boost::optional<NodeID> const& _replacementNodeID = {});

    /// Used by asynchronous operations to return NodeEntry which is active and managed by node table.
    std::shared_ptr<NodeEntry> nodeEntry(NodeID _id);

    /// Used to discovery nodes on network which are close to the given target.
    /// Sends s_alpha concurrent requests to nodes nearest to target, for nodes nearest to target, up to s_maxSteps rounds.
    void doDiscover(NodeID _target, unsigned _round, std::shared_ptr<std::set<std::shared_ptr<NodeEntry>>> _tried);
    /// Returns nodes from node table which are closest to target.
    std::vector<std::shared_ptr<NodeEntry>> nearestNodeEntries(NodeID _target);

    /// Asynchronously drops _leastSeen node if it doesn't reply and adds _new node, otherwise _new node is thrown away.
    void evict(NodeEntry const& _leastSeen, NodeEntry const& _new);
    /// Called whenever activity is received from a node in order to maintain node table.
    void noteActiveNode(NodeID const& _pubk, NodeIPEndpoint const& _endpoint);

    /// Used to drop node when timeout occurs or when evict() result is to keep previous node.
    void dropNode(std::shared_ptr<NodeEntry> _n);

    /// Returns references to bucket which corresponds to distance of node id.
    /// @warning Only use the return reference locked x_state mutex.
    // TODO p2p: Remove this method after removing offset-by-one functionality.
    NodeBucket& bucket_UNSAFE(NodeEntry const* _n);

    /// General Network Events

    /// Called by m_socket when packet is received.
    void handlemsg( bi::udp::endpoint const& _from, PingNode const& pingmsg );
    void handlemsg( bi::udp::endpoint const& _from, Pong const& pongmsg ) ;
    void handlemsg( bi::udp::endpoint const& _from, FindNode const& findnodemsg ) ;
    void handlemsg( bi::udp::endpoint const& _from, Neighbours const& msg ) ;
    /// Called by m_socket when socket is disconnected.
    void onSocketDisconnected() {}
    ///timers
    boost::asio::steady_timer::duration   nodetimeoutinterval{std::chrono::seconds{30}};
    std::unique_ptr<boost::asio::steady_timer> checknodereachable_timer;
    boost::asio::steady_timer::duration   pingtimeoutinterval{std::chrono::seconds{10}};
    std::unique_ptr<boost::asio::steady_timer> pingtimeout_timer;
    boost::asio::steady_timer::duration   discoverinterval{std::chrono::seconds{7}};
    std::unique_ptr<boost::asio::steady_timer> discover_timer;
    boost::asio::steady_timer::duration   discoversplitinterval{std::chrono::milliseconds{600}};
    std::unique_ptr<boost::asio::steady_timer> discover_splittimer;

    boost::asio::steady_timer::duration   idrequestinterval{std::chrono::seconds{2}};
    std::unique_ptr<boost::asio::steady_timer> idrequest_timer;

    boost::asio::steady_timer::duration   nodesrefindinterval{std::chrono::seconds{30}};
    std::unique_ptr<boost::asio::steady_timer> nodesrefindtimer;
    
    boost::asio::steady_timer::duration   packlimitcheckinterval{std::chrono::seconds{30}};
    std::unique_ptr<boost::asio::steady_timer> packlimitchecktimer;

    void start_p2p_monitor(ba::io_service& _io);
    void doHandlePingTimeouts();
    void doHandleNodeTimeouts();
    void doPackLimitTimeouts();
    void doNodeTimeoutCheck();
    void doPingTimeoutCheck();
    void doPackLimitCheck();
    /// Looks up a random node at @c_bucketRefresh interval.
    void doDiscovery();
    void doIDRequest();
    void doIDRequestCheck();
    void doNodeReFindTimeouts();
    void doNodeRefindCheck();
    NodeIPEndpoint m_hostNodeEndpoint;

    std::unordered_map<NodeID, std::shared_ptr<NodeEntry>> m_nodes;     ///< Known Node Endpoints
    struct node_feature
    {
          chain::public_key_type pk;
          chain::account_name account;
    };
    std::unordered_map<NodeID, node_feature> m_pknodes;     ///< Known Node Endpoints


    std::array<NodeBucket, s_bins> m_state;                             ///< State of p2p node network.

    std::unordered_map<NodeID, NodeValidation> m_sentPings; 
    std::unordered_map<NodeID, int> m_PingsBad; 

    std::unordered_map<bi::address, fc::time_point> m_pubkDiscoverPings;///< List of pending pings where node entry wasn't created due to unkown pubk.

    std::unordered_map<NodeID, fc::time_point> m_sentFindNodes;
    std::shared_ptr<NodeSocket> m_socket;                       ///< Shared pointer for our UDPSocket; ASIO requires shared_ptr.
    vector<string> m_seeds;
    void requireSeeds(const std::vector <std::string> &seeds);
    bool isnodevalid(Node const& _node);
    void recordBadNode(NodeID const& _id);
    bool isPrivateAddress(bi::address const& _addressToCheck);
    bool isLocalHostAddress(bi::address const& _addressToCheck);
}; // end of class NodeTable

} // end of namespace p2p
} // end of namespace ultrainio

