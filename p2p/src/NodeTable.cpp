#include "p2p/NodeTable.h"
#include "p2p/Common.h"
#include <fc/crypto/rand.hpp>
#include <memory>

using namespace std;

namespace ultrainio
{
namespace p2p {
    using boost::asio::ip::tcp;
    using boost::asio::ip::address_v4;
    using boost::asio::ip::host_name;
    constexpr int c_findTimeoutMicroSec = 1000000;

    inline bool operator==(
            std::weak_ptr < NodeEntry >
    const& _weak,
    std::shared_ptr <NodeEntry> const &_shared
    ) {
    return !_weak.
    owner_before(_shared)
    && !_shared.
    owner_before(_weak);
}

//TODO:JWN----alldistance 0
    NodeEntry::NodeEntry(NodeID const &localID, NodeID const &_pubk, NodeIPEndpoint const &_gw) : Node(_pubk, _gw),
                                                                                                  distance(
                                                                                                          NodeTable::distance(
                                                                                                                  localID,
                                                                                                                  _pubk)) {}


    NodeTable::NodeTable(ba::io_service &_io, NodeIPEndpoint const &_endpoint, NodeID const &nodeID, string const& chainid, bool _enabled) :
            m_hostNodeID(nodeID),
            m_chainid(chainid),
            m_hostNodeEndpoint(_endpoint),
            m_socket(make_shared<NodeSocket>(_io, *reinterpret_cast<UDPSocketEvents *>(this),
                                             (bi::udp::endpoint) _endpoint))
    {
        for (unsigned i = 0; i < s_bins; i++)
            m_state[i].distance = i;
        if (!_enabled)
            return;
	ilog("chain_id ${id}",("id",m_chainid));
    }

    NodeTable::~NodeTable() {
        m_socket->disconnect();
    }
//TODO::rpos seed& trx seed to one seed and must be same
void NodeTable::init( const std::vector <std::string> &seeds,ba::io_service &_io) {
	try {
		m_socket->connect();
		start_p2p_monitor(_io);
	}
	catch (std::exception const &_e) {
		elog("Exception connecting NodeTable socket:");
	}
	m_seeds = seeds;
	requireSeeds(m_seeds);
	doIDRequest();
}
void NodeTable::requireSeeds(const std::vector <std::string> &seeds)
{
    for (auto seed : seeds) {
        p2p::NodeIPEndpoint peer;
        peer.setAddress(seed);
        peer.setUdpPort(20124);
        p2p::NodeID id = fc::sha256();
        p2p::Node node(id, peer);
        m_pubkDiscoverPings[boost::asio::ip::address::from_string(node.m_endpoint.address())] = fc::time_point::now();
        ping(node.m_endpoint,id);
    }
    ilog("seeds size ${size} disping size ${pingsize}",("size",seeds.size())("pingsize",m_pubkDiscoverPings.size()));
}
void NodeTable::doIDRequestCheck()
{
    bi::udp::endpoint localep = (bi::udp::endpoint)m_hostNodeEndpoint;
    if (m_pubkDiscoverPings.count(localep.address()))
    {
        m_pubkDiscoverPings.erase(localep.address());
    }
    if(m_pubkDiscoverPings.size() !=0 )
    {
        for(auto& i :m_pubkDiscoverPings)
        {
            NodeIPEndpoint ep;
            ep.setAddress(i.first.to_string());
            ep.setUdpPort(20124);
            ping(ep,fc::sha256());
        }
        doIDRequest();
    }
}
void NodeTable::doIDRequest()
{
    idrequest_timer->expires_from_now( idrequestinterval);
    idrequest_timer->async_wait( [this](boost::system::error_code ec) {
        if (ec)
        {
            elog("doIDRequest timer error: ${ec}",("ec",ec.message()));

        }
        doIDRequestCheck();

    });

}
void NodeTable::addNodePkList(NodeID const& _id,chain::public_key_type const& _pk,chain::account_name const& account)
{
    if (_id == fc::sha256())
    {
            elog("do not add node without nodeID");
            return ;
    }
    if (m_hostNodeID == _id)
    {
        ilog("do not add local nodeID");
        return;
    }
    if(m_pknodes.count(_id))
    {
	    return ;
    }
    ilog("addNodePkList");
    m_pknodes[_id] = {_pk,account};
}
void NodeTable::addNode(Node const& _node, NodeRelation _relation)
{
    //TODO:JWN:can restore somehow,then the relation is Known
    if (_relation == Known)
    {
        ilog(" addNode Known");
        auto ret = make_shared<NodeEntry>(m_hostNodeID, _node.m_id, _node.m_endpoint);
        ret->pending = false;
      //  DEV_GUARDED(x_nodes)
        m_nodes[_node.m_id] = ret;
	noteActiveNode(_node.m_id, _node.m_endpoint);
        return ;
    }

    if (!_node.m_endpoint)
    {
	elog("do not add node without ep info");
        return ;
    }
    // ping address to recover nodeid if nodeid is empty
    if (_node.m_id == fc::sha256())
    {
	    elog("do not add node without nodeID");
	    return ;
    }
    if (m_hostNodeID == _node.m_id)
    {
        ilog("do not add local nodeID");
        return;
    }

    if (m_nodes.count(_node.m_id))
    {
        return ;
    }

    auto nodeEntry = make_shared<NodeEntry>(m_hostNodeID, _node.m_id, _node.m_endpoint);
    m_nodes[_node.m_id] = nodeEntry;
    ping(*nodeEntry);
    return ;
}

list<NodeID> NodeTable::nodes() const
{
    list<NodeID> nodes;
    for (auto& i: m_nodes)
        nodes.push_back(i.second->m_id);
    return nodes;
}
list<NodeIPEndpoint> NodeTable::getNodes()
{
	list<NodeIPEndpoint> nodes;
	NodeID randNodeId = fc::sha256();
	fc::rand_pseudo_bytes( randNodeId.data(), randNodeId.data_size());
	auto const nearestNodes = nearestNodeEntries(randNodeId);
	for (auto const& node : nearestNodes)
	{
		nodes.push_back(node->m_endpoint);
	}
	return nodes;
}
Node NodeTable::node(NodeID const& _id)
{
 //   Guard l(x_nodes);
    if (m_nodes.find(_id) != m_nodes.end())
    {
        auto entry = m_nodes[_id];
        return Node(_id, entry->m_endpoint);
    }
    Node UnspecifiedNode;
    NodeIPEndpoint UnspecifiedNodeIPEndpoint;
    UnspecifiedNodeIPEndpoint.m_address = bi::address().to_string();
    UnspecifiedNodeIPEndpoint.m_udpPort = 0;
    UnspecifiedNode = Node(NodeID(), UnspecifiedNodeIPEndpoint);
    return UnspecifiedNode;
}
shared_ptr<NodeEntry> NodeTable::nodeEntry(NodeID _id)
{
    auto const it = m_nodes.find(_id);
    return it != m_nodes.end() ? it->second : shared_ptr<NodeEntry>();
}

void NodeTable::doDiscover(NodeID _node, unsigned _round, shared_ptr<set<shared_ptr<NodeEntry>>> _tried)
{
    // NOTE: ONLY called by doDiscovery!
    
    if (!m_socket->isOpen())
        return;
    
    auto const nearestNodes = nearestNodeEntries(_node);
    ilog("doDiscover size ${size}",("size",nearestNodes.size()));
    auto newTriedCount = 0;
    for (auto const& node : nearestNodes)
    {
        if (!_tried->count(node)) 
        {
            FindNode p;
            p.type = 3;
            p.fromID = m_hostNodeID;
            p.targetID = _node;
            p.destid = node->m_id;
            p.fromep = m_hostNodeEndpoint;
            p.tartgetep = node->m_endpoint;
            p.chain_id = m_chainid;
            p.pk = m_pk;
	    p.account = m_account;
            fc::sha256 digest = fc::sha256::hash<UnsignedFindNode>(p);
            p.signature = std::string(m_sk.sign(digest)); 
            m_socket->send_msg(p,(bi::udp::endpoint)node->m_endpoint);
            m_sentFindNodes[node->m_id] = fc::time_point::now();
            _tried->emplace(node);
            if (++newTriedCount == s_alpha)
                break;
        }
    }
    if (_round == s_maxSteps || newTriedCount == 0)
    {
        ilog("Terminating discover after round ${round}",("round",_round));
        doDiscovery();
        return;
    }
    discover_splittimer->expires_from_now(discoversplitinterval);
    discover_splittimer->async_wait( [this, _node, _round, _tried](boost::system::error_code _ec) {
        if (_ec.value() == boost::asio::error::operation_aborted ) {
            elog("discover_splittimer  was probably cancelled:");
        }
        else
        {
            if(_ec)
            {
                elog("discover_splittimer  error");
            }
            doDiscover(_node, _round + 1, _tried);
        }
    });
}

vector<shared_ptr<NodeEntry>> NodeTable::nearestNodeEntries(NodeID _target)
{
    // send s_alpha FindNode packets to nodes we know, closest to target
    static unsigned lastBin = s_bins - 1;
    unsigned head = distance(m_hostNodeID, _target);
    unsigned tail = head == 0 ? lastBin : (head - 1) % s_bins;
    
    map<unsigned, list<shared_ptr<NodeEntry>>> found;
    
    // if d is 0, then we roll look forward, if last, we reverse, else, spread from d
    if (head > 1 && tail != lastBin)
        while (head != tail && head < s_bins)
        {
       //     Guard l(x_state);
            for (auto const& n: m_state[head].nodes)
                if (auto p = n.lock())
                    found[distance(_target, p->m_id)].push_back(p);

            if (tail)
                for (auto const& n: m_state[tail].nodes)
                    if (auto p = n.lock())
                        found[distance(_target, p->m_id)].push_back(p);

            head++;
            if (tail)
                tail--;
        }
    else if (head < 2)
        while (head < s_bins)
        {
       //     Guard l(x_state);
            for (auto const& n: m_state[head].nodes)
                if (auto p = n.lock())
                    found[distance(_target, p->m_id)].push_back(p);
            head++;
        }
    else
        while (tail > 0)
        {
       //     Guard l(x_state);
            for (auto const& n: m_state[tail].nodes)
                if (auto p = n.lock())
                    found[distance(_target, p->m_id)].push_back(p);
            tail--;
        }
    
    vector<shared_ptr<NodeEntry>> ret;
    for (auto& nodes: found)
        for (auto const& n: nodes.second)
            if (ret.size() < 16 && n->m_endpoint )
            {
                ret.push_back(n);
            }
                return ret;
}

void NodeTable::ping(NodeIPEndpoint _to,NodeID _toID)
{
    NodeIPEndpoint src;
    src = m_hostNodeEndpoint;
    PingNode p;
    p.type = 1;
    p.source = src;
    p.dest = _to;
    p.sourceid = m_hostNodeID;
    p.destid = _toID;
    p.chain_id = m_chainid;
    p.pk = m_pk; 
    p.account = m_account;
    fc::sha256 digest = fc::sha256::hash<UnsignedPingNode>(p); 
    p.signature = std::string(m_sk.sign(digest));
    m_socket->send_msg(p,(bi::udp::endpoint)_to);
}
void NodeTable::ping(NodeEntry const& _nodeEntry, boost::optional<NodeID> const& _replacementNodeID)
{
    auto const sentPing = m_sentPings.find(_nodeEntry.m_id);
    if (sentPing == m_sentPings.end())
    {
        m_sentPings[_nodeEntry.m_id] = {fc::time_point::now(),1, _replacementNodeID};
    }
    else
    {
        int sendtimes = sentPing->second.sendtimes;
        m_sentPings.erase(sentPing);
        m_sentPings[_nodeEntry.m_id] = {fc::time_point::now(),sendtimes+1, _replacementNodeID};
        ilog("ping times ${time}",("time",sendtimes));
    }
    ping(_nodeEntry.m_endpoint,_nodeEntry.m_id);

}
void NodeTable::evict(NodeEntry const& _leastSeen, NodeEntry const& _new)
{
    if (!m_socket->isOpen())
        return;

     ilog("evict old ${old} new ${new}",("old",_leastSeen.m_endpoint.address())("new",_new.m_endpoint.address()));
     ping(_leastSeen, _new.m_id);
}
void NodeTable::noteActiveNode(NodeID const& _pubk, NodeIPEndpoint const& _endpoint)
{
    bool buket_chg_flag = false;
    if (_pubk == m_hostNodeID)
    {
        return ;//TODO:JWN ip check
    }

    shared_ptr<NodeEntry> newNode = nodeEntry(_pubk);
    if (newNode)//TODO:JWN time check
    {
      //  newNode->m_endpoint.setAddress(_endpoint.address().to_string());
      //  newNode->m_endpoint.setUdpPort(_endpoint.port());
        newNode->m_endpoint = _endpoint;

        shared_ptr<NodeEntry> nodeToEvict;
        {
            NodeBucket& s = bucket_UNSAFE(newNode.get());
            auto& nodes = s.nodes;
            auto it = std::find(nodes.begin(), nodes.end(), newNode);
            if (it != nodes.end())
            {
                nodes.splice(nodes.end(), nodes, it);
            }
            else
            {
		        buket_chg_flag = true;
                if (nodes.size() < s_bucketSize)
                {
                   nodes.push_back(newNode);
                   nodeaddevent(newNode->m_endpoint); 
                }
                else
                {
                    nodeToEvict = nodes.front().lock();

                    if (!nodeToEvict)
                    {
                        ilog("noteActive change node");
                        nodes.pop_front();
                        nodes.push_back(newNode);
                        nodeaddevent(newNode->m_endpoint); 

                    }
                }

            }
        }

        if (nodeToEvict)
        {
            evict(*nodeToEvict, *newNode);
            
        }

    }
    if(buket_chg_flag)
    {
       printallbucket();
    }
}
void NodeTable::dropNode(shared_ptr<NodeEntry> _n)
{
	NodeBucket& s = bucket_UNSAFE(_n.get());
	s.nodes.remove_if(
			[_n](weak_ptr<NodeEntry> const& _bucketEntry) { return _bucketEntry == _n; });

	m_nodes.erase(_n->m_id);
	m_pknodes.erase(_n->m_id);
        auto sentPingBad = m_PingsBad.find(_n->m_id);
        if(sentPingBad != m_PingsBad.end())
	{
		nodedropevent(_n->m_endpoint);
	}
        m_PingsBad.erase(_n->m_id);
// notify host
	ilog("p2p.nodes.drop id ${id} ep ${ep}",("id",_n->m_id)("ep",_n->m_endpoint.address()));
	printallbucket();
}

NodeTable::NodeBucket& NodeTable::bucket_UNSAFE(NodeEntry const* _n)
{//TODO::JWN
        return m_state[_n->distance - 1];
}
void NodeTable::printallbucket()
{
	for (auto const& s : m_state)
		for (auto const& np : s.nodes)
		{
			if (auto n = np.lock())
			{
				ilog("bucket node ${ip} ${udp} ${trx} ${rpos}",("ip",(*n).m_endpoint.address())("udp",(*n).m_endpoint.udpPort())("trx",(*n).m_endpoint.listenPort(msg_priority_trx))("rpos",(*n).m_endpoint.listenPort(msg_priority_rpos)));
			}
		}
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, Pong const& pong ) {
    if(pong.sourceid == fc::sha256())
    {
        elog("pong msg has no id");
        return ;
    }
    if(pong.destid != m_hostNodeID)
    {
        elog("pong msg not for me");
        return ;
    }
    if(pong.chain_id != m_chainid)
    {
        elog("wrong chain");
	return ;
    }
    for(auto it = m_pknodes.begin(); it != m_pknodes.end();++it)
    {
	    if((it->second.account == pong.account) && (it->first != pong.sourceid))
	    {
		    elog("duplicate pong p2p pk");
		    return ;
	    }
    } 
    fc::sha256 digest_pong = fc::sha256::hash<p2p::UnsignedPong>(pong);
    bool isvalid = *pktcheckevent(digest_pong,pong.pk,chain::signature_type(pong.signature),pong.account);
    if(!isvalid)
    {
        elog("pong msg check fail:may not in whitelist or not a producer");
        recordBadNode(pong.sourceid); 
        return ;
    }
    auto const& sourceId = pong.sourceid;
    auto const sentPing = m_sentPings.find(sourceId);
    if (sentPing == m_sentPings.end())
    {
        ilog("Unexpected PONG from ${addr}",("addr",_from.address().to_string()));
        return;
    }
     auto it = m_nodes.find(sourceId);
     if(it == m_nodes.end())
     {
         return ;
     }
    auto sourceNodeEntry = it->second ;
    sourceNodeEntry->pongexpiredtime = fc::time_point::now() + fc::microseconds(c_bondingTimeMSeconds);
    auto const& optionalReplacementID = sentPing->second.replacementNodeID;
    if (optionalReplacementID)
    {
        if (auto replacementNode = nodeEntry(*optionalReplacementID))
        {
            dropNode(move(replacementNode));
        }
    }
    m_sentPings.erase(sentPing);
    
    if (!m_hostNodeEndpoint)
     {
	    ilog("local m_hostNodeEndpoint before ${host}",("host",m_hostNodeEndpoint.address()));
        m_hostNodeEndpoint.setAddress(pong.destep.address());
        m_hostNodeEndpoint.setUdpPort(pong.destep.udpPort());
	    ilog("local m_hostNodeEndpoint after ${host}",("host",m_hostNodeEndpoint.address()));
     }
    NodeIPEndpoint from;
    from.m_address = _from.address().to_string();
    from.m_udpPort = _from.port();
    from.m_listenPorts = pong.fromep.m_listenPorts;
    noteActiveNode(pong.sourceid, from);
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, FindNode const& in ) {
   // ilog("handle Find srcnodeid ${nodeid} tarID ${tarID} from ${src} dest ${des}",("nodeid",in.fromID)("tarID",in.targetID)("src",in.fromep)("des",in.tartgetep));
    if(in.fromID == fc::sha256())
    {
        ilog("Find msg has no id");
        return ;
    }
    if(in.destid != m_hostNodeID)
    {
        elog("FindNode msg not for me");
        return ;
    }
    if(in.chain_id != m_chainid)
    {
	    elog("wrong chain");
	    return ;
    }
    for(auto it = m_pknodes.begin(); it != m_pknodes.end();++it)
    {
	    if((it->second.account == in.account) && (it->first != in.fromID))
	    {
		    elog("duplicate find p2p pk");
		    return ;
	    }
    }
    fc::sha256 digest = fc::sha256::hash<p2p::UnsignedFindNode>(in);
    bool isvalid = *pktcheckevent(digest,in.pk,chain::signature_type(in.signature),in.account);
    if(!isvalid)
    {
            elog("FindNode msg check fail:may not in whitelist or not a producer");
            recordBadNode(in.fromID); 
            return ;
    }
    vector<shared_ptr<NodeEntry>> nearest = nearestNodeEntries(in.targetID);
    static unsigned constexpr nlimit = (NodeSocket::maxDatagramSize - 109) / 90;
    NodeIPEndpoint from;
    from.m_address = _from.address().to_string();
    from.m_udpPort = _from.port();
    from.m_listenPorts = in.fromep.m_listenPorts;
    for (unsigned offset = 0; offset < nearest.size(); offset += nlimit)
    {
       Neighbours out;
       out.type = 4;
       out.fromID = m_hostNodeID;
       out.fromep = m_hostNodeEndpoint;
       out.tartgetep =  from;
       out.destid = in.fromID;
       out.chain_id = m_chainid;
       out.pk = m_pk;
       out.account = m_account;
       auto _limit = nlimit ? std::min(nearest.size(), (size_t)(offset + nlimit)) : nearest.size();
       for (auto i = offset; i < _limit; i++)
       {
           Neighbour nei;
           nei.endpoint = nearest[i]->m_endpoint;
           nei.node = nearest[i]->m_id;
           out.neighbours.push_back(nei);

       }
       fc::sha256 digest = fc::sha256::hash<UnsignedNeighbours>(out);
       out.signature = std::string(m_sk.sign(digest)); 
       m_socket->send_msg(out,(bi::udp::endpoint)out.tartgetep);
       noteActiveNode(in.fromID, from);
    }
}
bool  NodeTable::isnodevalid(Node const& _node)
{
    if (!_node.m_endpoint)
    {
	elog("do not add node without ep info");
        return false;
    }
    // ping address to recover nodeid if nodeid is empty
    if (_node.m_id == fc::sha256())
    {
	    elog("do not add node without nodeID");
	    return false ;
    }
    if (m_hostNodeID == _node.m_id)
    {
        return false;
    }
    return true;
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, Neighbours const& in ) {
    //ilog("handle nei size ${size} srcnodeid ${nodeid} from ${src} dest ${des} ",("size",in.neighbours.size())("nodeid",in.fromID)("src",in.fromep)("des",in.tartgetep));
    if(in.fromID == fc::sha256())
    {
        ilog("nei msg has no id");
        return ;
    }
    if(in.destid != m_hostNodeID)
    {
        elog("nei msg not for me");
        return ;
    }
    if(in.chain_id != m_chainid)
    {
	    elog("wrong chain");
	    return ;
    }
    for(auto it = m_pknodes.begin(); it != m_pknodes.end();++it)
    {
	    if((it->second.account == in.account) && (it->first != in.fromID))
	    {
		    elog("duplicate nei p2p pk");
		    return ;
	    }
    }
    fc::sha256 digest = fc::sha256::hash<p2p::UnsignedNeighbours>(in);
    bool isvalid = *pktcheckevent(digest,in.pk,chain::signature_type(in.signature),in.account);
    if(!isvalid)
    {
        elog("Neighbours msg check fail:may not in whitelist or not a producer");
        recordBadNode(in.fromID); 
        return ;
    }
    NodeIPEndpoint from;
    from.m_address = _from.address().to_string();
    from.m_udpPort = _from.port();
    from.m_listenPorts = in.fromep.m_listenPorts;
    bool expected = false;
    auto const sentFind = m_sentFindNodes.find(in.fromID);
    if (sentFind != m_sentFindNodes.end())
    {
	if(sentFind->second + fc::microseconds(c_findTimeoutMicroSec) >  fc::time_point::now())
	{
		expected = true;
	}
    }
    if (!expected)
    {
        ilog("Dropping unsolicited neighbours packet from ${add}",("add",_from.address().to_string()));
                return ;
    }
    for (auto const& n : in.neighbours)
    {
	    if(isnodevalid(Node(n.node, n.endpoint)))
	    {
		    if(!m_nodes.count(n.node))
		    {
			    auto nodeEntry = make_shared<NodeEntry>(m_hostNodeID,n.node,n.endpoint);
			    ping(*nodeEntry);
		    }
	    }
    }    
    noteActiveNode(in.fromID, from);
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, PingNode const& pingmsg ) {
ilog("handle ping nodeid ${nodeid} from ${ep} srcep ${src} desep ${des}",("nodeid",pingmsg.sourceid)("ep",_from.address().to_string())("src",pingmsg.source.address())("des",pingmsg.dest.address())); 
if(pingmsg.sourceid == fc::sha256())
    {
        ilog("ping msg has no id");
        return ;
    }
    if(pingmsg.chain_id != m_chainid)
    {
	    elog("wrong chain");
	    return;
    }
    for(auto it = m_pknodes.begin(); it != m_pknodes.end();++it)
    {
	    if((it->second.account == pingmsg.account) && (it->first != pingmsg.sourceid))
	    {
		    elog("duplicate p2p pk");
		    return ;
	    }
    }
    fc::sha256 digest = fc::sha256::hash<p2p::UnsignedPingNode>(pingmsg);
    bool isvalid = *pktcheckevent(digest,pingmsg.pk,chain::signature_type(pingmsg.signature),pingmsg.account);
    if(!isvalid)
    {
	    elog("ping msg check fail:may not in whitelist or not a producer");
        recordBadNode(pingmsg.sourceid); 
        return ;
    }
    if(pingmsg.sourceid != fc::sha256())
    {
        auto const sentdiscoverping = m_pubkDiscoverPings.find(_from.address());
        if (sentdiscoverping != m_pubkDiscoverPings.end())
        {
            m_pubkDiscoverPings.erase(sentdiscoverping);
        }
    }
    if(pingmsg.destid != fc::sha256())
    {
        if(pingmsg.destid != m_hostNodeID)
        {
            elog("ping msg not for me");
            return ;
        }
    }
        
    NodeIPEndpoint from;
    from.m_address = _from.address().to_string();
    from.m_udpPort = _from.port();
    from.m_listenPorts = pingmsg.source.m_listenPorts;
    if (m_hostNodeID == pingmsg.sourceid)
    {
        return;
    }
    addNodePkList(pingmsg.sourceid,pingmsg.pk,pingmsg.account);
    addNode(Node(pingmsg.sourceid, from));

    Pong p;
    p.type = 2;
    p.destep = from;
    p.fromep = m_hostNodeEndpoint;
    p.sourceid = m_hostNodeID;
    p.destid = pingmsg.sourceid; 
    p.chain_id = m_chainid;
    p.pk = m_pk;
    p.account = m_account;
    fc::sha256 digest_pong = fc::sha256::hash<UnsignedPong>(p);
    p.signature = std::string(m_sk.sign(digest_pong));    
    m_socket->send_msg(p,(bi::udp::endpoint)p.destep);
    noteActiveNode(pingmsg.sourceid, from);
}
void NodeTable::recordBadNode(NodeID const& _id)
{
    auto sentPingBad = m_PingsBad.find(_id);
    if(sentPingBad == m_PingsBad.end())
    {
        m_PingsBad[_id] = 1;
    }else
    {
        int badtimes = sentPingBad->second;
        m_PingsBad[_id] = badtimes +1 ;
    }

}
void NodeTable::doDiscovery()
{
    discover_timer->expires_from_now( discoverinterval);
    discover_timer->async_wait( [this](boost::system::error_code _ec) {

        if (_ec)
        {
            elog("Discovery timer was probably cancelled:");
            doDiscovery();
        }
        else
        {
            NodeID randNodeId = fc::sha256();
            fc::rand_pseudo_bytes( randNodeId.data(), randNodeId.data_size());
            doDiscover(randNodeId, 0, make_shared<set<shared_ptr<NodeEntry>>>());
        }
    });
}
void NodeTable::doPingTimeoutCheck()
{
    vector<shared_ptr<NodeEntry>> nodesToActivate;
    for (auto it = m_sentPings.begin(); it != m_sentPings.end();)
    {
        if(it->second.sendtimes > 4)
        {
            if (auto node = nodeEntry(it->first))
            {
		ilog("ping timeout handle ${time} times${times}",("time",it->second.pingSendTime)("times",it->second.sendtimes));
                dropNode(move(node));

                if (it->second.replacementNodeID)
                    if (auto replacement = nodeEntry(*it->second.replacementNodeID))
                        nodesToActivate.emplace_back(replacement);

            }

            it = m_sentPings.erase(it);
        }
        else
            ++it;
    }


    for (auto const& n : nodesToActivate)
        noteActiveNode(n->m_id, n->m_endpoint);
}
void NodeTable::doHandlePingTimeouts()
{
    pingtimeout_timer->expires_from_now( pingtimeoutinterval);
    pingtimeout_timer->async_wait( [this](boost::system::error_code ec) {

        doPingTimeoutCheck();
        doHandlePingTimeouts();
    });
}

void NodeTable::doNodeTimeoutCheck()
{
    for (auto const& s : m_state)
    {
        for (auto const& np : s.nodes)
        {
            if (auto n = np.lock())
            {
                if (!n->hasValidEndpointProof())
                {
                    ping(*n);
                }
            }
        }
    }

}

void NodeTable::doHandleNodeTimeouts()
{
    checknodereachable_timer->expires_from_now( nodetimeoutinterval);
    checknodereachable_timer->async_wait( [this](boost::system::error_code ec) {

        doNodeTimeoutCheck();
        doHandleNodeTimeouts();
    });
}
void NodeTable::doNodeRefindCheck()
{
	requireSeeds(m_seeds);
}
void NodeTable::doNodeReFindTimeouts()
{
    nodesrefindtimer->expires_from_now( nodesrefindinterval);
    nodesrefindtimer->async_wait( [this](boost::system::error_code ec) {
            doNodeRefindCheck();
            doNodeReFindTimeouts();
            });
}
void NodeTable::start_p2p_monitor(ba::io_service& _io)
{
    checknodereachable_timer.reset(new boost::asio::steady_timer(_io));
    doHandleNodeTimeouts();
    pingtimeout_timer.reset(new boost::asio::steady_timer(_io));
    doHandlePingTimeouts();
    idrequest_timer.reset(new boost::asio::steady_timer(_io));
    discover_splittimer.reset(new boost::asio::steady_timer(_io));
    discover_timer.reset(new boost::asio::steady_timer(_io));
    doDiscovery();
    nodesrefindtimer.reset(new boost::asio::steady_timer(_io));
    doNodeReFindTimeouts();
}
}  // namespace p2p
}  // namespace ultrainio
