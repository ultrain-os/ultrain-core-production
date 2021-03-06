/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#pragma once

#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <ultrainio/http_client_plugin/http_client_plugin.hpp>

#include <core/Message.h>

#include <appbase/application.hpp>

namespace ultrainio {

using boost::signals2::signal;

class producer_rpos_plugin : public appbase::plugin<producer_rpos_plugin> {
public:
   APPBASE_PLUGIN_REQUIRES((chain_plugin)(http_client_plugin))

   struct runtime_options {
      fc::optional<int32_t> max_transaction_time;
      fc::optional<int32_t> max_irreversible_block_age;
   };

   producer_rpos_plugin();
   virtual ~producer_rpos_plugin();

   virtual void set_program_options(
      boost::program_options::options_description &command_line_options,
      boost::program_options::options_description &config_file_options
      ) override;

   bool                   is_producer_key(const chain::public_key_type& key) const;
   chain::signature_type  sign_compact(const chain::public_key_type& key, const fc::sha256& digest) const;

   virtual void plugin_initialize(const boost::program_options::variables_map& options);
   virtual void plugin_startup();
   virtual void plugin_shutdown();
   bool handle_message(const EchoMsg& echo);
   bool handle_message(const ProposeMsg& propose);
   bool handle_message(const fc::sha256& node_id, const ReqSyncMsg& msg);
   bool handle_message(const SyncBlockMsg& block, bool last_block, bool safe);
   bool handle_message(const fc::sha256& node_id, const ReqBlockNumRangeMsg& msg);
   bool handle_message(const fc::sha256& node_id, const SyncStopMsg& msg);
   bool sync_fail(const ultrainio::ReqSyncMsg& sync_msg);
   bool sync_cancel();
   int  get_round_interval();
   string get_account_sk();
   string get_account_name();
   void pause();
   void resume();
   bool paused() const;
   void update_runtime_options(const runtime_options& options);
   runtime_options get_runtime_options() const;

   signal<void(const chain::producer_confirmation&)> confirmed_block;
private:
   std::shared_ptr<class producer_rpos_plugin_impl> my;
};

} //ultrainio

//FC_REFLECT(ultrainio::producer_rpos_plugin::runtime_options, (max_transaction_time)(max_irreversible_block_age));
