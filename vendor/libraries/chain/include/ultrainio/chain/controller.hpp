#pragma once
#include <ultrainio/chain/block_state.hpp>
#include <ultrainio/chain/trace.hpp>
#include <ultrainio/chain/genesis_state.hpp>
#include <boost/signals2/signal.hpp>

#include <ultrainio/chain/abi_serializer.hpp>
#include <ultrainio/chain/account_object.hpp>
#include <fc/network/url.hpp>
#include <ultrainio/chain/worldstate.hpp>

namespace chainbase {
   class database;
}


namespace ultrainio { namespace chain {

   class authorization_manager;

   namespace resource_limits {
      class resource_limits_manager;
   };

   struct controller_impl;
   using chainbase::database;
   using boost::signals2::signal;

   class dynamic_global_property_object;
   class global_property_object;
   class permission_object;
   class account_object;
   using resource_limits::resource_limits_manager;
   using apply_handler = std::function<void(apply_context&)>;

   class fork_database;

   enum class db_read_mode {
      SPECULATIVE,
      HEAD,
      IRREVERSIBLE
   };

   class controller {
      public:

         struct config {
            flat_set<account_name>   actor_whitelist;
            flat_set<account_name>   actor_blacklist;
            flat_set<account_name>   contract_whitelist;
            flat_set<account_name>   contract_blacklist;
            flat_set< pair<account_name, action_name> > action_blacklist;
            flat_set<public_key_type> key_blacklist;
            path                     blocks_dir             =  chain::config::default_blocks_dir_name;
            path                     state_dir              =  chain::config::default_state_dir_name;
            uint64_t                 state_size             =  chain::config::default_state_size;
            uint64_t                 state_guard_size       =  chain::config::default_state_guard_size;
            path                     worldstate_dir         =  chain::config::default_worldstate_dir_name;
            uint64_t                 worldstate_interval    =  chain::config::default_worldstate_interval;
            bool                     worldstate_control     =  false;
            bool                     read_only              =  false;
            bool                     force_all_checks       =  false;
            bool                     contracts_console      =  false;

            genesis_state            genesis;
            wasm_interface::vm_type  wasm_runtime = chain::config::default_wasm_runtime;

            db_read_mode             read_mode    = db_read_mode::SPECULATIVE;

            flat_set<account_name>   resource_greylist;

            #ifdef ULTRAIN_CONFIG_CONTRACT_PARAMS
            uint64_t                 contract_return_length  = chain::config::default_contract_return_length;
            uint64_t                 contract_emit_length    = chain::config::default_contract_emit_length;
            #endif
         };

         enum class block_status {
            irreversible = 0, ///< this block has already been applied before by this node and is considered irreversible
            validated   = 1, ///< this is a complete block signed by a valid producer and has been previously applied by this node and therefore validated but it is not yet irreversible
            complete   = 2, ///< this is a complete block signed by a valid producer but is not yet irreversible nor has it yet been applied by this node
            incomplete  = 3, ///< this is an incomplete block (either being produced by a producer or speculatively produced by a node)
         };

         controller( const config& cfg );
         ~controller();

         void add_indices();
         void startup(const worldstate_reader_ptr& worldstate = nullptr );

         /**
          * Starts a new pending block session upon which new transactions can
          * be pushed.
          */
         void start_block( block_timestamp_type time, chain::checksum256_type);

         void abort_block();
         void set_emit_signal();
         void clear_emit_signal();
         void enable_worldstate_creation();
         void disable_worldstate_creation();

         /**
          *  These transactions were previously pushed by have since been unapplied, recalling push_transaction
          *  with the transaction_metadata_ptr will remove them from the source of this data IFF it succeeds.
          *
          *  The caller is responsible for calling drop_unapplied_transaction on a failing transaction that
          *  they never intend to retry
          *
          *  @return vector of transactions which have been unapplied
          */
         vector<transaction_metadata_ptr> get_unapplied_transactions();
         std::list<transaction_metadata_ptr>* get_pending_transactions();
         void drop_pending_transaction_from_set(const transaction_metadata_ptr& trx);
         void clear_unapplied_transaction();
         void drop_unapplied_transaction(const transaction_metadata_ptr& trx);
         // returns if (overflow, duplicate)
         std::pair<bool, bool> push_into_pending_transaction(const transaction_metadata_ptr& trx);

         /**
          * These transaction IDs represent transactions available in the head chain state as scheduled
          * or otherwise generated transactions.
          *
          * calling push_scheduled_transaction with these IDs will remove the associated transaction from
          * the chain state IFF it succeeds or objectively fails
          *
          * @return
          */
         vector<transaction_id_type> get_scheduled_transactions();

         /**
          *
          */
         transaction_trace_ptr push_transaction( transaction_metadata_ptr& trx, fc::time_point deadline, uint32_t billed_cpu_time_us = 0 );

         /**
          * Attempt to execute a specific transaction in our deferred trx database
          *
          */
         transaction_trace_ptr push_scheduled_transaction( const transaction_id_type& scheduled, fc::time_point deadline, uint32_t billed_cpu_time_us = 0 );

         void finalize_block();
         void assign_header_to_block();
         void commit_block();

         void push_block( const signed_block_ptr& b, block_status s = block_status::complete );

         void register_event(const std::string& account, const std::string& post_url);
         void unregister_event(const std::string& account, const std::string& post_url);
         bool check_event_listener(account_name account);
         void push_event(account_name act_name, transaction_id_type id, const char* event_name, size_t event_name_size,
                         const char* msg, size_t msg_size );
         void notify_event();
         void start_receive_event();
         void stop_receive_event();

         chainbase::database& db()const;

         fork_database& fork_db()const;

         const account_object&                 get_account( account_name n )const;
         const global_property_object&         get_global_properties()const;
         const dynamic_global_property_object& get_dynamic_global_properties()const;
         const permission_object&              get_permission( const permission_level& level )const;
         const resource_limits_manager&        get_resource_limits_manager()const;
         resource_limits_manager&              get_mutable_resource_limits_manager();
         const authorization_manager&          get_authorization_manager()const;
         authorization_manager&                get_mutable_authorization_manager();

         uint32_t             head_block_num()const;
         time_point           head_block_time()const;
         block_id_type        head_block_id()const;
         account_name         head_block_proposer()const;
         const block_header&  head_block_header()const;
         block_state_ptr      head_block_state()const;

         uint32_t             fork_db_head_block_num()const;
         block_id_type        fork_db_head_block_id()const;
         time_point           fork_db_head_block_time()const;
         account_name         fork_db_head_block_proposer()const;

         time_point      pending_block_time()const;
         block_state_ptr pending_block_state()const;
         uint32_t block_interval_seconds()const;

         block_timestamp_type get_proper_next_block_timestamp() const;
         // This is a hack ...
         block_state_ptr pending_block_state_hack();
         void set_action_merkle_hack();
         void set_trx_merkle_hack();
         void set_header_extensions(const extensions_type&);
         void add_header_extensions_entry(uint16_t key, const vector<char>& value);

         uint32_t last_irreversible_block_num() const;
         block_id_type last_irreversible_block_id() const;

         signed_block_ptr fetch_block_by_number( uint32_t block_num )const;
         signed_block_ptr fetch_block_by_id( block_id_type id )const;

         block_state_ptr fetch_block_state_by_number( uint32_t block_num )const;
         block_state_ptr fetch_block_state_by_id( block_id_type id )const;

         block_id_type get_block_id_for_num( uint32_t block_num )const;
         void write_worldstate()const;
         void read_worldstate( const worldstate_reader_ptr& worldstate );
         sha256 calculate_integrity_hash()const;

         void check_actor_list( const flat_set<account_name>& actors )const;
         void check_contract_list( account_name code )const;
         void check_action_list( account_name code, action_name action )const;
         void check_key_list( const public_key_type& key )const;
         bool is_producing_block()const;

         void add_resource_greylist(const account_name &name);
         void remove_resource_greylist(const account_name &name);
         bool is_resource_greylisted(const account_name &name) const;
         const flat_set<account_name> &get_resource_greylist() const;

         void validate_referenced_accounts( const transaction& t )const;
         void validate_expiration( const transaction& t )const;
         void validate_tapos( const transaction& t )const;
         void validate_db_available_size() const;
         void validate_reversible_available_size() const;

         bool is_known_unexpired_transaction( const transaction_id_type& id) const;

         bool skip_auth_check()const;

         bool contracts_console()const;

         chain_id_type get_chain_id()const;
         db_read_mode get_read_mode()const;

         void enable_event_register(bool v);

         #ifdef ULTRAIN_CONFIG_CONTRACT_PARAMS
         uint64_t get_contract_return_length() const;
         uint64_t get_contract_emit_length() const;
         #endif


         vector<digest_type> merkle_proof_of(const uint32_t& block_number, const digest_type& trx_id, std::vector<char>& trx_receipt_bytes) const;
         bool verify_merkle_proof(const vector<digest_type>& merkle_proof, const digest_type& transaction_mroot, const std::vector<char>& trx_receipt_bytes) const;

         void set_subjective_cpu_leeway(fc::microseconds leeway);

         signal<void(const signed_block_ptr&)>         pre_accepted_block;
         signal<void(const block_state_ptr&)>          accepted_block_header;
         signal<void(const block_state_ptr&)>          accepted_block;
         signal<void(const block_state_ptr&)>          irreversible_block;
         signal<void(const transaction_metadata_ptr&)> accepted_transaction;
         signal<void(const transaction_trace_ptr&)>    applied_transaction;
         signal<void(const int&)>                      bad_alloc;
         signal<bool(const url& dest, const variant& payload, const time_point&)> http_async_post;

         /*
         signal<void()>                                  pre_apply_block;
         signal<void()>                                  post_apply_block;
         signal<void()>                                  abort_apply_block;
         signal<void(const transaction_metadata_ptr&)>   pre_apply_transaction;
         signal<void(const transaction_trace_ptr&)>      post_apply_transaction;
         signal<void(const transaction_trace_ptr&)>  pre_apply_action;
         signal<void(const transaction_trace_ptr&)>  post_apply_action;
         */

         const apply_handler* find_apply_handler( account_name contract, scope_name scope, action_name act )const;
         wasm_interface& get_wasm_interface();


         optional<abi_serializer> get_abi_serializer( account_name n, const fc::microseconds& max_serialization_time )const {
            if( n.good() ) {
               try {
                  const auto& a = get_account( n );
                  abi_def abi;
                  if( abi_serializer::to_abi( a.abi, abi ))
                     return abi_serializer( abi, max_serialization_time );
               } FC_CAPTURE_AND_LOG((n))
            }
            return optional<abi_serializer>();
         }

         template<typename T>
         fc::variant to_variant_with_abi( const T& obj, const fc::microseconds& max_serialization_time ) {
            fc::variant pretty_output;
            abi_serializer::to_variant( obj, pretty_output,
                                        [&]( account_name n ){ return get_abi_serializer( n, max_serialization_time ); },
                                        max_serialization_time);
            return pretty_output;
         }

      private:

         std::unique_ptr<controller_impl> my;

   };

} }  /// ultrainio::chain

FC_REFLECT( ultrainio::chain::controller::config,
            (actor_whitelist)
            (actor_blacklist)
            (contract_whitelist)
            (contract_blacklist)
            (blocks_dir)
            (state_dir)
            (state_size)
            (worldstate_dir)
            (worldstate_control)
            (read_only)
            (force_all_checks)
            (contracts_console)
            (genesis)
            (wasm_runtime)
            (resource_greylist)
            #ifdef ULTRAIN_CONFIG_CONTRACT_PARAMS
            (contract_return_length)
            (contract_emit_length)
            #endif
          )
