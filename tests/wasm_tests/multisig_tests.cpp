#include <boost/test/unit_test.hpp>
#include <ultrainio/testing/tester.hpp>
#include <ultrainio/chain/contracts/abi_serializer.hpp>
#include <ultrainio/chain/wast_to_wasm.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>

#include <ultrainio.msig/ultrainio.msig.wast.hpp>
#include <ultrainio.msig/ultrainio.msig.abi.hpp>

#include <exchange/exchange.wast.hpp>
#include <exchange/exchange.abi.hpp>

#include <Runtime/Runtime.h>

#include <fc/variant_object.hpp>

using namespace ultrainio::testing;
using namespace ultrainio;
using namespace ultrainio::chain;
using namespace ultrainio::chain::contracts;
using namespace ultrainio::chain_apis;
using namespace ultrainio::testing;
using namespace fc;

using mvo = fc::mutable_variant_object;

class ultrainio_msig_tester : public tester {
public:

   ultrainio_msig_tester() {
      create_accounts( { N(ultrainio.msig), N(alice), N(bob), N(carol) } );
      produce_block();

      auto trace = base_tester::push_action(config::system_account_name, N(setpriv),
                                            config::system_account_name,  mutable_variant_object()
                                            ("account", "ultrainio.msig")
                                            ("is_priv", 1)
      );

      set_code( N(ultrainio.msig), ultrainio_msig_wast );
      set_abi( N(ultrainio.msig), ultrainio_msig_abi );

      produce_blocks();
      const auto& accnt = control->get_database().get<account_object,by_name>( N(ultrainio.msig) );
      abi_def abi;
      BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
      abi_ser.set_abi(abi);
   }

   action_result push_action( const account_name& signer, const action_name &name, const variant_object &data, bool auth = true ) {
         string action_type_name = abi_ser.get_action_type(name);

         action act;
         act.account = N(ultrainio.msig);
         act.name = name;
         act.data = abi_ser.variant_to_binary( action_type_name, data );
         //std::cout << "test:\n" << fc::to_hex(act.data.data(), act.data.size()) << " size = " << act.data.size() << std::endl;

         return base_tester::push_action( std::move(act), auth ? uint64_t(signer) : 0 );
   }

   transaction reqauth( account_name from, const vector<permission_level>& auths );

   abi_serializer abi_ser;
};

transaction ultrainio_msig_tester::reqauth( account_name from, const vector<permission_level>& auths ) {
   fc::variants v;
   for ( auto& level : auths ) {
      v.push_back(fc::mutable_variant_object()
                  ("actor", level.actor)
                  ("permission", level.permission)
      );
   }
   variant pretty_trx = fc::mutable_variant_object()
      ("expiration", "2020-01-01T00:30")
      ("region", 0)
      ("ref_block_num", 2)
      ("ref_block_prefix", 3)
      ("max_net_usage_words", 0)
      ("max_kcpu_usage", 0)
      ("delay_sec", 0)
      ("actions", fc::variants({
            fc::mutable_variant_object()
               ("account", name(config::system_account_name))
               ("name", "reqauth")
               ("authorization", v)
               ("data", fc::mutable_variant_object() ("from", from) )
               })
      );
   transaction trx;
   contracts::abi_serializer::from_variant(pretty_trx, trx, get_resolver());
   return trx;
}

BOOST_AUTO_TEST_SUITE(ultrainio_msig_tests)

BOOST_FIXTURE_TEST_CASE( propose_approve_execute, ultrainio_msig_tester ) try {
   auto trx = reqauth("alice", {permission_level{N(alice), config::active_name}} );

   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(propose), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("trx",           trx)
                                                ("requested", vector<permission_level>{{ N(alice), config::active_name }})
                        ));

   //fail to execute before approval
   BOOST_REQUIRE_EQUAL( error("transaction declares authority '{\"actor\":\"alice\",\"permission\":\"active\"}', but does not have signatures for it."),
                        push_action( N(alice), N(exec), mvo()
                                     ("proposer",      "alice")
                                     ("proposal_name", "first")
                                     ("executer",      "alice")
                        ));

   //approve and execute
   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(approve), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(alice), config::active_name })
                        ));

   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(exec), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("executer",      "alice")
                        ));

   auto traces = control->push_deferred_transactions( true );
   BOOST_CHECK_EQUAL( 1, traces.size() );
   BOOST_CHECK_EQUAL( 1, traces.at(0).action_traces.size() );
   BOOST_CHECK_EQUAL( transaction_receipt::executed, traces.at(0).status );
} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( propose_approve_unapprove, ultrainio_msig_tester ) try {
   auto trx = reqauth("alice", {permission_level{N(alice), config::active_name}} );

   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(propose), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("trx",           trx)
                                                ("requested", vector<permission_level>{{ N(alice), config::active_name }})
                        ));

   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(approve), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(alice), config::active_name })
                        ));

   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(unapprove), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(alice), config::active_name })
                        ));

   BOOST_REQUIRE_EQUAL( error("transaction declares authority '{\"actor\":\"alice\",\"permission\":\"active\"}', but does not have signatures for it."),
                        push_action( N(alice), N(exec), mvo()
                                     ("proposer",      "alice")
                                     ("proposal_name", "first")
                                     ("executer",      "alice")
                        ));
} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( propose_approve_by_two, ultrainio_msig_tester ) try {
   auto trx = reqauth("alice", vector<permission_level>{ { N(alice), config::active_name }, { N(bob), config::active_name } } );
   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(propose), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("trx",           trx)
                                                ("requested", vector<permission_level>{ { N(alice), config::active_name }, { N(bob), config::active_name } })
                        ));

   //approve by alice
   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(approve), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(alice), config::active_name })
                        ));

   //fail because approval by bob is missing
   BOOST_REQUIRE_EQUAL( error("transaction declares authority '{\"actor\":\"bob\",\"permission\":\"active\"}', but does not have signatures for it."),
                        push_action( N(alice), N(exec), mvo()
                                     ("proposer",      "alice")
                                     ("proposal_name", "first")
                                     ("executer",      "alice")
                        ));

   //approve by bob and execute
   BOOST_REQUIRE_EQUAL( success(), push_action( N(bob), N(approve), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(bob), config::active_name })
                        ));
   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(exec), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("executer",      "alice")
                        ));
   auto traces = control->push_deferred_transactions( true );
   BOOST_CHECK_EQUAL( 1, traces.size() );
   BOOST_CHECK_EQUAL( 1, traces.at(0).action_traces.size() );
   BOOST_CHECK_EQUAL( transaction_receipt::executed, traces.at(0).status );
} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( propose_with_wrong_requested_auth, ultrainio_msig_tester ) try {
   auto trx = reqauth("alice", vector<permission_level>{ { N(alice), config::active_name },  { N(bob), config::active_name } } );
   //try with not enough requested auth
   BOOST_REQUIRE_EQUAL( error("transaction declares authority '{\"actor\":\"bob\",\"permission\":\"active\"}', but does not have signatures for it."),
                        push_action( N(alice), N(propose), mvo()
                                     ("proposer",      "alice")
                                     ("proposal_name", "third")
                                     ("trx",           trx)
                                     ("requested", vector<permission_level>{ { N(alice), config::active_name } } )
                        ));
} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( big_transaction, ultrainio_msig_tester ) try {
   vector<permission_level> perm = { { N(alice), config::active_name }, { N(bob), config::active_name } };
   auto wasm = wast_to_wasm( exchange_wast );

   variant pretty_trx = fc::mutable_variant_object()
      ("expiration", "2020-01-01T00:30")
      ("region", 0)
      ("ref_block_num", 2)
      ("ref_block_prefix", 3)
      ("max_net_usage_words", 0)
      ("max_kcpu_usage", 0)
      ("delay_sec", 0)
      ("actions", fc::variants({
            fc::mutable_variant_object()
               ("account", name(config::system_account_name))
               ("name", "setcode")
               ("authorization", perm)
               ("data", fc::mutable_variant_object()
                ("account", "alice")
                ("vmtype", 0)
                ("vmversion", 0)
                ("code", bytes( wasm.begin(), wasm.end() ))
               )
               })
      );

   transaction trx;
   contracts::abi_serializer::from_variant(pretty_trx, trx, get_resolver());

   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(propose), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("trx",           trx)
                                                ("requested", perm)
                        ));

   //approve by alice
   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(approve), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(alice), config::active_name })
                        ));
   //approve by bob and execute
   BOOST_REQUIRE_EQUAL( success(), push_action( N(bob), N(approve), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("level",         permission_level{ N(bob), config::active_name })
                        ));
   BOOST_REQUIRE_EQUAL( success(), push_action( N(alice), N(exec), mvo()
                                                ("proposer",      "alice")
                                                ("proposal_name", "first")
                                                ("executer",      "alice")
                        ));
   auto traces = control->push_deferred_transactions( true );
   BOOST_CHECK_EQUAL( 1, traces.size() );
   BOOST_CHECK_EQUAL( 1, traces.at(0).action_traces.size() );
   BOOST_CHECK_EQUAL( transaction_receipt::executed, traces.at(0).status );
} FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
