/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include "p2p/NodeTable.h"
#include "p2p/Common.h"
#include "p2p/UPnP.h"
#include <fc/crypto/rand.hpp>
#include <memory>
#ifndef _WIN32
#include <ifaddrs.h>
#endif
#include <core/utils.h>
#include "kcp/ikcp.h"
#include <appbase/application.hpp>
using namespace std;

namespace ultrainio
{
namespace p2p {
    using boost::asio::ip::tcp;
    using boost::asio::ip::address_v4;
    using boost::asio::ip::host_name;
    using namespace appbase;
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
    NodeTable* NodeTable::s_self = nullptr;
    void NodeTable::initInstance(ba::io_service &_io, NodeIPEndpoint const &_endpoint, NodeID const &nodeID, string const& chainid,string& listenIP,bool traverseNat) {
        if (s_self == NULL) {
            s_self = new NodeTable(_io,_endpoint,nodeID,chainid,listenIP,traverseNat);
        }
    }
   NodeTable* NodeTable::getInstance()
   {
       return s_self;
    }
    NodeTable::NodeTable(ba::io_service &_io, NodeIPEndpoint const &_endpoint, NodeID const &nodeID, string const& chainid,string& listenIP,bool traverseNat) :
            m_hostNodeID(nodeID),
            m_chainid(chainid),
            m_hostNodeEndpoint(_endpoint),
            m_hostPublicEp(_endpoint),
            m_socket_rpos(make_shared<NodeSocket>(_io, *reinterpret_cast<UDPSocketEvents *>(this),bi::udp::endpoint(bi::address::from_string(_endpoint.address()), _endpoint.listenPort(msg_priority_rpos)))),
            m_socket_trx(make_shared<NodeSocket>(_io, *reinterpret_cast<UDPSocketEvents *>(this),bi::udp::endpoint(bi::address::from_string(_endpoint.address()), _endpoint.listenPort(msg_priority_trx)))),
            m_socket(make_shared<NodeSocket>(_io, *reinterpret_cast<UDPSocketEvents *>(this), (bi::udp::endpoint) _endpoint))
    {
        m_listenIP = listenIP;
        m_traverseNat = traverseNat;
        for (unsigned i = 0; i < s_bins; i++)
            m_state[i].distance = i;
        ilog("chain_id ${id}",("id",m_chainid));
        m_hostNodeEndpoint.set_ext_nat_type(nat_type::type_none);
        toPunch.is_set = false;
        toPunch.is_agreed = false;
        toPunch.punch_times = 0;
        toPunch.stage = punchNegoStage::none;
        toPunch._id = fc::sha256();
        toPunch.punch_failure_times = 0;
    }

    NodeTable::~NodeTable() {
        m_socket->disconnect();
        m_socket_rpos->disconnect();
        m_socket_trx->disconnect();
    }
void NodeTable::init( const std::vector <std::string> &seeds,ba::io_service &_io) {
    if(init_flag)
    {
        return ;
    }
    try {
        ilog("init");
        m_socket->connect();
        start_p2p_monitor(_io);
    }
    catch (std::exception const &_e) {
        elog("Exception connecting NodeTable socket:");
    }
    if(m_traverseNat)
    {
        determinePublic();
    }
    m_seeds = seeds;
    doSeedRequest(m_seeds);
    doGetNatType();
    doIDRequestLoop();
    init_flag = true;
}
void NodeTable::doSocketInit()
{
   if(!init_socket_flag)
   {
       m_socket_rpos->connect();
       m_socket_trx->connect();
       init_socket_flag = true;
   }
}
void NodeTable::doGetNatType(){
    send_nat_type_get();
    doNatTypeGetLoop();
}
void NodeTable::doNatTypeGetLoop()
{
    nattypechecktimer->expires_from_now( nattypeinterval);
    nattypechecktimer->async_wait( [this](boost::system::error_code ec){
         ilog("local nattype ${nat}",("nat",m_hostNodeEndpoint.get_ext_nat_type()));
         if(m_hostNodeEndpoint.get_ext_nat_type()  == nat_type::type_none){
            doGetNatType();
        }
    });
}
void NodeTable::doSeedRequest(const std::vector <std::string> &seeds)
{
    for (auto seed : seeds) {
        p2p::NodeIPEndpoint peer;
        peer.setAddress(seed);
        peer.setUdpPort(20124);
        peer.setListenPort(msg_priority_trx,20122);
        peer.setListenPort(msg_priority_rpos,20123);
        p2p::NodeID id = fc::sha256();
        p2p::Node node(id, peer);
        m_pubkDiscoverPings[boost::asio::ip::address::from_string(node.m_endpoint.address())] = fc::time_point::now();
        ping(node.m_endpoint,id,false);
        constructNewPing(node.m_endpoint,id,msg_priority_rpos);
        constructNewPing(node.m_endpoint,id,msg_priority_trx);
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
	    ep.setListenPort(msg_priority_trx,20122);
	    ep.setListenPort(msg_priority_rpos,20123);
            ping(ep,fc::sha256(),false);
            constructNewPing(ep,fc::sha256(),msg_priority_trx);
            constructNewPing(ep,fc::sha256(),msg_priority_rpos);
        }
        doIDRequestLoop();
    }
}
void NodeTable::doIDRequestLoop()
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
void NodeTable::addNode(Node const& _node)
{
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
list<NodeEntry> NodeTable::getNodes()
{
	list<NodeEntry> nodes;
	NodeID randNodeId = fc::sha256();
	fc::rand_pseudo_bytes( randNodeId.data(), randNodeId.data_size());
	auto const nearestNodes = nearestNodeEntries(randNodeId);
	for (auto const& node : nearestNodes)
	{
		nodes.push_back(*node);
	}
	return nodes;
}
Node NodeTable::node(NodeID const& _id)
{
    if (m_nodes.find(_id) != m_nodes.end())
    {
        auto entry = m_nodes[_id];
        return Node(_id, entry->m_endpoint);
    }
    NodeIPEndpoint UnspecifiedNodeIPEndpoint;
    UnspecifiedNodeIPEndpoint.m_address = bi::address().to_string();
    UnspecifiedNodeIPEndpoint.m_udpPort = 0;
    return Node(NodeID(), UnspecifiedNodeIPEndpoint);
}
shared_ptr<NodeEntry> NodeTable::nodeEntry(NodeID _id)
{
    auto const it = m_nodes.find(_id);
    return it != m_nodes.end() ? it->second : shared_ptr<NodeEntry>();
}

void NodeTable::doDiscover(NodeID _node, unsigned _round, shared_ptr<set<shared_ptr<NodeEntry>>> _tried)
{
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
        doDiscoveryLoop();
        return;
    }
    discover_roundtimer->expires_from_now(discoverroundinterval);
    discover_roundtimer->async_wait( [this, _node, _round, _tried](boost::system::error_code _ec) {
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
    static unsigned lastBin = s_bins - 1;
    unsigned head = distance(m_hostNodeID, _target);
    unsigned tail = head == 0 ? lastBin : (head - 1) % s_bins;
    
    map<unsigned, list<shared_ptr<NodeEntry>>> found;
    
    if (head > 1 && tail != lastBin)
        while (head != tail && head < s_bins)
        {
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
            for (auto const& n: m_state[head].nodes)
                if (auto p = n.lock())
                    found[distance(_target, p->m_id)].push_back(p);
            head++;
        }
    else
        while (tail > 0)
        {
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

void NodeTable::ping(NodeIPEndpoint _to,NodeID _toID,bool  need_tcp_connect)
{
    PingNode p;
    p.type = 1;
    p.source = m_hostNodeEndpoint;
    p.dest = _to;
    p.sourceid = m_hostNodeID;
    p.destid = _toID;
    p.chain_id = m_chainid;
    p.pk = m_pk; 
    p.account = m_account;
    if(need_tcp_connect)
    {
        p.to_save.push_back({ext_udp_msg_type::need_tcp_connect,"filled"});
    }
    fc::sha256 digest = fc::sha256::hash<UnsignedPingNode>(p); 
    p.signature = std::string(m_sk.sign(digest));
    m_socket->send_msg(p,(bi::udp::endpoint)_to);
}
void NodeTable::ping(NodeEntry const& _nodeEntry, boost::optional<NodeID> const& _replacementNodeID)
{
    auto const sentPing = m_sentPings.find(_nodeEntry.m_id);
    if (sentPing == m_sentPings.end())
    {
        NodeIPEndpoint from;
        from.setAddress("0.0.0.0");
        from.setUdpPort(0);
        m_sentPings[_nodeEntry.m_id] = {fc::time_point::now(),1, _replacementNodeID,from,_nodeEntry.m_endpoint};
    }
    else
    {
        //just times changes,the first pingsendtime do not change
        sentPing->second.sendtimes+=1;
    }
    ping(_nodeEntry.m_endpoint,_nodeEntry.m_id,false);
    constructNewPing(_nodeEntry.m_endpoint,_nodeEntry.m_id,msg_priority_rpos);
    constructNewPing(_nodeEntry.m_endpoint,_nodeEntry.m_id,msg_priority_trx);
}
void NodeTable::evict(NodeEntry const& _leastSeen, NodeEntry const& _new)
{
    if (!m_socket->isOpen())
        return;

     ilog("evict old ${old} new ${new}",("old",_leastSeen.m_endpoint.address())("new",_new.m_endpoint.address()));
     ping(_leastSeen, _new.m_id);
}
void NodeTable::setNodeNatType(NodeID const& _pubk,string nat_type){
	if (_pubk == m_hostNodeID)
	{
		return ;
	}
	shared_ptr<NodeEntry> newNode = nodeEntry(_pubk);
	if (!newNode)
	{
		return;
	}
	if(newNode->m_endpoint.get_ext_nat_type()!= nat_type::type_none){
		return;
	}else{
		//newNode->m_natType = std::atoi(nat_type.c_str());
		newNode->m_endpoint.set_ext_nat_type(std::atoi(nat_type.c_str()));
	}
	NodeBucket& s = bucket_UNSAFE(newNode.get());
	auto& nodes = s.nodes;
	auto it = std::find(nodes.begin(), nodes.end(), newNode);
	if (it != nodes.end())
	{
		auto nd = it->lock();
		if (!nd)
		{
			elog("invalid weak ptr of node");
			return;
		}
		//    nd->m_natType = std::atoi(nat_type.c_str());
		nd->m_endpoint.set_ext_nat_type(std::atoi(nat_type.c_str()));
		printallbucket();
	}
}
void NodeTable::noteActiveNode(NodeID const& _pubk, NodeIPEndpoint const& _endpoint)
{
    bool buket_chg_flag = false;
    if (_pubk == m_hostNodeID)
    {
        return ;
    }

    shared_ptr<NodeEntry> newNode = nodeEntry(_pubk);
    if (!newNode)
    {
        return;
    }
    shared_ptr<NodeEntry> nodeToEvict;

    NodeBucket& s = bucket_UNSAFE(newNode.get());
    auto& nodes = s.nodes;
    auto it = std::find(nodes.begin(), nodes.end(), newNode);
    if (it != nodes.end())
    {
        auto nd = it->lock();
        if (!nd)
        {
            elog("invalid weak ptr of node");
            return;
        }
        auto old_addr = bi::address::from_string(nd->m_endpoint.m_address);
        auto new_addr = bi::address::from_string(_endpoint.m_address);
        if (isPrivateAddress(old_addr) && !isPrivateAddress(new_addr))
        {
            return;
        }
        else if (!isPrivateAddress(old_addr) && isPrivateAddress(new_addr))
        {
            ilog("old addr is public and new one is private, close old");
            nodedropevent(nd->m_endpoint);
            nodedropevent_kcp(nd->m_endpoint);
            nd->m_endpoint = _endpoint;
            buket_chg_flag = true;
        }

        nodes.splice(nodes.end(), nodes, it);
    }
    else
    {
        newNode->m_endpoint = _endpoint;
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

    if (nodeToEvict)
    {
        evict(*nodeToEvict, *newNode);
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
		nodedropevent_kcp(_n->m_endpoint);
	}
        m_PingsBad.erase(_n->m_id);
	ilog("p2p.nodes.drop id ${id} ep ${ep}",("id",_n->m_id)("ep",_n->m_endpoint.address()));
	printallbucket();
}

NodeTable::NodeBucket& NodeTable::bucket_UNSAFE(NodeEntry const* _n)
{
        return m_state[_n->distance - 1];
}
void NodeTable::printallbucket()
{
    for (auto const& s : m_state)
        for (auto const& np : s.nodes)
        {
            if (auto n = np.lock())
            {
                auto ip = (*n).m_endpoint.address();
ilog("bucket node ${ip} ${udp} ${trx} ${rpos} ${nat_type}",("ip",ip)("udp",(*n).m_endpoint.udpPort())("trx",(*n).m_endpoint.listenPort(msg_priority_trx))("rpos",(*n).m_endpoint.listenPort(msg_priority_rpos))("nat_type",(*n).m_endpoint.get_ext_nat_type()));
            }
        }

}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, Pong const& pong ) {
    if(pong.sourceid == fc::sha256() || pong.sourceid == m_hostNodeID)
    {
        elog("pong msg has no id or sent by myself");
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
    bool isvalid = *pktcheckevent(digest_pong,pong.pk,chain::signature_type(pong.signature),pong.account,false);
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
    #if 1
    if(!m_hostPublicEp){
        auto addr = bi::address::from_string(pong.destep.address());
        if(isPublicAddress(addr)){
            m_hostPublicEp.setAddress(pong.destep.address());
            m_hostPublicEp.setUdpPort(pong.destep.udpPort());            
            ilog("local m_hostPublicEp  ${host}",("host",m_hostPublicEp.address()));
        }
    }
    #endif
    NodeIPEndpoint from;
    from.m_address = _from.address().to_string();
    from.m_udpPort = _from.port();
    from.m_listenPorts = pong.fromep.m_listenPorts;
    noteActiveNode(pong.sourceid, from);
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, FindNode const& in ) {
   // ilog("handle Find srcnodeid ${nodeid} tarID ${tarID} from ${src} dest ${des}",("nodeid",in.fromID)("tarID",in.targetID)("src",in.fromep)("des",in.tartgetep));
    if(in.fromID == fc::sha256() || in.fromID == m_hostNodeID)
    {
        ilog("Find msg has no id or sent by myself");
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
    bool isvalid = *pktcheckevent(digest,in.pk,chain::signature_type(in.signature),in.account,false);
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
bool  NodeTable::isNodeValid(Node const& _node)
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
    if(in.fromID == fc::sha256() || in.fromID == m_hostNodeID)
    {
        ilog("nei msg has no id or sent by myself");
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
    bool isvalid = *pktcheckevent(digest,in.pk,chain::signature_type(in.signature),in.account,false);
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
	    if(isNodeValid(Node(n.node, n.endpoint)))
	    {
			auto nodeEntry = make_shared<NodeEntry>(m_hostNodeID,n.node,n.endpoint);
            ping(*nodeEntry);
            if(!m_sentPings[n.node].from){
                m_sentPings[n.node].from = from;
            }
        }
    }
    noteActiveNode(in.fromID, from);
}

void NodeTable::handlemsg( bi::udp::endpoint const& _from, PingNode const& pingmsg ) {
    if(pingmsg.sourceid == fc::sha256() || pingmsg.sourceid == m_hostNodeID)
    {
        ilog("ping msg has no id or sent by myself");
        return ;
    }

    if(pingmsg.destid != fc::sha256() && pingmsg.destid != m_hostNodeID)
    {
        elog("ping msg not for me. dest: ${dest} host: ${host}", ("dest", pingmsg.destid)("host", m_hostNodeID));
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
    bool isvalid = *pktcheckevent(digest,pingmsg.pk,chain::signature_type(pingmsg.signature),pingmsg.account,false);
    if(!isvalid)
    {
        elog("ping msg check fail:may not in whitelist or not a producer");
        recordBadNode(pingmsg.sourceid); 
        return ;
    }

    m_pubkDiscoverPings.erase(_from.address());
    NodeIPEndpoint from;
    from.m_address = _from.address().to_string();
    from.m_udpPort = _from.port();
    from.m_listenPorts = pingmsg.source.m_listenPorts;
    for(auto& ext : pingmsg.to_save)
    {
	    if(ext.key == ext_udp_msg_type::need_tcp_connect)
	    {
            needtcpevent(from);
        }
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
    if(pingmsg.source.get_ext_nat_type()!= nat_type::type_none){
        setNodeNatType(pingmsg.sourceid,std::to_string(pingmsg.source.get_ext_nat_type()));
    }
    #if 0
    NatTypeRspMsg rspmsg;
    rspmsg.nat_type = "symmetric";
    m_socket->send_msg(rspmsg,(bi::udp::endpoint)p.destep);
    #endif
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
void NodeTable::doDiscoveryLoop()
{
    discover_timer->expires_from_now( discoverinterval);
    discover_timer->async_wait( [this](boost::system::error_code _ec) {

        if (_ec)
        {
            elog("Discovery timer was probably cancelled:");
            doDiscoveryLoop();
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
        //when receive nei-msg,may continue ping a node several times,the condition 'times' may cause unexpected drop
        if(it->second.pingSendTime + fc::microseconds(120000000) < fc::time_point::now())
        {
            if (auto node = nodeEntry(it->first))
            {
                ilog("ping timeout handle ${time} times${times}",("time",it->second.pingSendTime)("times",it->second.sendtimes));
                dropNode(move(node));

                if (it->second.replacementNodeID)
                    if (auto replacement = nodeEntry(*it->second.replacementNodeID))
                        nodesToActivate.emplace_back(replacement);

            }
            else{//no this node  in nodetable
                if(!toPunch.is_set){
                 if(need_nat_punch(it->second.endpoint.address(),it->second.endpoint.get_ext_nat_type())){
                     toPunch.is_set = true;
                     toPunch._id = it->first;
                     ilog("first choose ${nodeid}",("nodeid",toPunch._id));
                     toPunch.is_agreed = false;
                     send_punch_req_sync(it->second.from,m_hostNodeID,it->first,m_account);
                     toPunch.stage =punchNegoStage::sync_sent;
                     doNatPunchNegoLoop();
                 }
                }
            }

            it = m_sentPings.erase(it);
        }
        else
            ++it;
    }


    for (auto const& n : nodesToActivate)
        noteActiveNode(n->m_id, n->m_endpoint);
}
void NodeTable::doPingTimeoutLoop()
{
    pingtimeout_timer->expires_from_now( pingtimeoutinterval);
    pingtimeout_timer->async_wait( [this](boost::system::error_code ec) {

        doPingTimeoutCheck();
        doPingTimeoutLoop();
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

void NodeTable::doNodeTimeoutLoop()
{
    nodetimeout_timer->expires_from_now( nodetimeoutinterval);
    nodetimeout_timer->async_wait( [this](boost::system::error_code ec) {

        doNodeTimeoutCheck();
        doNodeTimeoutLoop();
    });
}
void NodeTable::doSeedKeepaliveCheck()
{
	doSeedRequest(m_seeds);
}
void NodeTable::doSeedKeepaliveLoop()
{
    seedkeepalive_timer->expires_from_now( seedkeepaliveinterval);
    seedkeepalive_timer->async_wait( [this](boost::system::error_code ec) {
            doSeedKeepaliveCheck();
            doSeedKeepaliveLoop();
            });
}
void NodeTable::doPktLimitCheck()
{
    m_socket->reset_pktlimit_monitor();
    m_socket_rpos->reset_pktlimit_monitor();
    m_socket_trx->reset_pktlimit_monitor();
    for (auto& i: m_nat_sockets){
        i.second->ticker_check();
    }
    doNatSocketCheck();
}
int NodeTable::find_peer_priority(string peer){
    for(auto& i:m_sockets_pri){
        if(i.second == peer){
            return i.first;
        }
    }
    return msg_priority::msg_priority_none;
}
void NodeTable::doNatSocketCheck(){
    auto it = m_nat_sockets.begin();
    bool change_flag = false;
    while(it != m_nat_sockets.end()){
        if(!it->second->isOpen()){
            string peer = it->first;
            int peer_pri = find_peer_priority(peer);
            if(peer_pri != msg_priority::msg_priority_none){
                m_sockets_pri.erase(peer_pri);
            }
            it = m_nat_sockets.erase(it);
            change_flag = true;
            ilog("socket size${size1} ${size2}",("size1",m_nat_sockets.size())("size2",m_sockets_pri.size()));
        }
        else{
            ++it;
        }
    }
    if(change_flag){
        if(m_nat_sockets.size()==0){
            doPunchReset();
        }else if(m_nat_sockets.size()==1){
            doNatRePunchLoop();
            toPunch.punch_times = 0;
        }
    }


}
void NodeTable::doPktLimitLoop()
{
    pktlimit_timer->expires_from_now( pktlimitinterval);
    pktlimit_timer->async_wait( [this](boost::system::error_code ec) {
            doPktLimitCheck();
            doPktLimitLoop();
            });
}
void NodeTable::start_p2p_monitor(ba::io_service& _io)
{
    nodetimeout_timer.reset(new boost::asio::steady_timer(_io));
    doNodeTimeoutLoop();
    pingtimeout_timer.reset(new boost::asio::steady_timer(_io));
    doPingTimeoutLoop();
    idrequest_timer.reset(new boost::asio::steady_timer(_io));
    discover_roundtimer.reset(new boost::asio::steady_timer(_io));
    discover_timer.reset(new boost::asio::steady_timer(_io));
    doDiscoveryLoop();
    seedkeepalive_timer.reset(new boost::asio::steady_timer(_io));
    doSeedKeepaliveLoop();
    pktlimit_timer.reset(new boost::asio::steady_timer(_io));
    doPktLimitLoop();
    nattypechecktimer.reset(new boost::asio::steady_timer(_io));
    natPunchNegotimer.reset(new boost::asio::steady_timer(_io));
    natPunchNotifytimer.reset(new boost::asio::steady_timer(_io));
    natRePunchtimer.reset(new boost::asio::steady_timer(_io));
}

void NodeTable::send_request_connect(NodeID nodeID)
{
    std::unordered_map<NodeID, std::shared_ptr<NodeEntry>>::iterator it = m_nodes.find(nodeID);
    if (it != m_nodes.end())
    {
        ping(it->second->m_endpoint, nodeID, true);
    }
    else
    {
        wlog("No node with id: ${id} found, can't send_request_connect.", ("id", nodeID));
    }
}

bool NodeTable::isPrivateAddress(bi::address const& _addressToCheck)
{
    if (_addressToCheck.is_v4())
    {
        bi::address_v4 v4Address = _addressToCheck.to_v4();
        bi::address_v4::bytes_type bytesToCheck = v4Address.to_bytes();
        if (bytesToCheck[0] == 10 || bytesToCheck[0] == 127)
            return true;
        if (bytesToCheck[0] == 169 && bytesToCheck[1] == 254)
            return true;
        if (bytesToCheck[0] == 172 && (bytesToCheck[1] >= 16 && bytesToCheck[1] <= 31))
            return true;
        if (bytesToCheck[0] == 192 && bytesToCheck[1] == 168)
            return true;
    }
    else if (_addressToCheck.is_v6())
    {
        bi::address_v6 v6Address = _addressToCheck.to_v6();
        bi::address_v6::bytes_type bytesToCheck = v6Address.to_bytes();
        if (bytesToCheck[0] == 0xfd && bytesToCheck[1] == 0)
            return true;
        if (!bytesToCheck[0] && !bytesToCheck[1] && !bytesToCheck[2] && !bytesToCheck[3] &&
                !bytesToCheck[4] && !bytesToCheck[5] && !bytesToCheck[6] && !bytesToCheck[7] &&
                !bytesToCheck[8] && !bytesToCheck[9] && !bytesToCheck[10] && !bytesToCheck[11] &&
                !bytesToCheck[12] && !bytesToCheck[13] && !bytesToCheck[14] &&
                (bytesToCheck[15] == 0 || bytesToCheck[15] == 1))
            return true;
    }
    return false;
}

bool NodeTable::isLocalHostAddress(bi::address const& _addressToCheck)
{
    static const set<bi::address> c_rejectAddresses = {
        {bi::address_v4::from_string("127.0.0.1")},
        {bi::address_v4::from_string("0.0.0.0")},
        {bi::address_v6::from_string("::1")},
        {bi::address_v6::from_string("::")},
    };

    return c_rejectAddresses.find(_addressToCheck) != c_rejectAddresses.end();
}
bool NodeTable::isPublicAddress(bi::address const& _addressToCheck)
{
    return !(isPrivateAddress(_addressToCheck) || isLocalHostAddress(_addressToCheck));
}
bi::tcp::endpoint NodeTable::traverseNAT(std::set<bi::address> const& _ifAddresses, unsigned short _listenPort, bi::address& o_upnpInterfaceAddr)
{
    unique_ptr<UPnP> upnp;
    try
    {
        upnp.reset(new UPnP);
    }

    catch (...) {
        elog("upnp init error");
    }

    bi::tcp::endpoint upnpEP;
    if (upnp && upnp->isValid())
    {
        bi::address pAddr;
        int extPort = 0;
        for (auto const& addr: _ifAddresses)
            if (addr.is_v4() && isPrivateAddress(addr) && (extPort = upnp->addRedirect(addr.to_string().c_str(), _listenPort)))
            {
                pAddr = addr;
                break;
            }

        auto eIP = upnp->externalIP();
        bi::address eIPAddr(bi::address::from_string(eIP));
        if (extPort && eIP != string("0.0.0.0") && !isPrivateAddress(eIPAddr))
        {
            ilog("Punched through NAT and mapped local port ${local} ex ${extPort}",("local",_listenPort)("extPort",extPort));
            ilog("External addr: ${ip}",("ip",eIP));
            o_upnpInterfaceAddr = pAddr;
            upnpEP = bi::tcp::endpoint(eIPAddr, (unsigned short)extPort);
        }
        else
            ilog("Couldn't punch through NAT (or no NAT in place).");
    }

    return upnpEP;
}

std::set<bi::address> NodeTable::getInterfaceAddresses()
{
    std::set<bi::address> addresses;

#if defined(_WIN32)
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
    {
        elog("no network");
        return addresses;
    }


    char ac[80] = {0};
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR)
    {
        WSACleanup();
        elog("no network");
    }

    struct hostent* phe = gethostbyname(ac);
    if (phe == 0)
    {
        WSACleanup();
        elog("no network");
        return addresses;
    }

    for (int i = 0; phe->h_addr_list[i] != 0; ++i)
    {
        struct in_addr addr;
        memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
        char *addrStr = inet_ntoa(addr);
        bi::address address(bi::address::from_string(addrStr));
        if (!isLocalHostAddress(address))
            addresses.insert(address.to_v4());
    }

    WSACleanup();
#else
    ifaddrs* ifaddr;
    if (getifaddrs(&ifaddr) == -1)
    {
        elog("no network");
        return addresses;
    }

    for (auto ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr || string(ifa->ifa_name) == "lo0" || !(ifa->ifa_flags & IFF_UP))
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            in_addr addr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            boost::asio::ip::address_v4 address(boost::asio::detail::socket_ops::network_to_host_long(addr.s_addr));
            if (!isLocalHostAddress(address))
                addresses.insert(address);
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            sockaddr_in6* sockaddr = ((struct sockaddr_in6 *)ifa->ifa_addr);
            in6_addr addr = sockaddr->sin6_addr;
            boost::asio::ip::address_v6::bytes_type bytes;
            memcpy(&bytes[0], addr.s6_addr, 16);
            boost::asio::ip::address_v6 address(bytes, sockaddr->sin6_scope_id);
            if (!isLocalHostAddress(address))
                addresses.insert(address);
        }
    }

    if (ifaddr!=NULL)
        freeifaddrs(ifaddr);

#endif

    return addresses;
}
void NodeTable::determinePublic()
{
    auto ifAddresses = getInterfaceAddresses();
    auto laddr = m_listenIP.empty()?
                     bi::address() :
                     bi::make_address(m_listenIP);
    auto lset = !laddr.is_unspecified();
    bool listenIsPublic = lset && isPublicAddress(laddr);
    bool listenIsPrivate = lset && (!listenIsPublic);
    if(m_traverseNat)
    {
        if(listenIsPublic)
        {
            m_hostNodeEndpoint.setAddress(m_listenIP);
        }
        else
        {
            for (auto& p : m_hostNodeEndpoint.m_listenPorts)
            {
                uint16_t curr_port = p.port;
                bi::tcp::endpoint ep(bi::address(), curr_port);
                bi::address natIFAddr;
                ep = traverseNAT(listenIsPrivate ? set<bi::address>({laddr}) : ifAddresses, curr_port, natIFAddr);
                if(0 != ep.port())
                {
                    p.port = ep.port();
                    m_hostNodeEndpoint.setAddress(natIFAddr.to_string());
                }
             //   ilog( "local address by upnp ${addr}",("addr",natIFAddr.to_string()));
            }
            ilog("m_hostep is ${addr} rpos_port ${rpos_port} trx_port ${trx_port}",("addr",m_hostNodeEndpoint.address())("rpos_port",m_hostNodeEndpoint.listenPort(msg_priority_rpos))("trx_port",m_hostNodeEndpoint.listenPort(msg_priority_trx)));
        }
    }
}
void NodeTable::constructNewPing(NodeIPEndpoint _to,NodeID _toID,msg_priority pri)
{
    NewPing p;
    p.type = 5;
    p.source = m_hostNodeEndpoint;
    p.dest = _to;
    p.sourceid = m_hostNodeID;
    p.destid = _toID;
    p.chain_id = m_chainid;
    p.pk = m_pk;
    p.account = m_account;
    p.pri = pri;
    fc::sha256 digest = fc::sha256::hash<UnsignedNewPing>(p);
    p.signature = std::string(m_sk.sign(digest));
    switch(pri)
    {
        case msg_priority_rpos:
            m_socket_rpos->send_msg(p,bi::udp::endpoint(bi::address::from_string(_to.address()),_to.listenPort(msg_priority_rpos)));
            break;
        case msg_priority_trx:
            m_socket_trx->send_msg(p,bi::udp::endpoint(bi::address::from_string(_to.address()),_to.listenPort(msg_priority_trx)));
            break;
	default :
	    break;
    }

}
void NodeTable::updateListenPort(NodeID const& _pubk, uint16_t port ,msg_priority pri)
{
    bool buket_chg_flag = false;
    if (_pubk == m_hostNodeID)
    {
        return ;
    }

    shared_ptr<NodeEntry> newNode = nodeEntry(_pubk);
    if (!newNode)
    {
        return;
    }
    NodeBucket& s = bucket_UNSAFE(newNode.get());
    auto& nodes = s.nodes;
    auto it = std::find(nodes.begin(), nodes.end(), newNode);
    if (it != nodes.end())
    {
        auto nd = it->lock();
        if (!nd)
        {
            elog("invalid weak ptr of node");
            return;
        }
        if(nd->m_endpoint.listenPort(pri) != port)
        {
            nd->m_endpoint.setListenPort(pri,port);
            buket_chg_flag = true;
        }
    }
    if(buket_chg_flag)
    {
        printallbucket();
    }
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, NewPing const& pingmsg ) {
    if(pingmsg.sourceid == fc::sha256() || pingmsg.sourceid == m_hostNodeID)
    {
        ilog("ping msg has no id or sent by myself");
        return ;
    }

    if(pingmsg.destid != fc::sha256() && pingmsg.destid != m_hostNodeID)
    {
        elog("ping msg not for me. dest: ${dest} host: ${host}", ("dest", pingmsg.destid.str().substr(0,7))("host", m_hostNodeID.str().substr(0,7)));
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
    fc::sha256 digest = fc::sha256::hash<p2p::UnsignedNewPing>(pingmsg);
    bool isvalid = *pktcheckevent(digest,pingmsg.pk,chain::signature_type(pingmsg.signature),pingmsg.account,false);
    if(!isvalid)
    {
        elog("newping msg check fail:may not in whitelist or not a producer");
        recordBadNode(pingmsg.sourceid);
        return ;
    }

    updateListenPort(pingmsg.sourceid,_from.port(),pingmsg.pri);
}
void NodeTable::do_send_connect_packet(string peer,msg_priority pri,uint16_t port)
{
    ConnectMsg msg;
    msg.type = 6;
    msg.peer = peer + ":" + to_string(port);
    msg.pri = pri;
    msg.sourceid = m_hostNodeID;
    msg.chain_id = m_chainid;
    msg.pk = m_pk;
    msg.account = m_account;
    fc::sha256 digest = fc::sha256::hash<UnsignedConnectMsg>(msg);
    msg.signature = std::string(m_sk.sign(digest));
    string peer_info = peer+":"+to_string(port);
    if(m_nat_sockets.count(peer_info)){
        auto socket = m_nat_sockets[peer_info];
        ilog("punch socket");
        socket->send_msg(msg,bi::udp::endpoint(bi::address::from_string(peer),port));
        return ;
    }
    switch(pri)
    {
        case msg_priority_rpos:
            m_socket_rpos->send_msg(msg,bi::udp::endpoint(bi::address::from_string(peer),port));
            break;
        case msg_priority_trx:
            m_socket_trx->send_msg(msg,bi::udp::endpoint(bi::address::from_string(peer),port));
            break;
        default :
            break;
    }
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, ConnectMsg const&  msg ) {
    ilog("handle connectmsg nodeid ${nodeid}",("nodeid", msg.sourceid.str().substr(0,7)));
    if( msg.sourceid == fc::sha256() ||  msg.sourceid == m_hostNodeID)
    {
        ilog("connect msg has no id or sent by myself");
        return ;
    }

    if( msg.chain_id != m_chainid)
    {
        elog("wrong chain");
        return;
    }

    for(auto it = m_pknodes.begin(); it != m_pknodes.end();++it)
    {
        if((it->second.account ==  msg.account) && (it->first !=  msg.sourceid))
        {
            elog("connnect msg duplicate p2p pk");
            return ;
        }
    }
    fc::sha256 digest = fc::sha256::hash<p2p::UnsignedConnectMsg>( msg);
    bool isvalid = *pktcheckevent(digest, msg.pk,chain::signature_type( msg.signature), msg.account,true);
    if(!isvalid)
    {
        elog("connect msg check fail:may not in whitelist or not a producer");
        recordBadNode( msg.sourceid);
        return ;
    }
    ConnectAckMsg ackmsg;
    ackmsg.type = 7;
    ackmsg.peer = msg.peer;
    ackmsg.pri = msg.pri;
    ackmsg.sourceid = m_hostNodeID;
    ackmsg.chain_id = m_chainid;
    ackmsg.pk = m_pk;
    ackmsg.account = m_account;
    ackmsg.conv = get_new_conv();
    fc::sha256 hash = fc::sha256::hash<UnsignedConnectAckMsg>(ackmsg);
    ackmsg.signature = std::string(m_sk.sign(hash));
    string peer_info = _from.address().to_string()+":"+to_string(_from.port());
    if(m_nat_sockets.count(peer_info)){
        auto socket = m_nat_sockets[peer_info];
        ilog("punch socket");
        socket->send_msg(ackmsg,bi::udp::endpoint(_from.address(),_from.port()));
        kcpconnectevent(ackmsg.conv,_from.address().to_string()+":"+std::to_string(_from.port()),msg.pri);
        return ;
    }
    switch(msg.pri)
    {
	    case msg_priority_rpos:
		    m_socket_rpos->send_msg(ackmsg,bi::udp::endpoint(_from.address(),_from.port()));
		    break;
	    case msg_priority_trx:
		    m_socket_trx->send_msg(ackmsg,bi::udp::endpoint(_from.address(),_from.port()));
		    break;
	    default :
		    break;
    }
    kcpconnectevent(ackmsg.conv,_from.address().to_string()+":"+std::to_string(_from.port()),msg.pri);
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, ConnectAckMsg const& msg)
{
    ilog("handle connectAckmsg nodeid ${nodeid}",("nodeid", msg.sourceid.str().substr(0,7)));
    if( msg.sourceid == fc::sha256() ||  msg.sourceid == m_hostNodeID)
    {
        ilog("connectAck msg has no id or sent by myself");
        return ;
    }

    if( msg.chain_id != m_chainid)
    {
        elog("wrong chain");
        return;
    }

    for(auto it = m_pknodes.begin(); it != m_pknodes.end();++it)
    {
        if((it->second.account ==  msg.account) && (it->first !=  msg.sourceid))
        {
            elog("connectAck msg duplicate p2p pk");
            return ;
        }
    }
    fc::sha256 digest = fc::sha256::hash<p2p::UnsignedConnectAckMsg>( msg);
    bool isvalid = *pktcheckevent(digest, msg.pk,chain::signature_type( msg.signature), msg.account,true);
    if(!isvalid)
    {   
        elog("connectAck msg check fail:may not in whitelist or not a producer");
        recordBadNode( msg.sourceid);
        return ;
    }
    kcpconnectackevent(msg.conv,msg.peer);
}
void NodeTable::sendSessionCloseMsg(bi::udp::endpoint const& _to,kcp_conv_t conv,msg_priority pri,bool todel)
{
    SessionCloseMsg msg;
    msg.type = 8;
    msg.sourceid = m_hostNodeID;
    msg.chain_id = m_chainid;
    msg.pk = m_pk;
    msg.account = m_account;
    msg.conv = conv;
    msg.todel = todel;
    fc::sha256 hash = fc::sha256::hash<UnsignedSessionCloseMsg>(msg);
    msg.signature = std::string(m_sk.sign(hash));
    ilog("send sessonclose ${to} conv ${conv}",("to",_to.address().to_string())("conv",conv));
    string peer_info = _to.address().to_string()+":"+to_string(_to.port());
    if(m_nat_sockets.count(peer_info)){
        auto socket = m_nat_sockets[peer_info];
        ilog("punch socket");
        socket->send_msg(msg,_to);
        return ;
    }

    switch(pri)
    {
            case msg_priority_rpos:
                    m_socket_rpos->send_msg(msg,_to);
                    break;
            case msg_priority_trx:
                    m_socket_trx->send_msg(msg,_to);
                    break;
            default :
                    break;
    }
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, SessionCloseMsg const& msg)
{
    ilog("handle SessionCloseMsg nodeid ${nodeid} conv ${conv}",("nodeid", _from.address().to_string())("conv",msg.conv));
    if( msg.sourceid == fc::sha256() ||  msg.sourceid == m_hostNodeID)
    {
	    ilog("SessionCloseMsg msg has no id or sent by myself");
	    return ;
    }

    if( msg.chain_id != m_chainid)
    {
	    elog("wrong chain");
	    return;
    }

    for(auto it = m_pknodes.begin(); it != m_pknodes.end();++it)
    {
	    if((it->second.account ==  msg.account) && (it->first !=  msg.sourceid))
	    {
		    elog("SessionCloseMsg msg duplicate p2p pk");
		    return ;
	    }
    }
    fc::sha256 digest = fc::sha256::hash<p2p::UnsignedSessionCloseMsg>( msg);
    bool isvalid = *pktcheckevent(digest, msg.pk,chain::signature_type( msg.signature), msg.account,true);
    if(!isvalid)
    {
        elog("SessionCloseMsg msg check fail:may not in whitelist or not a producer");
        recordBadNode( msg.sourceid);
        return ;
    }
 
    sessioncloseevent(msg.conv,msg.todel);
}
void NodeTable::doNatPunchNegoTimeout(){
    if(!toPunch.is_agreed && !toBePunchList.empty()){
        ilog("no response but has ask");
        auto& latest_msg = toBePunchList.back();
        toPunch.is_set = true;
        toPunch._id = latest_msg.msg.reqsrc;
        toPunch.name = latest_msg.msg.reqsrc_name;
        send_punch_req_ack(latest_msg.msg_src,m_hostNodeID,latest_msg.msg.reqsrc,m_account); 
        toPunch.stage  = punchNegoStage::sync_sent;
        toBePunchList.clear();
        doNatPunchNegoLoop();
    }else{
        if(!toPunch.is_agreed){
            ilog("no response no  ask");
            toPunch.is_set = false;
        }
    }
}
void NodeTable::doNatPunchNegoLoop(){
    boost::asio::steady_timer::duration interval;
    int int_interval = 10+rand()%9;
    ilog("interval ${int}",("int",int_interval));
    interval = {std::chrono::seconds{int_interval}};
    natPunchNegotimer->expires_from_now( interval);
    natPunchNegotimer->async_wait( [this](boost::system::error_code ec){
        doNatPunchNegoTimeout();
    });
}
void NodeTable::send_punch_req_sync(NodeIPEndpoint const& _to,NodeID reqsrc,NodeID reqdes,chain::account_name reqsrc_name){
	NatPunchReqSyncMsg msg;
	msg.sendsrc = m_hostNodeID;
	msg.reqsrc = reqsrc;
	msg.reqdst = reqdes;
    msg.reqsrc_name = reqsrc_name;
	m_socket->send_msg(msg,(bi::udp::endpoint)_to);
}
void NodeTable::send_punch_req_ack(NodeIPEndpoint const& _to,NodeID reqsrc,NodeID reqdes,chain::account_name reqsrc_name){
    NatPunchReqAckMsg msg;
    msg.sendsrc = m_hostNodeID;
    msg.reqsrc = reqsrc;
    msg.reqdst = reqdes;
    msg.reqsrc_name = reqsrc_name;
    m_socket->send_msg(msg,(bi::udp::endpoint)_to);
}
void NodeTable::send_punch_rsp_ack(NodeIPEndpoint const& _to,NodeID reqsrc,NodeID reqdes){
     NatPunchRspAckMsg msg;
     msg.sendsrc = m_hostNodeID;
     msg.reqsrc = reqsrc;
     msg.reqdst = reqdes;
     m_socket->send_msg(msg,(bi::udp::endpoint)_to);
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, NatTypeReqMsg const& msg){
}
#if 1
void NodeTable::handlemsg( bi::udp::endpoint const& _from, NatPunchReqSyncMsg const& msg ){
    if(msg.reqdst != m_hostNodeID){
        shared_ptr<NodeEntry> newNode = nodeEntry(msg.reqdst);
        if(!newNode){
                return ;
        }else{
        ilog("NatPunchReqSyncMsg i am bridge");
            send_punch_req_sync(newNode->endPoint(),msg.reqsrc,msg.reqdst,msg.reqsrc_name);
        }
    }else{
        shared_ptr<NodeEntry> newNode = nodeEntry(msg.reqdst);
        if(newNode){
            elog("you ${id} is my neighbour,no need to punch",("id",msg.reqdst));
            if(toPunch.is_set && toPunch._id == msg.reqsrc){
               doPunchReset();
            }
            return ;
        }
    //to me
        NodeIPEndpoint from;
        from.m_address = _from.address().to_string();
        from.m_udpPort = _from.port();
        if(!toPunch.is_set){//me is avalible
        ilog("for me  i can NatPunchReqSyncMsg");
            send_punch_req_ack(from,m_hostNodeID,msg.reqsrc,m_account);
            toPunch.is_set = true;
            toPunch._id = msg.reqsrc;
            toPunch.stage =punchNegoStage::sync_sent;
            toPunch.name = msg.reqsrc_name;
        }else{
            if(!toPunch.is_agreed){
        	    ilog("i am busy wait NatPunchReqSyncMsg");
                toBePunchFeature element;
                element.msg_src = from;
                element.msg = msg;
                toBePunchList.emplace_back(element);
            }
        }
    }
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, NatPunchReqAckMsg const& msg ){
    if(msg.reqdst != m_hostNodeID){
        shared_ptr<NodeEntry> newNode = nodeEntry(msg.reqdst);
        if(!newNode){
            return ;
        }else{
            ilog("NatPunchReqAckMsg i am bridge");
           send_punch_req_ack(newNode->endPoint(),msg.reqsrc,msg.reqdst,msg.reqsrc_name); 
        }
    }else{
        shared_ptr<NodeEntry> newNode = nodeEntry(msg.reqdst);
        if(newNode){
            elog("you ${id} is my neighbour,no need to punch",("id",msg.reqdst));
            if(toPunch.is_set && toPunch._id == msg.reqsrc){
               doPunchReset();
            }
            return ;
        }
        if(toPunch.is_set && toPunch._id == msg.reqsrc && toPunch.stage==punchNegoStage::sync_sent){
             ilog("for me okey NatPunchReqAckMsg");
             NodeIPEndpoint from;
             from.m_address = _from.address().to_string();
             from.m_udpPort = _from.port();
             send_punch_rsp_ack(from,m_hostNodeID,msg.reqsrc);
             toPunch.is_agreed = true;
             toPunch.name = msg.reqsrc_name;
             toPunch.stage = punchNegoStage::establish;
             toBePunchList.clear();//be asked is useless;
             send_nat_punch_notify();
         }
    }
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, NatPunchRspAckMsg const& msg ){
    if(msg.reqdst != m_hostNodeID){
        shared_ptr<NodeEntry> newNode = nodeEntry(msg.reqdst);
        if(!newNode){
            return ;
        }else{
            ilog("i am bridge");
            send_punch_rsp_ack(newNode->endPoint(),msg.reqsrc,msg.reqdst);
        }

    }else{
        shared_ptr<NodeEntry> newNode = nodeEntry(msg.reqdst);
        if(newNode){
            elog("you ${id} is my neighbour,no need to punch",("id",msg.reqdst));
            if(toPunch.is_set && toPunch._id == msg.reqsrc){
               doPunchReset();
            }
            return ;
        }
        if(toPunch.is_set && toPunch._id == msg.reqsrc && toPunch.stage==punchNegoStage::sync_sent){
            ilog("for me i can NatPunchRspAckMsg");
            toPunch.is_agreed = true;
            toPunch.stage = punchNegoStage::establish;
            toBePunchList.clear();//be asked is useless;
            send_nat_punch_notify();
        }
    }
}
void NodeTable::doNatRePunchLoop(){
    natRePunchtimer->expires_from_now( natRePunchInterval);
    natRePunchtimer->async_wait( [this](boost::system::error_code ec){
        if(ec.value() == boost::asio::error::operation_aborted){
            ilog(" doNatRePunchLoop ${ec}",("ec",ec.message()));
        }else{
        send_nat_punch_notify();
        }
    });
}


void NodeTable::doNatPunchNotifyLoop(){
    natPunchNotifytimer->expires_from_now( natPunchNotifyInterval);
    natPunchNotifytimer->async_wait( [this](boost::system::error_code ec){
        if(ec.value() == boost::asio::error::operation_aborted){
            ilog(" doNatPunchNotifyLoop ${ec}",("ec",ec.message()));
        }else{
        if(!punch_ans_recved){
            elog("no response from punch app");
            if(toPunch.punch_times<4){
                toPunch.punch_times++;
                send_nat_punch_notify();
            }
            else{
               //no response is actually no exsit,so just Repunch
               doNatRePunchLoop();
               toPunch.punch_times = 0;
            }
        }
        }

    });
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, NatTypeRspMsg const& msg){
	ilog("handle NAt");
    ilog("nattype ${nat}",("nat",msg.nat_type));
    string nat_type = msg.nat_type;
    uint32_t nat_type_int = nat_type::type_none;
    if(nat_type == "full_cone"){
        nat_type_int = nat_type::full_cone;
    }else if(nat_type == "restrict_ip_cone"){
        nat_type_int = nat_type::ip_restrict_cone;
    }else if(nat_type == "restrict_port_cone"){
        nat_type_int = nat_type::port_restrict_cone;
    }else if(nat_type == "symmetric"){
        nat_type_int = nat_type::symmetric;
    }else{
        nat_type_int = nat_type::type_none;
    } 
    m_hostNodeEndpoint.set_ext_nat_type(nat_type_int);
}
void NodeTable::handlemsg( bi::udp::endpoint const& _from, NatPunchNotifyMsg const& msg){
    //string nat_info = "20000,10.0.0.216:20000";
    try{
        msg_priority pri_need = msg_priority_none;
        string nat_info = msg.peer_info;
        ilog("NatPunchNotifyMsg ${msg}",("msg",nat_info));
        auto colon = nat_info.find(",");
        if (colon == std::string::npos || colon == 0) {
            elog("invalid");
            return ;
        }
        if(punch_ans_recved){
            elog("no ask but answer,is duplicate msg");
            return ;
        }
        if(!toPunch.is_set || !toPunch.is_agreed){
            elog("not for this round.maybe expired");
            return ;
        }
        punch_ans_recved = true;
        auto local_port = nat_info.substr(0,colon);
        if(std::stoi(local_port) == 0){
            elog("can not punch");
            toPunch.punch_failure_times++;
            if(m_nat_sockets.size()<2){//just need 2
                if(m_nat_sockets.size()==0 && toPunch.punch_failure_times > 5)
                {// no enough socket && failure in this one several times change to another
                     ilog("change source");
                     doPunchReset();
                }else{
                    doNatRePunchLoop();
                    toPunch.punch_times = 0;
                }
            }
            return ;
        }
        toPunch.punch_failure_times = 0;// clear punch failure log
        auto peer_info = nat_info.substr(colon + 1);
        if(m_nat_sockets.find(peer_info) != m_nat_sockets.end()){
            elog("duplicate msg");
            if(m_nat_sockets.size() == 1){
                ilog("need another punch");
                doNatRePunchLoop();
                toPunch.punch_times = 0;
            }

            return ;
        }
        auto it = m_sockets_pri.find(msg_priority_rpos);
        if(it == m_sockets_pri.end()){
            pri_need = msg_priority_rpos;
        }else{
            if(m_sockets_pri.find(msg_priority_trx) == m_sockets_pri.end()){
                 pri_need = msg_priority_trx;
            }
        }
        if(pri_need == msg_priority_none){
            return ;//just need 2
        }
        auto newSocket = make_shared<NodeSocket>(std::ref( appbase::app().get_io_service() ), *reinterpret_cast<UDPSocketEvents *>(this),bi::udp::endpoint(bi::address::from_string(m_hostNodeEndpoint.address()),std::stoi(local_port)));
        m_nat_sockets[peer_info] = newSocket;
        newSocket->connect();
        natPunchConnEvent(peer_info,pri_need);
        m_sockets_pri[pri_need] = peer_info;
    }
    catch(  const fc::exception& e ) {
        edump((e.to_detail_string() ));
    }
    catch(const std::exception& e)
    {
        elog( "error: ${e}", ("e",e.what()));
    }
    catch(...)
    {
        elog("handlemsg error");
    }
    if(m_nat_sockets.size() == 1){
        ilog("need another punch");
        doNatRePunchLoop();
        toPunch.punch_times = 0;
    }
}

#endif
void NodeTable::doPunchReset(){
     toPunch.is_set = false;
     toPunch.is_agreed = false;
     toPunch.punch_times = 0;
     toPunch.stage = punchNegoStage::none;
     toPunch._id = fc::sha256();
     toPunch.punch_failure_times = 0;
     natPunchNotifytimer->cancel();
     natRePunchtimer->cancel();
}
void NodeTable::handlekcpmsg(const char *data,size_t bytes_recvd)
{
    kcp_conv_t conv;
    int ret = ikcp_get_conv(data, bytes_recvd, &conv);
    if (ret == 0)
    {
        ilog("ikcp_get_conv return 0");
        return;
    }
    kcppktrcvevent(conv,data,bytes_recvd);
}
kcp_conv_t NodeTable::get_new_conv(void) const
{
    static uint32_t static_cur_conv = hash64(m_hostNodeID.str().substr(0,7).c_str(),7);
    static_cur_conv++;
    return static_cur_conv;
}
void NodeTable::send_kcp_package(const char *buf, int len,bi::udp::endpoint to,msg_priority pri)
{
    string peer_info = to.address().to_string()+":"+to_string(to.port());
    if(m_nat_sockets.count(peer_info)){
        auto socket = m_nat_sockets[peer_info];
        socket->send_kcp_packet(buf, len,to);
        return ;
    }

    
    switch(pri)
    {
            case msg_priority_rpos:
                    m_socket_rpos->send_kcp_packet(buf, len,to);
                    break;
            case msg_priority_trx:
                    m_socket_trx->send_kcp_packet(buf, len,to);
                    break;
            default :
                    break;
    }
}
void NodeTable::send_nat_punch_notify(){
    int nat_socket_size = m_nat_sockets.size();
    if(nat_socket_size>=2 || !toPunch.is_set){
        return ;
    }
    string msg = "{\"msg_type\":\"set_host_id\",\"msg_id\":\""+to_string(nat_socket_size+1)+"\",\"host_id\":\""+m_account.to_string()+"\",\"target_host_id\":\""+toPunch.name.to_string()+"\"}";
    ilog("msg ${msg}",("msg",msg));
    send_nat_package(msg.c_str(),msg.size());
    punch_ans_recved = false;
    doNatPunchNotifyLoop();
}
void NodeTable::send_nat_type_get(){
    string msg = "{\"msg_type\":\"ask_nat_type\"}";
    ilog("msg ${msg}",("msg",msg));
    send_nat_package(msg.c_str(),msg.size());
}
void NodeTable::send_nat_package(const char *buf, int len){
    m_socket->send_kcp_packet(buf,len,bi::udp::endpoint(bi::address::from_string("0.0.0.0"), 6010));
}
bool NodeTable::need_nat_punch(string ip,uint32_t nat_type){
    uint32_t local_nat_type =0;
    local_nat_type = m_hostNodeEndpoint.get_ext_nat_type();
    if(local_nat_type == nat_type::type_none){
        return false;
    }
    if((local_nat_type > nat_type::ip_restrict_cone && nat_type == symmetric)
            || (local_nat_type == symmetric && nat_type > nat_type::ip_restrict_cone)){
       auto addr = bi::address::from_string(ip);
       auto local_public = bi::address::from_string(m_hostPublicEp.address());
       if(isPublicAddress(addr) && isPublicAddress(local_public) && ip == m_hostPublicEp.address()){
	       return false;//most router can not support loopback
       }else{
           return true;
       }
    }
    return false;
}
}  // namespace p2p
}  // namespace ultrainio
