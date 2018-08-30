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
#include "httpc.hpp"

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

    void startMonitorTaskTimer();

    tcp::endpoint  self_endpoint; // same as p2p-listen-endpoint in net plugin
    std::string    monitor_central_server;
    std::string    call_path;
    bool           needReportTask = true;
    uint32_t       reportInterval;

    monitor_apis::monitor_only m_monitorHandler;

  private:
    void processReportTask();

    std::unique_ptr<boost::asio::steady_timer> m_reportTaskTimer;
};

monitor_plugin_impl::monitor_plugin_impl() {
    m_reportTaskTimer.reset(new boost::asio::steady_timer(app().get_io_service()));
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
            processReportTask();
            startMonitorTaskTimer();
        }
    });
}

void monitor_plugin_impl::processReportTask() {
  try{
    periodic_reort_data rst = m_monitorHandler.getPeriodicReortData();
    rst.nodeIp = self_endpoint.address().to_v4().to_string();
    auto rsp = call(monitor_central_server, call_path, rst);
  }
  catch(chain::node_not_found_exception& e) {
    //auto exceptionInfo = std::string("exception happened: node not initialized.");
    //call(monitor_central_server, call_path, exceptionInfo);
  }
  catch(...) {} //don't allow exception be thrown out, to prevent Ultrainode from exiting.
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

    my->call_path = "/status/info";

   context = ultrainio::client::http::create_http_context();
}

void monitor_plugin::plugin_startup() {
   // Make the magic happen
   my->startMonitorTaskTimer();
}

void monitor_plugin::plugin_shutdown() {
   // OK, that's enough magic
}

monitor_apis::monitor_only  monitor_plugin::get_monitor_only_api()const {
    return my->m_monitorHandler;
}

   namespace monitor_apis {

     const std::shared_ptr<UranusNode>  monitor_only::getNodePtr() const {
       auto nodePtr = UranusNode::getInstance();
       ULTRAIN_ASSERT( nodePtr != nullptr, chain::node_not_found_exception, "Failed to get UranusNode. Most possible reason: This Node is not a producer." );
       return nodePtr;
     }

     monitor_only::monitor_node_result monitor_only::monitor_node( const monitor_only::monitor_node_params& params )const {
        auto nodePtr = getNodePtr();
        UranusNodeMonitor nodeMonitor(nodePtr);
        return nodeMonitor.getNodeInfo();
     }

     monitor_only::monitor_propose_msg_result monitor_only::monitor_propose_msg( const monitor_only::monitor_propose_msg_params& params) const {
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getController());
        return controllerMonitor.findProposeMsgByBlockId(chain::block_id_type(params.block_id));
     }

     monitor_only::monitor_echo_msg_result monitor_only::monitor_echo_msg( const monitor_only::monitor_echo_msg_params& params) const{
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getController());
        return controllerMonitor.findEchoMsgByBlockId(chain::block_id_type(params.block_id));
     }

     monitor_only::monitor_propose_cache_result monitor_only::monitor_propose_cache(const monitor_only::monitor_propose_cache_params& params) const{
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getController());
        return {controllerMonitor.findProposeCacheByKey(params)};
     }

     monitor_only::monitor_echo_cache_result monitor_only::monitor_echo_cache(const monitor_only::monitor_echo_cache_params& params) const {
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getController());
        return {controllerMonitor.findEchoCacheByKey(params)};
     }

     monitor_only::monitor_echo_ap_cache_result monitor_only::monitor_echo_ap_cache(const monitor_only::monitor_echo_ap_cache_params& params) const {
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getController());
        return {controllerMonitor.findEchoApMsgByKey(params)};
     }

     periodic_reort_data monitor_only::getPeriodicReortData() {
        if(nullptr == m_nodeMonitor) {
            auto nodePtr = getNodePtr();
            m_nodeMonitor = std::make_shared<UranusNodeMonitor>(nodePtr);
            m_nodeMonitor->setCallbackInNode();
        }

        return m_nodeMonitor->getReortData();
     }
   } //namespace monitor_apis 
} ///namespace ultrainio