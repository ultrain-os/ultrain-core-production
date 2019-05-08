/**
 *  @file
 *  @copyright defined in ultrainio/LICENSE.txt
 */
#include <appbase/application.hpp>

#include <ultrainio/http_plugin/http_plugin.hpp>
#include <ultrainio/sync_net_plugin/sync_net_plugin.hpp>
#include <ultrainio/sync_net_api_plugin/sync_net_api_plugin.hpp>

#include <fc/log/logger_config.hpp>
#include <fc/log/appender.hpp>
#include <fc/exception/exception.hpp>

#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <ultrainio/utilities/common.hpp>
#include "config.hpp"

namespace bpo = boost::program_options;

using bpo::options_description;
using bpo::variables_map;
using std::string;
using std::vector;

using namespace ultrainio;

namespace fc {
    std::unordered_map<std::string,appender::ptr>& get_appender_map();
}

namespace detail {

    void configure_logging(const bfs::path& config_path)
    {
       try {
          try {
             fc::configure_logging(config_path);
          } catch (...) {
             elog("Error reloading logging.json");
             throw;
          }
       } catch (const fc::exception& e) {
          elog("${e}", ("e",e.to_detail_string()));
       } catch (const boost::exception& e) {
          elog("${e}", ("e",boost::diagnostic_information(e)));
       } catch (const std::exception& e) {
          elog("${e}", ("e",e.what()));
       } catch (...) {
          // empty
       }
    }

} // namespace detail

void logging_conf_loop()
{
   std::shared_ptr<boost::asio::signal_set> sighup_set(new boost::asio::signal_set(app().get_io_service(), SIGHUP));
   sighup_set->async_wait([sighup_set](const boost::system::error_code& err, int /*num*/) {
       if(!err)
       {
          ilog("Received HUP.  Reloading logging configuration.");
          auto config_path = app().get_logging_conf();
          if(fc::exists(config_path))
             ::detail::configure_logging(config_path);
          for(auto iter : fc::get_appender_map())
             iter.second->initialize(app().get_io_service());
          logging_conf_loop();
       }
   });
}

void initialize_logging()
{
   auto config_path = app().get_logging_conf();
   if(fc::exists(config_path))
      fc::configure_logging(config_path); // intentionally allowing exceptions to escape
   for(auto iter : fc::get_appender_map())
      iter.second->initialize(app().get_io_service());

   logging_conf_loop();
}

enum return_codes {
    OTHER_FAIL        = -2,
    INITIALIZE_FAIL   = -1,
    SUCCESS           = 0,
    BAD_ALLOC         = 1,
    FIXED_REVERSIBLE  = 3,
    EXTRACTED_GENESIS = 4,
    NODE_MANAGEMENT_SUCCESS = 5
};

int main( int argc, char** argv ) {
   try {
      app().set_version(ultrainio::wss::config::version);
      appbase::app().register_plugin<sync_net_plugin>();

      auto root = fc::app_path();
      app().set_default_data_dir(root / "ultrainio/wssultrain/data" );
      app().set_default_config_dir(root / "ultrainio/wssultrain/config" );
      if( !appbase::app().initialize<http_plugin, sync_net_plugin,sync_net_api_plugin>( argc, argv ) )
         return INITIALIZE_FAIL;
      initialize_logging();
      ilog("wssultrain root is ${root}", ("root", root.string()));
      ilog("wssultrain version ${ver}", ("ver", ultrainio::utilities::common::itoh(static_cast<uint32_t>(app().version()))));
      appbase::app().startup();
      appbase::app().exec();
   } catch ( const boost::exception& e ) {
      std::cerr << boost::diagnostic_information(e) << "\n";
   } catch ( const std::exception& e ) {
      elog("${e}", ("e",e.what()));
      return OTHER_FAIL;
   } catch ( ... ) {
      elog("unknown exception");
      return OTHER_FAIL;
   }
   ilog("wssultrain exited");
   return 0;
}
