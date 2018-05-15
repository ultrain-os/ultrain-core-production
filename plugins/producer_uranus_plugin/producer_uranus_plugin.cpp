/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/producer_uranus_plugin/producer_uranus_plugin.hpp>
#include <iostream>
#include <unistd.h>
#include <string>
#include <boost/asio.hpp>
#include <chrono>
#include <fc/log/logger.hpp>

#include <ultrainio/producer_uranus_plugin/define.hpp>
#include <ultrainio/producer_uranus_plugin/node.hpp>
#include <ultrainio/producer_uranus_plugin/pktmanage.hpp>
#include <ultrainio/producer_uranus_plugin/connect.hpp>
#include <ultrainio/producer_uranus_plugin/security.hpp>

namespace ultrainio {
   static appbase::abstract_plugin& _producer_uranus_plugin = app().register_plugin<producer_uranus_plugin>();

class producer_uranus_plugin_impl {
   public:
    producer_uranus_plugin_impl(boost::asio::io_service& io) {}

    bool createKeyPair(){
       if (ultrain::security::vrf_keypair(uranus_public_key,uranus_private_key))
          return false;

       return true;
    }

    boost::asio::io_service io_service;
};

producer_uranus_plugin::producer_uranus_plugin():my(new producer_uranus_plugin_impl(app().get_io_service())){}
producer_uranus_plugin::~producer_uranus_plugin(){}

void producer_uranus_plugin::set_program_options(options_description&, options_description& cfg) {
   cfg.add_options()
         ("option-name", bpo::value<string>()->default_value("default value"),
          "Option Description")
         ;
}

void producer_uranus_plugin::plugin_initialize(const variables_map& options) {
   if(options.count("option-name")) {
      // Handle the option
   }
}

void producer_uranus_plugin::plugin_startup() {
   // Make the magic happen
   ilog("uranus algorithm starting");

   boost::asio::io_service io_service;

   if (!my->createKeyPair())
      return;

   ultrain::UranusNode node(my->io_service);

   while (true) {
      try {
         node.startup();
      }
      catch (const std::exception& e)
      {
         std::cerr << "Exception:" << e.what() << std::endl;
         return;
      }
   }
//    sleep(1);
//    for(int a = 10; a < 20 ; a++){
//        ilog("uranus algorithm");
//        sleep(1);
//    }
}

void producer_uranus_plugin::plugin_shutdown() {
   // OK, that's enough magic
}

}
