/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/txn_test_gen_plugin/txn_test_gen_plugin.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <ultrainio/chain/wast_to_wasm.hpp>
#include <ultrainio/utilities/key_conversion.hpp>

#include <fc/variant.hpp>
#include <fc/io/json.hpp>
#include <fc/exception/exception.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/io/json.hpp>

#include <boost/asio/high_resolution_timer.hpp>
#include <boost/algorithm/clamp.hpp>

#include <Inline/BasicTypes.h>
#include <IR/Module.h>
#include <IR/Validate.h>
#include <WAST/WAST.h>
#include <WASM/WASM.h>
#include <Runtime/Runtime.h>

#include <ultrainio.token/ultrainio.token.wast.hpp>
#include <ultrainio.token/ultrainio.token.abi.hpp>
#include <hello/hello.wast.hpp>
#include <hello/hello.abi.hpp>

namespace ultrainio { namespace detail {
  struct txn_test_gen_empty {};
}}

FC_REFLECT(ultrainio::detail::txn_test_gen_empty, );

namespace ultrainio {

static appbase::abstract_plugin& _txn_test_gen_plugin = app().register_plugin<txn_test_gen_plugin>();
static abi_serializer* hello_serializer = nullptr;
static fc::microseconds abi_serializer_max_time;

using namespace ultrainio::chain;

#define CALL(api_name, api_handle, call_name, INVOKE, http_response_code) \
{std::string("/v1/" #api_name "/" #call_name), \
   [this](string, string body, url_response_callback cb) mutable { \
          try { \
             if (body.empty()) body = "{}"; \
             INVOKE \
             cb(http_response_code, fc::json::to_string(result)); \
          } catch (...) { \
             http_plugin::handle_exception(#api_name, #call_name, body, cb); \
          } \
       }}

#define INVOKE_V_R_R_R(api_handle, call_name, in_param0, in_param1, in_param2) \
     const auto& vs = fc::json::json::from_string(body).as<fc::variants>(); \
     api_handle->call_name(vs.at(0).as<in_param0>(), vs.at(1).as<in_param1>(), vs.at(2).as<in_param2>()); \
     ultrainio::detail::txn_test_gen_empty result;

#define INVOKE_V_R_R(api_handle, call_name, in_param0, in_param1) \
     const auto& vs = fc::json::json::from_string(body).as<fc::variants>(); \
     api_handle->call_name(vs.at(0).as<in_param0>(), vs.at(1).as<in_param1>()); \
     ultrainio::detail::txn_test_gen_empty result;

#define INVOKE_V_V(api_handle, call_name) \
     api_handle->call_name(); \
     ultrainio::detail::txn_test_gen_empty result;

#define CALL_ASYNC(api_name, api_handle, call_name, INVOKE, http_response_code) \
{std::string("/v1/" #api_name "/" #call_name), \
   [this](string, string body, url_response_callback cb) mutable { \
      if (body.empty()) body = "{}"; \
      auto result_handler = [cb, body](const fc::exception_ptr& e) {\
         if (e) {\
            try {\
               e->dynamic_rethrow_exception();\
            } catch (...) {\
               http_plugin::handle_exception(#api_name, #call_name, body, cb);\
            }\
         } else {\
            cb(http_response_code, fc::json::to_string(ultrainio::detail::txn_test_gen_empty())); \
         }\
      };\
      INVOKE \
   }\
}

#define INVOKE_ASYNC_R_R(api_handle, call_name, in_param0, in_param1) \
   const auto& vs = fc::json::json::from_string(body).as<fc::variants>(); \
   api_handle->call_name(vs.at(0).as<in_param0>(), vs.at(1).as<in_param1>(), result_handler);

struct txn_test_gen_plugin_impl {
   static void push_next_transaction(const std::shared_ptr<std::vector<signed_transaction>>& trxs, size_t index, const std::function<void(const fc::exception_ptr&)>& next ) {
      //ilog("push_next_transaction: trxs->size=${p} index=${m}", ("p", trxs->size())("m", index));
      chain_plugin& cp = app().get_plugin<chain_plugin>();
      cp.accept_transaction( packed_transaction(trxs->at(index), packed_transaction::zlib), false, [=](const fc::static_variant<fc::exception_ptr, transaction_trace_ptr>& result){
      //      cp.accept_transaction( packed_transaction(trxs->at(index)), [=](const fc::static_variant<fc::exception_ptr, transaction_trace_ptr>& result){
	  if (index + 1 < trxs->size()) {
	      push_next_transaction(trxs, index + 1, next);
	  } else {
	       next(nullptr);
          }
         /*if (result.contains<fc::exception_ptr>()) {
            next(result.get<fc::exception_ptr>());
         } else {
            if (index + 1 < trxs->size()) {
               push_next_transaction(trxs, index + 1, next);
            } else {
               next(nullptr);
            }
         }*/
      });
   }

   void push_transactions( std::vector<signed_transaction>&& trxs, const std::function<void(fc::exception_ptr)>& next ) {
      auto trxs_copy = std::make_shared<std::decay_t<decltype(trxs)>>(std::move(trxs));
      push_next_transaction(trxs_copy, 0, next);
   }

   void create_test_accounts(const std::string& init_name, const std::string& init_priv_key, const std::function<void(const fc::exception_ptr&)>& next) {
      std::vector<signed_transaction> trxs;
      trxs.reserve(2);

      try {
         name newaccountA("txn.test.a");
         name newaccountB("txn.test.b");
         name newaccountC("txn.test.t");
         name creator(init_name);

         abi_def currency_abi_def = fc::json::from_string(ultrainio_token_abi).as<abi_def>();

         controller& cc = app().get_plugin<chain_plugin>().chain();
         auto chainid = app().get_plugin<chain_plugin>().get_chain_id();
         abi_serializer_max_time = app().get_plugin<chain_plugin>().get_abi_serializer_max_time();

         abi_serializer ultrainio_token_serializer{fc::json::from_string(ultrainio_token_abi).as<abi_def>(), abi_serializer_max_time};

         fc::crypto::private_key txn_test_receiver_A_priv_key = fc::crypto::private_key::regenerate(fc::sha256(std::string(64, 'a')));
         fc::crypto::private_key txn_test_receiver_B_priv_key = fc::crypto::private_key::regenerate(fc::sha256(std::string(64, 'b')));
         fc::crypto::private_key txn_test_receiver_C_priv_key = fc::crypto::private_key::regenerate(fc::sha256(std::string(64, 'c')));
         fc::crypto::public_key  txn_text_receiver_A_pub_key = txn_test_receiver_A_priv_key.get_public_key();
         fc::crypto::public_key  txn_text_receiver_B_pub_key = txn_test_receiver_B_priv_key.get_public_key();
         fc::crypto::public_key  txn_text_receiver_C_pub_key = txn_test_receiver_C_priv_key.get_public_key();
         fc::crypto::private_key creator_priv_key = fc::crypto::private_key(init_priv_key);

         //create some test accounts
         {
            signed_transaction trx;

            //create "A" account
            {
            auto owner_auth   = ultrainio::chain::authority{1, {{txn_text_receiver_A_pub_key, 1}}, {}};
            auto active_auth  = ultrainio::chain::authority{1, {{txn_text_receiver_A_pub_key, 1}}, {}};

            trx.actions.emplace_back(vector<chain::permission_level>{{creator,"active"}}, newaccount{creator, newaccountA, owner_auth, active_auth});
            }
            //create "B" account
            {
            auto owner_auth   = ultrainio::chain::authority{1, {{txn_text_receiver_B_pub_key, 1}}, {}};
            auto active_auth  = ultrainio::chain::authority{1, {{txn_text_receiver_B_pub_key, 1}}, {}};

            trx.actions.emplace_back(vector<chain::permission_level>{{creator,"active"}}, newaccount{creator, newaccountB, owner_auth, active_auth});
            }
            //create "txn.test.t" account
            {
            auto owner_auth   = ultrainio::chain::authority{1, {{txn_text_receiver_C_pub_key, 1}}, {}};
            auto active_auth  = ultrainio::chain::authority{1, {{txn_text_receiver_C_pub_key, 1}}, {}};

            trx.actions.emplace_back(vector<chain::permission_level>{{creator,"active"}}, newaccount{creator, newaccountC, owner_auth, active_auth});
            }

            trx.expiration = cc.head_block_time() + fc::seconds(30);
            trx.set_reference_block(cc.head_block_id());
            trx.sign(creator_priv_key, chainid);
            trxs.emplace_back(std::move(trx));
         }

         //set txn.test.t contract to ultrainio.token & initialize it
         {
            signed_transaction trx;

            vector<uint8_t> wasm = wast_to_wasm(std::string(ultrainio_token_wast));

            setcode handler;
            handler.account = newaccountC;
            handler.code.assign(wasm.begin(), wasm.end());

            trx.actions.emplace_back( vector<chain::permission_level>{{newaccountC,"active"}}, handler);

            {
               setabi handler;
               handler.account = newaccountC;
               handler.abi = fc::raw::pack(json::from_string(ultrainio_token_abi).as<abi_def>());
               trx.actions.emplace_back( vector<chain::permission_level>{{newaccountC,"active"}}, handler);
            }

            {
               action act;
               act.account = N(txn.test.t);
               act.name = NEX(create);
               act.authorization = vector<permission_level>{{newaccountC,config::active_name}};
               act.data = ultrainio_token_serializer.variant_to_binary("create", fc::json::from_string("{\"issuer\":\"txn.test.t\",\"maximum_supply\":\"1000000000.0000 CUR\"}}"), abi_serializer_max_time);
               trx.actions.push_back(act);
            }
            {
               action act;
               act.account = N(txn.test.t);
               act.name = NEX(issue);
               act.authorization = vector<permission_level>{{newaccountC,config::active_name}};
               act.data = ultrainio_token_serializer.variant_to_binary("issue", fc::json::from_string("{\"to\":\"txn.test.t\",\"quantity\":\"600.0000 CUR\",\"memo\":\"\"}"), abi_serializer_max_time);
               trx.actions.push_back(act);
            }
            {
               action act;
               act.account = N(txn.test.t);
               act.name = NEX(transfer);
               act.authorization = vector<permission_level>{{newaccountC,config::active_name}};
               act.data = ultrainio_token_serializer.variant_to_binary("transfer", fc::json::from_string("{\"from\":\"txn.test.t\",\"to\":\"txn.test.a\",\"quantity\":\"200.0000 CUR\",\"memo\":\"\"}"), abi_serializer_max_time);
               trx.actions.push_back(act);
            }
            {
               action act;
               act.account = N(txn.test.t);
               act.name = NEX(transfer);
               act.authorization = vector<permission_level>{{newaccountC,config::active_name}};
               act.data = ultrainio_token_serializer.variant_to_binary("transfer", fc::json::from_string("{\"from\":\"txn.test.t\",\"to\":\"txn.test.b\",\"quantity\":\"200.0000 CUR\",\"memo\":\"\"}"), abi_serializer_max_time);
               trx.actions.push_back(act);
            }

            trx.expiration = cc.head_block_time() + fc::seconds(60);
            trx.set_reference_block(cc.head_block_id());
            trx.max_net_usage_words = 5000;
            trx.sign(txn_test_receiver_C_priv_key, chainid);
            trxs.emplace_back(std::move(trx));
         }
      } catch (const fc::exception& e) {
         next(e.dynamic_copy_exception());
         return;
      }

      push_transactions(std::move(trxs), next);
   }

   void start_generation_transfer(const std::string& salt, const uint64_t& period, const uint64_t& batch_size) {
      if(running)
         throw fc::exception(fc::invalid_operation_exception_code);
      if(period < 1 || period > 2500)
         throw fc::exception(fc::invalid_operation_exception_code);
      if(batch_size < 1 || batch_size > 250)
         throw fc::exception(fc::invalid_operation_exception_code);
     // if(batch_size & 1)
     //    throw fc::exception(fc::invalid_operation_exception_code);

      is_transfer_test = true;
      running = true;

      controller& cc = app().get_plugin<chain_plugin>().chain();
      abi_serializer_max_time = app().get_plugin<chain_plugin>().get_abi_serializer_max_time();
      abi_serializer ultrainio_token_serializer{fc::json::from_string(ultrainio_token_abi).as<abi_def>(), abi_serializer_max_time};
      //create the actions here
      act_a_to_b.account = N(utrio.token);
      act_a_to_b.name = NEX(transfer);
      act_a_to_b.authorization = vector<permission_level>{{name("user.111"),config::active_name}};
      act_a_to_b.data = ultrainio_token_serializer.variant_to_binary("transfer",
                                                                  fc::json::from_string(fc::format_string("{\"from\":\"user.111\",\"to\":\"user.112\",\"quantity\":\"0.1234 UGAS\",\"memo\":\"${l}\"}",
                                                                  fc::mutable_variant_object()("l", salt))),
                                                                  abi_serializer_max_time);

      act_b_to_a.account = N(utrio.token);
      act_b_to_a.name = NEX(transfer);
      act_b_to_a.authorization = vector<permission_level>{{name("user.112"),config::active_name}};
      act_b_to_a.data = ultrainio_token_serializer.variant_to_binary("transfer",
                                                                  fc::json::from_string(fc::format_string("{\"from\":\"user.112\",\"to\":\"user.111\",\"quantity\":\"0.1234 UGAS\",\"memo\":\"${l}\"}",
                                                                  fc::mutable_variant_object()("l", salt))),
                                                                  abi_serializer_max_time);

      timer_timeout = period;
      //batch = batch_size/2;
      batch = batch_size;

      ilog("Started transaction test plugin; performing ${p} transactions every ${m}ms", ("p", batch_size)("m", period));

      arm_timer(boost::asio::high_resolution_timer::clock_type::now());
   }

   void start_generation_hello(const std::string& salt, const uint64_t& period, const uint64_t& batch_size) {
      if(running)
         throw fc::exception(fc::invalid_operation_exception_code);
      if(period < 1 || period > 2500)
         throw fc::exception(fc::invalid_operation_exception_code);
      if(batch_size < 1 || batch_size > 250)
         throw fc::exception(fc::invalid_operation_exception_code);
     // if(batch_size & 1)
     //    throw fc::exception(fc::invalid_operation_exception_code);

      running = true;
      is_hello_test = true;

      controller& cc = app().get_plugin<chain_plugin>().chain();
      abi_serializer_max_time = app().get_plugin<chain_plugin>().get_abi_serializer_max_time();
      if (!hello_serializer) {
          hello_serializer = new abi_serializer(fc::json::from_string(hello_abi).as<abi_def>(), abi_serializer_max_time);
      }
      //create the actions here
      hello.account = N(hello);
      hello.name = NEX(hi);
      hello.authorization = vector<permission_level>{{name("user.111"),config::active_name}};
      hello.data = hello_serializer->variant_to_binary("hi",fc::json::from_string("{\"user\":\"hhh\"}"), abi_serializer_max_time);
      timer_timeout = period;
      //batch = batch_size/2;
      batch = batch_size;

      ilog("Started transaction test plugin; performing ${p} transactions every ${m}ms", ("p", batch_size)("m", period));

      arm_timer(boost::asio::high_resolution_timer::clock_type::now());
   }

   void arm_timer(boost::asio::high_resolution_timer::time_point s) {
      timer.expires_at(s + std::chrono::milliseconds(timer_timeout));
      timer.async_wait([this](const boost::system::error_code& ec) {
         if(!running || ec)
            return;

         send_transaction([this](const fc::exception_ptr& e){
//            if (e) {
//               elog("pushing transaction failed: ${e}", ("e", e->to_detail_string()));
//               stop_generation();
//            } else {
//               arm_timer(timer.expires_at());
//            }
             arm_timer(timer.expires_at());
         });
      });
   }

   void send_transaction(std::function<void(const fc::exception_ptr&)> next) {
      std::vector<signed_transaction> trxs;
      trxs.reserve(2*batch);

      try {
          controller& cc = app().get_plugin<chain_plugin>().chain();
          auto chainid = app().get_plugin<chain_plugin>().get_chain_id();

          fc::crypto::private_key a_priv_key(std::string("5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H"));
          fc::crypto::private_key b_priv_key(std::string("5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H"));

         static uint64_t nonce = static_cast<uint64_t>(fc::time_point::now().sec_since_epoch()) << 32;
         //         abi_serializer ultrainio_serializer(cc.db().find<account_object, by_name>(config::system_account_name)->get_abi());

         uint32_t reference_block_num = cc.last_irreversible_block_num();
         if (txn_reference_block_lag >= 0) {
            reference_block_num = cc.head_block_num();
            if (reference_block_num <= (uint32_t)txn_reference_block_lag) {
               reference_block_num = 0;
            } else {
               reference_block_num -= (uint32_t)txn_reference_block_lag;
            }
         }

         block_id_type reference_block_id = cc.get_block_id_for_num(reference_block_num);

         for(unsigned int i = 0; i < batch; ++i) {
             if(trx_count%1000 == 0){
                 ilog("trx_count ${p}", ("p", trx_count));
             }

             if (is_transfer_test) {
                 {
                     signed_transaction trx;
                     trx_count++;
                     trx.actions.push_back(act_a_to_b);
                     trx.context_free_actions.emplace_back(action({}, config::null_account_name, "nonce", fc::raw::pack(nonce++)));
                     trx.set_reference_block(reference_block_id);
                     trx.expiration = cc.head_block_time() + fc::seconds(60);
                     trx.max_net_usage_words = 100;
                     trx.sign(a_priv_key, chainid);
                     trxs.emplace_back(std::move(trx));
                 }

                 {
                     signed_transaction trx;
                     trx_count++;
                     trx.actions.push_back(act_b_to_a);
                     trx.context_free_actions.emplace_back(action({}, config::null_account_name, "nonce", fc::raw::pack(nonce++)));
                     trx.set_reference_block(reference_block_id);
                     trx.expiration = cc.head_block_time() + fc::seconds(60);
                     trx.max_net_usage_words = 100;
                     trx.sign(b_priv_key, chainid);
                     trxs.emplace_back(std::move(trx));
                 }
             }

             if (is_hello_test) {
                 {
                     signed_transaction trx;
                     trx_count++;
                     // Lets use different payload to generate unique trx, so to avoid embedding context free action.
                     char buf[5] = "aaaa";
                     int t = trx_count;
                     for (int i = 0; i < 4; i++) {
                         buf[i] = (t % 26) + 'a';
                         t = t / 26;
                     }
                     std::string s = "{\"user\":\"" + std::string(buf) + std::string("\"}");
                     hello.data = hello_serializer->variant_to_binary("hi",fc::json::from_string(s.c_str()), abi_serializer_max_time);
                     trx.actions.push_back(hello);
                     // trx.context_free_actions.emplace_back(action({}, config::null_account_name, "nonce", fc::raw::pack(nonce++)));
                     trx.set_reference_block(reference_block_id);
                     trx.expiration = cc.head_block_time() + fc::seconds(60);
                     trx.max_net_usage_words = 100;
                     trx.sign(a_priv_key, chainid);
                     trxs.emplace_back(std::move(trx));
                 }
             }
         }
      } catch ( const fc::exception& e ) {
         next(e.dynamic_copy_exception());
      }

      push_transactions(std::move(trxs), next);
   }

   void stop_generation() {
      if(!running)
         throw fc::exception(fc::invalid_operation_exception_code);
      timer.cancel();
      running = false;
      is_transfer_test = false;
      is_hello_test = false;
      ilog("Stopping transaction generation test");
   }

    boost::asio::high_resolution_timer timer{app().get_io_service()};
    bool running{false};

    bool is_transfer_test{false};
    bool is_hello_test{false};

    unsigned timer_timeout;
    unsigned batch;

    action act_a_to_b;
    action act_b_to_a;
    action hello;

    int32_t txn_reference_block_lag;

//    abi_serializer ultrainio_token_serializer = fc::json::from_string(ultrainio_token_abi).as<abi_def>();
    static int64_t trx_count;

};

int64_t txn_test_gen_plugin_impl::trx_count = 0;
txn_test_gen_plugin::txn_test_gen_plugin() {}
txn_test_gen_plugin::~txn_test_gen_plugin() {}

void txn_test_gen_plugin::set_program_options(options_description&, options_description& cfg) {
   cfg.add_options()
      ("txn-reference-block-lag", bpo::value<int32_t>()->default_value(0), "Lag in number of blocks from the head block when selecting the reference block for transactions (-1 means Last Irreversible Block)")
   ;
}

void txn_test_gen_plugin::plugin_initialize(const variables_map& options) {
   try {
      my.reset( new txn_test_gen_plugin_impl );
      my->txn_reference_block_lag = options.at( "txn-reference-block-lag" ).as<int32_t>();
   } FC_LOG_AND_RETHROW()
}

void txn_test_gen_plugin::plugin_startup() {
   app().get_plugin<http_plugin>().add_api({
      CALL_ASYNC(txn_test_gen, my, create_test_accounts, INVOKE_ASYNC_R_R(my, create_test_accounts, std::string, std::string), 200),
      CALL(txn_test_gen, my, stop_generation, INVOKE_V_V(my, stop_generation), 200),
      CALL(txn_test_gen, my, start_generation_transfer, INVOKE_V_R_R_R(my, start_generation_transfer, std::string, uint64_t, uint64_t), 200),
      CALL(txn_test_gen, my, start_generation_hello, INVOKE_V_R_R_R(my, start_generation_hello, std::string, uint64_t, uint64_t), 200)
   });
}

void txn_test_gen_plugin::plugin_shutdown() {
   try {
      my->stop_generation();
   }
   catch(fc::exception e) {
   }
}

}
