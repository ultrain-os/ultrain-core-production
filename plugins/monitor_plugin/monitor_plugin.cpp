/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/monitor_plugin/monitor_plugin.hpp>
#include <fc/exception/exception.hpp>
#include <ultrainio/chain/exceptions.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <fc/variant.hpp>
#include <fc/io/json.hpp>
#include <thread>
#include "httpc_only_send.hpp"

using namespace ultrainio::client::http;

namespace ultrainio {
   static appbase::abstract_plugin& _monitor_plugin = app().register_plugin<monitor_plugin>();

   using boost::asio::ip::tcp;


ultrainio::client::http::http_context context;

template<typename T>
fc::variant call( const std::string& url,
                  const std::string& path,
                  const T& v ) {
   try {
       vector<string> headers; //pass an empty header vector
       auto sp = std::make_unique<ultrainio::client::http::connection_param>(context, parse_url(url) + path,  false, headers);
       return ultrainio::client::http::do_http_call( *sp, fc::variant(v), false, false );
   }
   catch(client::http::connection_exception& e) {
       std::cerr << e.to_detail_string() << std::endl;
       return fc::json::from_string("{\"exception\":\"Connection refused\"}");
   }
   catch(boost::system::system_error& e) {
       std::cerr << e.what() << std::endl;
       return fc::json::from_string("{\"exception\":\" system error\"}");
   }
   catch (...) {
       std::cerr << "Exception happen." << std::endl;
       return fc::json::from_string("{\"exception\":\" unknown\"}");
   }
}


class monitor_plugin_impl {
  public:

    monitor_plugin_impl();
    ~monitor_plugin_impl() = default;

    void shutdown();
    void startMonitorTaskTimer();

    void sendStaticConfigInfo();

    tcp::endpoint   self_endpoint; // same as p2p-listen-endpoint in net plugin
    std::string     monitor_central_server;
    std::string     call_path_dynamic;
    std::string     call_path_static;
    bool            needReportTask = true;
    uint32_t        reportInterval;
    vector<string>  supplied_peers;

    monitor_apis::monitor_only m_monitorHandler;

  private:
    void processReportTask();

    bool firstLoop;
    std::unique_ptr<boost::asio::steady_timer> m_reportTaskTimer;
};

monitor_plugin_impl::monitor_plugin_impl() : firstLoop(true) {
    m_reportTaskTimer.reset(new boost::asio::steady_timer(app().get_io_service()));
}

void monitor_plugin_impl::shutdown() {
    if(m_reportTaskTimer) {
        m_reportTaskTimer->cancel();
    }
}

void monitor_plugin_impl::startMonitorTaskTimer() {
    if(!needReportTask)
        return;

    boost::asio::steady_timer::duration reportTaskPeriod = std::chrono::seconds(reportInterval);
    m_reportTaskTimer->expires_from_now(reportTaskPeriod);
    m_reportTaskTimer->async_wait([this](boost::system::error_code ec) {
        if (ec.value() == boost::asio::error::operation_aborted) {
            ilog("report task timer be canceled.");
        } else {
            //auto start_timestamp = fc::time_point::now();
            processReportTask();
            startMonitorTaskTimer();
            //ilog("report task taking time ${time}", ("time", fc::time_point::now() - start_timestamp));
        }
    });
}

void monitor_plugin_impl::processReportTask() {
  try{
    if (firstLoop) {
      sendStaticConfigInfo();
      firstLoop = false;
    }

    std::shared_ptr<periodic_report_dynamic_data> dynamicData = std::make_shared<periodic_report_dynamic_data>();
    m_monitorHandler.getDynamicNodeData(*dynamicData);
    auto foo = [dynamicData, this](){
                   this->m_monitorHandler.getDynamicOsData(*dynamicData);
                   dynamicData->nodeIp = this->self_endpoint.address().to_v4().to_string();
                   call(this->monitor_central_server, this->call_path_dynamic, *dynamicData);
               };
    std::thread thrd(foo);
    thrd.detach();
  }
  catch(chain::node_not_found_exception& e) {
    auto exceptionInfo = std::string("exception happened, node not initialized.");
    ilog("Periodic report: node not initialized.");
    //call(monitor_central_server, call_path_dynamic, exceptionInfo);
  }
  catch(fc::key_not_found_exception& e) {
  }
  catch(fc::bad_cast_exception& e) {
  }
  catch(...) {
    ilog("Periodic report: unknown exception.");
  } //don't allow exception be thrown out, to prevent Ultrainode from exiting.
}

void monitor_plugin_impl::sendStaticConfigInfo() {
  auto sendStatic = [this]() {
      periodic_report_static_data configInfo = this->m_monitorHandler.getStaticConfigInfo();
      configInfo.nodeIp = this->self_endpoint.address().to_v4().to_string();
      configInfo.configuredPeers = this->supplied_peers;
      call(this->monitor_central_server, this->call_path_static, configInfo);
  };
  std::thread thrd(sendStatic);
  thrd.detach();
}

monitor_plugin::monitor_plugin():my(new monitor_plugin_impl()){}
monitor_plugin::~monitor_plugin() = default;

void monitor_plugin::set_program_options(options_description&, options_description& cfg) {
    cfg.add_options()
         ( "monitor-server-endpoint", bpo::value<string>()->default_value("http://127.0.0.1:8078"),
           "The actual host:port used to monitor central server")
         ( "periodic-report", bpo::value<bool>()->default_value(true),
           "True to enable the periodic report to central server.")
         ( "report-interval", bpo::value<int>()->default_value(12),
           "The interval time (seconds) of the periodic report to monitor central server");
}

void monitor_plugin::plugin_initialize(const variables_map& options) {
    try{
        if( options.count( "monitor-server-endpoint" )) {
            my->monitor_central_server = options.at( "monitor-server-endpoint" ).as<string>();
        }
        if( options.count( "p2p-peer-address" )) {
            my->supplied_peers = options.at( "p2p-peer-address" ).as<vector<string> >();
         }

        auto resolver = std::make_shared<tcp::resolver>( std::ref( app().get_io_service()));

        tcp::resolver::query query(tcp::v4(), boost::asio::ip::host_name(), "");
        tcp::resolver::iterator iter = resolver->resolve(query);
        tcp::resolver::iterator end; // End marker.
        if (iter != end)
        {
            my->self_endpoint = *iter;
        }

        my->needReportTask = options.at( "periodic-report" ).as<bool>();
        my->reportInterval = options.at( "report-interval" ).as<int>();

    }FC_LOG_AND_RETHROW()

    my->call_path_dynamic = "/status/info";
    my->call_path_static  = "/status/staticInfo";

   context = ultrainio::client::http::create_http_context();
}

void monitor_plugin::plugin_startup() {
   // Make the magic happen
   my->startMonitorTaskTimer();
}

void monitor_plugin::plugin_shutdown() {
   // OK, that's enough magic
   my->shutdown();
}

monitor_apis::monitor_only  monitor_plugin::get_monitor_only_api()const {
    return my->m_monitorHandler;
}

   namespace monitor_apis {

     const std::shared_ptr<Node>  monitor_only::getNodePtr() const {
       auto nodePtr = Node::getInstance();
       ULTRAIN_ASSERT( nodePtr != nullptr, chain::node_not_found_exception, "Failed to get Node. Most possible reason: This Node is not a producer." );
       return nodePtr;
     }

     monitor_only::monitor_node_result monitor_only::monitor_node( const monitor_only::monitor_node_params& params )const {
        auto nodePtr = getNodePtr();
        UranusNodeMonitor nodeMonitor(nodePtr);
        return nodeMonitor.getNodeInfo();
     }

     monitor_only::monitor_propose_msg_result monitor_only::monitor_propose_msg( const monitor_only::monitor_propose_msg_params& params) const {
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getScheduler());
        return controllerMonitor.findProposeMsgByBlockId(chain::block_id_type(params.block_id));
     }

     monitor_only::monitor_echo_msg_result monitor_only::monitor_echo_msg( const monitor_only::monitor_echo_msg_params& params) const{
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getScheduler());
        return controllerMonitor.findEchoMsgByBlockId(chain::block_id_type(params.block_id));
     }

     monitor_only::monitor_propose_cache_result monitor_only::monitor_propose_cache(const monitor_only::monitor_propose_cache_params& params) const{
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getScheduler());
        return {controllerMonitor.findProposeCacheByKey(params)};
     }

     monitor_only::monitor_echo_cache_result monitor_only::monitor_echo_cache(const monitor_only::monitor_echo_cache_params& params) const {
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getScheduler());
        return {controllerMonitor.findEchoCacheByKey(params)};
     }

     monitor_only::monitor_echo_ap_cache_result monitor_only::monitor_echo_ap_cache(const monitor_only::monitor_echo_ap_cache_params& params) const {
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getScheduler());
        return {controllerMonitor.findEchoApMsgByKey(params)};
     }

     void monitor_only::getDynamicNodeData(periodic_report_dynamic_data& reportData) {
        if(nullptr == m_nodeMonitor) {
            auto nodePtr = getNodePtr();
            m_nodeMonitor = std::make_shared<UranusNodeMonitor>(nodePtr);
        }
        m_nodeMonitor->getNodeData(reportData);
     }

     void monitor_only::getDynamicOsData(periodic_report_dynamic_data& reportData) {
        if(nullptr == m_nodeMonitor) {
            auto nodePtr = getNodePtr();
            m_nodeMonitor = std::make_shared<UranusNodeMonitor>(nodePtr);
        }
        m_nodeMonitor->getOsData(reportData);
     }

     periodic_report_static_data monitor_only::getStaticConfigInfo() {
       if(nullptr == m_nodeMonitor) {
            auto nodePtr = getNodePtr();
            m_nodeMonitor = std::make_shared<UranusNodeMonitor>(nodePtr);
        }
        return m_nodeMonitor->getStaticConfigInfo();
     }
  }
} ///namespace ultrainio
