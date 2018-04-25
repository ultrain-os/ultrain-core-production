#include <boost/test/unit_test.hpp>
#include <ultrainio/testing/tester.hpp>
#include <ultrainio/chain/contracts/abi_serializer.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>

#include <ultrainio.system/ultrainio.system.wast.hpp>
#include <ultrainio.system/ultrainio.system.abi.hpp>
// These contracts are still under dev
#if _READY
#endif
#include <ultrainio.bios/ultrainio.bios.wast.hpp>
#include <ultrainio.bios/ultrainio.bios.abi.hpp>
#include <ultrainio.token/ultrainio.token.wast.hpp>
#include <ultrainio.token/ultrainio.token.abi.hpp>

#include <Runtime/Runtime.h>

#include <fc/variant_object.hpp>

#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif


using namespace ultrainio;
using namespace ultrainio::chain;
using namespace ultrainio::chain::contracts;
using namespace ultrainio::testing;
using namespace fc;

using mvo = fc::mutable_variant_object;

class bootseq_tester : public TESTER
{
public:

    static fc::variant_object producer_parameters_example( int n ) {
        return mutable_variant_object()
                ("target_block_size", 1024 * 1024 + n)
                ("max_block_size", 10 * 1024 + n)
                ("target_block_acts_per_scope", 1000 + n)
                ("max_block_acts_per_scope", 10000 + n)
                ("target_block_acts", 1100 + n)
                ("max_block_acts", 11000 + n)
                ("max_storage_size", 2000 + n)
                ("max_transaction_lifetime", 3600 + n)
                ("max_transaction_exec_time", 9900 + n)
                ("max_authority_depth", 6 + n)
                ("max_inline_depth", 4 + n)
                ("max_inline_action_size", 4096 + n)
                ("max_generated_transaction_size", 64*1024 + n)
                ("percent_of_max_inflation_rate", 50 + n)
                ("storage_reserve_ratio", 100 + n);
    }

    bootseq_tester() {
        const auto &accnt = control->get_database().get<account_object, by_name>(config::system_account_name);
        abi_def abi;
        BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
        abi_ser.set_abi(abi);
    }

    action_result push_action( const account_name& account, const account_name& signer, const action_name &name, const variant_object &data, bool auth = true ) {
        string action_type_name = abi_ser.get_action_type(name);

        action act;
        act.account = account;
        act.name = name;
        act.data = abi_ser.variant_to_binary( action_type_name, data );

        return base_tester::push_action( std::move(act), auth ? uint64_t(signer) : 0 );
    }


    void create_currency( name contract, name manager, asset maxsupply ) {
        auto act =  mutable_variant_object()
                ("issuer",       manager )
                ("maximum_supply", maxsupply )
                ("can_freeze", 0)
                ("can_recall", 0)
                ("can_whitelist", 0);

        base_tester::push_action(contract, N(create), contract, act );
    }

    void issue( name contract, name manager, name to, asset amount ) {
       base_tester::push_action( contract, N(issue), manager, mutable_variant_object()
                ("to",      to )
                ("quantity", amount )
                ("memo", "")
        );
    }



    action_result stake(const account_name& from, const account_name& to, const string& net, const string& cpu, const string& storage ) {
        return push_action( name(from), name(from), N(delegatebw), mvo()
                ("from",     from)
                ("receiver", to)
                ("stake_net", net)
                ("stake_cpu", cpu)
                ("stake_storage", storage)
        );
    }
#if _READY
    fc::variant get_total_stake( const account_name& act )
    {
        vector<char> data = get_row_by_account( config::system_account_name, act, N(totalband), act );
        return data.empty() ? fc::variant() : abi_ser.binary_to_variant( "total_resources", data );
    }
#endif

    action_result stake( const account_name& acnt, const string& net, const string& cpu, const string& storage ) {
        return stake( acnt, acnt, net, cpu, storage );
    }

    asset get_balance( const account_name& act )
    {
         return get_currency_balance(N(ultrainio.token), symbol(SY(4,ULTRAIN)), act);
    }

    action_result regproducer( const account_name& acnt, int params_fixture = 1 ) {
        return push_action( acnt, acnt, N(regproducer), mvo()
                ("producer",  name(acnt).to_string() )
                ("producer_key", fc::raw::pack( get_public_key( acnt, "active" ) ) )
                ("prefs", producer_parameters_example( params_fixture ) )
        );
    }

    void set_code_abi(const account_name& account, const char* wast, const char* abi)
    {
       wdump((account));
        set_code(account, wast);
        set_abi(account, abi);
        produce_blocks();
    }


    abi_serializer abi_ser;
};

BOOST_AUTO_TEST_SUITE(bootseq_tests)

BOOST_FIXTURE_TEST_CASE( bootseq_test, bootseq_tester ) {
    try {

        // Create the following accounts:
        //  ultrainio.msig
        //  ultrainio.token
        create_accounts({N(ultrainio.msig), N(ultrainio.token)});

        // Set code for the following accounts:
        //  ultrainio.system  (code: ultrainio.bios)
        //  ultrainio.msig (code: ultrainio.msig)
        //  ultrainio.token    (code: ultrainio.token)
// These contracts are still under dev
#if _READY
        set_code_abi(N(ultrainio.msig), ultrainio_msig_wast, ultrainio_msig_abi);
#endif
//        set_code_abi(config::system_account_name, ultrainio_bios_wast, ultrainio_bios_abi);
        set_code_abi(N(ultrainio.token), ultrainio_token_wast, ultrainio_token_abi);

        ilog(".");
        // Set privileges for ultrainio.msig
        auto trace = base_tester::push_action(config::system_account_name, N(setpriv), 
                                              config::system_account_name,  mutable_variant_object()
                ("account", "ultrainio.msig")
                ("is_priv", 1)
        );

        ilog(".");
        // Todo : how to check the privilege is set? (use is_priv action)


        auto expected = asset::from_string("1000000000.0000 UTR");
        // Create ULTRAIN tokens in ultrainio.token, set its manager as ultrainio.system
        create_currency(N(ultrainio.token), config::system_account_name, expected);

        ilog(".");

        // Issue the genesis supply of 1 billion ULTRAIN tokens to ultrainio.system
        // Issue the genesis supply of 1 billion ULTRAIN tokens to ultrainio.system
        issue(N(ultrainio.token), config::system_account_name, config::system_account_name, expected); 

        ilog(".");

        auto actual = get_balance(config::system_account_name);
        BOOST_REQUIRE_EQUAL(expected, actual);
        ilog(".");

        // Create a few genesis accounts
        std::vector<account_name> gen_accounts{N(inita), N(initb), N(initc)};
        ilog(".");
        create_accounts(gen_accounts);
        ilog(".");
        // Transfer ULTRAIN to genesis accounts
        for (auto gen_acc : gen_accounts) {
            auto quantity = "10000.0000 UTR";
            auto stake_quantity = "5000.0000 UTR";

            ilog(".");
            auto trace = base_tester::push_action(N(ultrainio.token), N(transfer), config::system_account_name, mutable_variant_object()
                    ("from", name(config::system_account_name))
                    ("to", gen_acc)
                    ("quantity", quantity)
                    ("memo", gen_acc)
            );
            ilog( "." );

            auto balance = get_balance(gen_acc);
            BOOST_REQUIRE_EQUAL(asset::from_string(quantity), balance);
#if _READY
            // Stake 50% of balance to CPU and other 50% to bandwidth
            BOOST_REQUIRE_EQUAL(success(),
                                stake(config::system_account_name, gen_acc, stake_quantity, stake_quantity, ""));
            auto total = get_total_stake(gen_acc);
            BOOST_REQUIRE_EQUAL(asset::from_string(stake_quantity).amount, total["net_weight"].as_uint64());
            BOOST_REQUIRE_EQUAL(asset::from_string(stake_quantity).amount, total["cpu_weight"].as_uint64());
#endif
        }
        ilog(".");

        // Set code ultrainio.system from ultrainio.bios to ultrainio.system
        set_code_abi(config::system_account_name, ultrainio_system_wast, ultrainio_system_abi);

        ilog(".");
        // Register these genesis accts as producer account
        for (auto gen_acc : gen_accounts) {
        //    BOOST_REQUIRE_EQUAL(success(), regproducer(gen_acc));
        }
    } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()
