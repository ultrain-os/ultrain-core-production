#include <ultrainio/chain/controller.hpp>
#include <ultrainio/chain/transaction_context.hpp>

#include <ultrainio/chain/block_log.hpp>
#include <ultrainio/chain/fork_database.hpp>

#include <ultrainio/chain/name.hpp>
#include <ultrainio/chain/config.hpp>

#include <ultrainio/chain/account_object.hpp>
#include <ultrainio/chain/block_summary_object.hpp>
#include <ultrainio/chain/global_property_object.hpp>
#include <ultrainio/chain/contract_table_objects.hpp>
#include <ultrainio/chain/generated_transaction_object.hpp>
#include <ultrainio/chain/transaction_object.hpp>
#include <ultrainio/chain/whiteblacklist_object.hpp>
#include <ultrainio/chain/callback.hpp>
#include <ultrainio/chain/callback_manager.hpp>

#include <ultrainio/chain/database_utils.hpp>
#include <ultrainio/chain/authorization_manager.hpp>
#include <ultrainio/chain/resource_limits.hpp>
#include <ultrainio/chain/bls_votes.hpp>
#include <ultrainio/chain/chain_worldstate.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>

#include <chainbase/chainbase.hpp>
#include <fc/io/json.hpp>
#include <fc/scoped_exit.hpp>
#include <fc/crypto/rand.hpp>

#include <ultrainio/chain/ultrainio_contract.hpp>
#include <appbase/application.hpp>
#include <fc/network/url.hpp>
#include <fc/network/http/http_client.hpp>
#include <chrono>

#include <ultrainio/chain/worldstate_file_manager.hpp>
#include <appbase/application.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
namespace ultrainio { namespace chain {

using namespace appbase;
using resource_limits::resource_limits_manager;
using bls_votes::bls_votes_manager;

using controller_index_set = index_set<
   account_index,
   account_sequence_index,
   global_property_multi_index,
   dynamic_global_property_multi_index,
   block_summary_multi_index,
   transaction_multi_index,
   generated_transaction_multi_index,
   table_id_multi_index,
   auth_sequence_index,
   whiteblacklist_index
>;

using contract_database_index_set = index_set<
   key_value_index,
   index64_index,
   index128_index,
   index_double_index,
   index_long_double_index,
   index256_index
>;

#define WS_INTERVAL 100

// these are the default numbers, and it could be changed during startup.
//int chain::config::block_interval_ms = 10*1000;
//int chain::config::block_interval_us = 10*1000*1000;
struct pending_state {
   pending_state( database::session&& s )
   :_db_session( move(s) ){}

   database::session                  _db_session;

   block_state_ptr                    _pending_block_state;

   vector<action_receipt>             _actions;

   controller::block_status           _block_status = controller::block_status::incomplete;

   void push(bool ws =false) {
      _db_session.push(ws);
   }
};

struct controller_impl {
   controller&                    self;
   chainbase::database            db;
   block_log                      blog;
   optional<pending_state>        pending;
   block_state_ptr                head;
   bool                           emit_signal = false;
   bool                           worldstate_allowed = false;
   fork_database                  fork_db;
   wasm_interface                 wasmif;
   resource_limits_manager        resource_limits;
   authorization_manager          authorization;
   bls_votes_manager              bls_votes;
   controller::config             conf;
   chain_id_type                  chain_id;
   bool                           replaying = false;
   db_read_mode                   read_mode = db_read_mode::SPECULATIVE;
   bool                           in_trx_requiring_checks = false; ///< if true, checks that are normally skipped on replay (e.g. auth checks) cannot be skipped
   optional<fc::microseconds>     subjective_cpu_leeway;

   bool                           can_accept_event_register = true;
   bool                           can_receive_event = false;
   const uint32_t                 event_lifetime = 20; ///< 20 blocks' time, which = 10 seconds by default configuration
   uint32_t                       worldstate_head_block = 0;
   uint32_t                       worldstate_previous_block = 0;
   bool                           worldstate_thread_running = false;
   bool                           worldstate_data_processing = false;
   boost::thread                  worldstate_thread;
   typedef pair<scope_name,action_name>                   handler_key;
   map< account_name, map<handler_key, apply_handler> >   apply_handlers;
   std::shared_ptr<ws_file_manager>                       ws_manager_ptr;

   map< account_name, std::list<fc::url> > registered_event_map;
   struct contract_event_type {
      account_name name;
      transaction_id_type id;
      std::string event_name;
      std::string message;
      uint32_t head_block_num;
      bool notified;

      contract_event_type(account_name name_in, transaction_id_type id_in, const std::string& event_name_in,
                          const std::string& msg_in, uint32_t head_block_num_in)
      {
        name = name_in;
        id = id_in;
        event_name = event_name_in;
        message = msg_in;
        head_block_num = head_block_num_in;
        notified = false;
      }
   };

   // BUG!!! TODO: We need save event list to database to avoid duplicate when restarts system
   std::list<contract_event_type> event_list;

   struct whiteblack_type {
       bip::set<account_name>   actor_whitelist;
       bip::set<account_name>   actor_blacklist;
       bip::set<account_name>   contract_whitelist;
       bip::set<account_name>   contract_blacklist;
       bip::set< pair<account_name, action_name> > action_blacklist;
       bip::set<public_key_type> key_blacklist;
       whiteblack_type()=default;
       whiteblack_type(const shared_set<account_name>& actor_w, const shared_set<account_name>& actor_b,
               const shared_set<account_name>& contract_w, const shared_set<account_name>& contract_b,
               const shared_set< pair<account_name, action_name> >& action_b, const shared_set<public_key_type>& key_b):
           actor_whitelist(actor_w.begin(),actor_w.end()),
           actor_blacklist(actor_b.begin(),actor_b.end()),
           contract_whitelist(contract_w.begin(),contract_w.end()),
           contract_blacklist(contract_b.begin(),contract_b.end()),
           action_blacklist(action_b.begin(),action_b.end()),
           key_blacklist(key_b.begin(),key_b.end())
       {}
   };

   whiteblack_type whiteblack_list;

   /**
    *  Transactions that were undone by pop_block or abort_block, transactions
    *  are removed from this list if they are re-applied in other blocks. Producers
    *  can query this list when scheduling new transactions into blocks.
    */

    map<digest_type, transaction_metadata_ptr>     unapplied_transactions;
    std::list<transaction_metadata_ptr>    pending_transactions;
    set<digest_type>     pending_transactions_set;

   void set_apply_handler( account_name receiver, account_name contract, action_name action, apply_handler v ) {
      apply_handlers[receiver][make_pair(contract,action)] = v;
   }

   controller_impl( const controller::config& cfg, controller& s  )
   :self(s),
    db( cfg.state_dir,
        cfg.read_only ? database::read_only : database::read_write,
        cfg.state_size,
        cfg.worldstate_control),
    blog( cfg.blocks_dir ),
    fork_db( cfg.state_dir ),
    wasmif( cfg.wasm_runtime ),
    resource_limits( db ),
    authorization( s, db ),
    bls_votes( db ),
    conf( cfg ),
    chain_id( cfg.genesis.compute_chain_id() ),
    read_mode( cfg.read_mode )
   {
#define SET_APP_HANDLER( receiver, contract, action) \
   set_apply_handler( #receiver, #contract, #action, &BOOST_PP_CAT(apply_, BOOST_PP_CAT(contract, BOOST_PP_CAT(_,action) ) ) )

    ilog("genesis ${block_cpu}",("block_cpu",cfg.genesis.initial_configuration.max_block_cpu_usage));
   SET_APP_HANDLER( ultrainio, ultrainio, newaccount );
   SET_APP_HANDLER( ultrainio, ultrainio, setcode );
   SET_APP_HANDLER( ultrainio, ultrainio, setabi );
   SET_APP_HANDLER( ultrainio, ultrainio, updateauth );
   SET_APP_HANDLER( ultrainio, ultrainio, deleteauth );
   SET_APP_HANDLER( ultrainio, ultrainio, linkauth );
   SET_APP_HANDLER( ultrainio, ultrainio, unlinkauth );
   SET_APP_HANDLER( ultrainio, ultrainio, delaccount );

/*
   SET_APP_HANDLER( ultrainio, ultrainio, postrecovery );
   SET_APP_HANDLER( ultrainio, ultrainio, passrecovery );
   SET_APP_HANDLER( ultrainio, ultrainio, vetorecovery );
*/

   SET_APP_HANDLER( ultrainio, ultrainio, canceldelay );
   SET_APP_HANDLER( ultrainio, ultrainio, addwhiteblack );
   SET_APP_HANDLER( ultrainio, ultrainio, rmwhiteblack );

   fork_db.irreversible.connect( [&]( auto b ) {
                                 on_irreversible(b);
                                 });

   ws_manager_ptr = std::make_shared<ws_file_manager>();

   if (db.get_data("worldstate_previous_block", worldstate_previous_block))
      ilog("Get worldstate_previous_block from  db: ${s}", ("s", worldstate_previous_block));
   }

   /**
    *  Plugins / observers listening to signals emited (such as accepted_transaction) might trigger
    *  errors and throw exceptions. Unless those exceptions are caught it could impact consensus and/or
    *  cause a node to fork.
    *
    *  If it is ever desirable to let a signal handler bubble an exception out of this method
    *  a full audit of its uses needs to be undertaken.
    *
    */
   template<typename Signal, typename Arg>
   void emit( const Signal& s, Arg&& a ) {
      try {
        s(std::forward<Arg>(a));
      } catch (boost::interprocess::bad_alloc& e) {
         wlog( "bad alloc" );
         throw e;
      } catch ( controller_emit_signal_exception& e ) {
         wlog( "${details}", ("details", e.to_detail_string()) );
         throw e;
      } catch ( fc::exception& e ) {
         wlog( "${details}", ("details", e.to_detail_string()) );
      } catch ( ... ) {
         wlog( "signal handler threw exception" );
      }
   }
   void create_fake_ws(uint32_t block_height){
      try {
         ws_info info;
         info.chain_id = self.get_chain_id();
         info.block_height = block_height;

         std::string new_ws_file = ws_manager_ptr->get_file_path_by_info(info.chain_id, info.block_height);
         std::string new_ws_file_name = new_ws_file + ".ws";
         auto fd = std::ofstream(new_ws_file_name, (std::ios::out | std::ios::binary));

         std::string msg = "Fake ws, blk_height:" + std::to_string(block_height) + "\n";

         //Add random number, so that diff node could create diff hash value of fake ws.
         fc::sha256  random_id;
         fc::rand_pseudo_bytes( random_id.data(), random_id.data_size());
         msg += "Random(for different hash): " + fc::json::to_string(random_id);

         fd.write(msg.c_str(), msg.size());
         fd.close();

         info.file_size = fc::file_size(new_ws_file_name);
         info.hash_string = ws_manager_ptr->calculate_file_hash(new_ws_file_name).str();
         ws_manager_ptr->save_info(info);
         elog("create a fake ws, blk_height: ${b}", ("b", block_height));
      } catch( ... ) {
         elog("create_fake_ws error: unknown exception");
      }
   }

   void create_worldstate(){
      auto begin = fc::time_point::now();
      block_state fork_head = *fork_db.head();
      uint32_t block_height = fork_head.block_num;
      if ( worldstate_data_processing ){
         /*In this case, error happend in previous ws thread, and the cache data have been modify, data was incomplete.
         We can't handle this error now, return*/
         elog("ws error, cache data was incomplete!");
         return;
      }

      //thread to generate worldstate file
      worldstate_thread = boost::thread([this,fork_head,block_height](){
         auto begin = fc::time_point::now();
         bool is_error = true;
         try {
            add_to_worldstate(db, block_height, fork_head);
            is_error = false;
         } catch( const boost::interprocess::bad_alloc& e ) {
            elog("worldstate thread error: bad alloc");
         } catch( const boost::exception& e ) {
            elog("worldstate thread error: ${e}", ("e",boost::diagnostic_information(e)));
         } catch( const std::runtime_error& e ) {
            elog( "worldstate thread error: ${e}", ("e",e.what()));
         } catch( const std::exception& e ) {
            elog("worldstate thread error: ${e}", ("e",e.what()));
         } catch (const fc::exception &e) {
            elog("worldstate thread error:");
            edump((e.to_detail_string()));
         } catch( ... ) {
            elog("worldstate thread error: unknown exception");
         }

         if (is_error && worldstate_data_processing){
            /*In this case, error happen, and the cache data have been modify, data was incomplete. We can't handle this error now,
            so we make some log, and create a fake ws file, so it will report to monitor, we could noticed the error earlier*/
            create_fake_ws(block_height);
            elog("add_to_worldstate error, cache data was incomplete, blk_heigh: ${b}", ("b", block_height));
         } else {
            worldstate_data_processing = false;
         }

         worldstate_thread_running = false;
         auto end = fc::time_point::now();
         ilog("worldstate thread end, cost time: ${time}", ("time", end - begin));
      });
      worldstate_thread_running = true;

      auto end=fc::time_point::now();
      ilog("start world state thread end, cost time: ${time_delta}", ("time_delta", end - begin));
   }

   void on_irreversible( const block_state_ptr& s ) {
      if( !blog.head() )
         blog.read_head();
      const auto& log_head = blog.head();
      bool append_to_blog = false;
      if (!log_head) {
         if (s->block) {
            //after restore from worldstate, the block is set with the header info, but we do not need save it to blog
            if(s->block_num >= blog.first_block_num()) {
               append_to_blog = true;
	    }
         } else {
            ULTRAIN_ASSERT(s->block_num == blog.first_block_num() - 1, block_log_exception, "block log has no blocks and is not properly set up to start after the worldstate");
         }
      } else {
         auto lh_block_num = log_head->block_num();
         if (s->block_num > lh_block_num) {
            ULTRAIN_ASSERT(s->block_num - 1 == lh_block_num, unlinkable_block_exception, "unlinkable block", ("s->block_num", s->block_num)("lh_block_num", lh_block_num));
            ULTRAIN_ASSERT(s->block->previous == log_head->id(), unlinkable_block_exception, "irreversible doesn't link to block log head");
            append_to_blog = true;
         }
      }

      db.commit( s->block_num );

      if( append_to_blog ) {
         blog.append(s->block);
      }

      // the "head" block when a worldstate is loaded is virtual and has no block data, all of its effects
      // should already have been loaded from the worldstate so, it cannot be applied
      if (s->block) {
         if (read_mode == db_read_mode::IRREVERSIBLE) {
            // when applying a worldstate, head may not be present
            // when not applying a worldstate, make sure this is the next block
            if (!head || s->block_num == head->block_num + 1) {
               apply_block(s->block, controller::block_status::complete);
               head = s;
            } else {
               // otherwise, assert the one odd case where initializing a chain
               // from genesis creates and applies the first block automatically.
               // when syncing from another chain, this is pushed in again
               ULTRAIN_ASSERT(!head || head->block_num == 1, block_validate_exception, "Attempting to re-apply an irreversible block that was not the implied genesis block");
            }

            fork_db.mark_in_current_chain(head, true);
            fork_db.set_validity(head, true);
         }
         emit(self.irreversible_block, s);
      }
   }

   void replay()
   {
      auto head_num  = head->block_num;
      auto blog_head = blog.read_head();
      replaying = true;
      ilog( "existing block log, attempting to replay ${n} blocks: #${sn} - #${en}",
            ("n", blog_head->block_num()-head_num) ("sn", head_num+1)("en", blog_head->block_num()));

      std::shared_ptr<callback> cb = callback_manager::get_self()->get_callback();
      auto start = fc::time_point::now();
      emit_signal = true;
      while( auto next = blog.read_block_by_num( head->block_num + 1 ) ) {
         self.push_block( next, controller::block_status::irreversible );
         if( next->block_num() % 100 == 0 ) {
            std::cerr << std::setw(10) << next->block_num() << " of " << blog_head->block_num() <<"\r";
         }
         cb->on_replay_block(*(next.get()));
      }
      emit_signal = false;

      auto end = fc::time_point::now();
      ilog( "replayed ${n} blocks in ${duration} seconds, ${mspb} ms/block",
         ("n", head->block_num-head_num)("duration", (end-start).count()/1000000)
         ("mspb", ((end-start).count()/1000.0)/(head->block_num-head_num) ) );
      std::cerr<< "\n";
      replaying = false;
   }

   template<typename MultiIndexType>
   void standalone_object(){
       if(0 == db.get_index<MultiIndexType>().indices().size() )
           db.create<typename MultiIndexType::value_type>([](auto&){});
   }

   void standalone_object(){
       standalone_object<whiteblacklist_index>();
   }
   void init(const bfs::path& worldstate_path) {

      /**
      *  The fork database needs an initial block_state to be set before
      *  it can accept any new blocks. This initial block state can be found
      *  in the database (whose head block state should be irreversible) or
      *  it would be the genesis state.
      */
      if (bfs::exists(worldstate_path)) {
         ULTRAIN_ASSERT(!head, fork_database_exception, "");
         read_from_worldstate(worldstate_path);
         standalone_object();

         auto end = blog.read_head();
         if( !end ) {
            blog.reset_to_genesis(conf.genesis, signed_block_ptr(), head->block_num + 1);
         } else if ( end->block_num() > head->block_num) {
            replay();
         } else {
            ULTRAIN_ASSERT(end->block_num() == head->block_num, fork_database_exception,
                       "Block log is provided with worldstate but does not contain the head block from the worldstate");
         }
      } else if( !head ) {
         initialize_fork_db(); // set head to genesis state

         auto end = blog.read_head();
         if( end && end->block_num() > 1 ) {
            replay();
         } else if( !end ) {
            blog.reset_to_genesis( conf.genesis, head->block );
         }
      }

      ULTRAIN_ASSERT( db.revision() >= head->block_num, fork_database_exception, "fork database is inconsistent with shared memory",
                 ("db",db.revision())("head",head->block_num) );

      if( db.revision() > head->block_num ) {
         wlog( "warning: database revision (${db}) is greater than head block number (${head}), "
               "attempting to undo pending changes",
               ("db",db.revision())("head",head->block_num) );
      }
      while( db.revision() > head->block_num ) {
         db.undo();
      }

      standalone_object();
   }

   ~controller_impl() {
      if(worldstate_thread_running && worldstate_thread.joinable()) {
         ilog("Worldstate thread is runging, waiting!!!");
         worldstate_thread.join();
      }

      db.save_data("worldstate_previous_block", worldstate_previous_block);

      pending.reset();

      db.flush();
   }

   void add_indices() {
      controller_index_set::add_indices(db);
      contract_database_index_set::add_indices(db);

      authorization.add_indices(db);
      resource_limits.add_indices(db);
      bls_votes.add_indices(db);
   }

   bool restore_contract_database_index(chainbase::database& worldstate_db, worldstate_reader::section_reader& section_reader,
               table_id_object::id_type& t_id, std::shared_ptr<ws_helper> ws_helper_ptr){
      bool more = false;
      bool id_more = false;
      ilog("restore_contract_database_index");
      contract_database_index_set::walk_indices([&](auto utils) {
         using utils_t = decltype(utils);

         unsigned_int size;
         more = section_reader.read_row(size, db);
         // ilog("contract table size: ${t}", ("t", size ));

         /*Import, read a ignore value*/
         uint64_t ignore1 = 0, ignore2 = 0;
         ws_helper_ptr->get_id_reader()->read_id_row(ignore1, ignore2);

         for (size_t idx = 0; idx < size.value; idx++) {
            uint64_t old_id = 0, size = 0;
            id_more = ws_helper_ptr->get_id_reader()->read_id_row(old_id, size);
            // ilog("contract table old_id ${t}", ("t", old_id ));
            utils_t::create(db, [&](auto& row) {
               row.t_id = t_id;
               row.id._id = old_id;
               more = section_reader.read_row(row, db);
            }, true);
            ULTRAIN_ASSERT(more == id_more, worldstate_exception, "Restore to backup indices error: the ws data conflict ${x} ${t}", ("x", more)("t", id_more));
         }
      });

      return more;
   }

   void store_one_contract_table(chainbase::database& worldstate_db, worldstate_writer::section_writer& writer_section, std::shared_ptr<ws_helper> ws_helper_ptr){
      ilog("store_one_contract_table");
      int count = 0;
      index_utils<table_id_multi_index>::walk(worldstate_db, [&]( const table_id_object& table_row ){
         ws_helper_ptr->get_id_writer()->write_row_id(table_row.id._id, 0);
         // add a row for the table
         writer_section.add_row(table_row, worldstate_db);
         count++;

         // followed by a size row and then N data rows for each type of table
         contract_database_index_set::walk_indices([&]( auto utils ) {
            using utils_t = decltype(utils);
            using value_t = typename decltype(utils)::index_t::value_type;
            using by_table_id = object_to_table_id_tag_t<value_t>;

            auto tid_key = boost::make_tuple(table_row.id);
            auto next_tid_key = boost::make_tuple(table_id_object::id_type(table_row.id._id + 1));

            unsigned_int size = utils_t::template size_range<by_table_id>(worldstate_db, tid_key, next_tid_key);
            writer_section.add_row(size, worldstate_db);

            /*Import, insert a ignore value,keep same relative pos with ws file*/
            ws_helper_ptr->get_id_writer()->write_row_id(0, 0);

            utils_t::template walk_range<by_table_id>(worldstate_db, tid_key, next_tid_key, [&]( const auto &row ) {
               ws_helper_ptr->get_id_writer()->write_row_id(row.id._id, 0);
               writer_section.add_row(row, worldstate_db);
            });
         });
      });
      ULTRAIN_ASSERT(count <= 1, worldstate_exception, "The table_id count should not more than 1, because we handle only one every time!!");
   }

   void add_contract_tables_to_worldstate(std::shared_ptr<ws_helper> ws_helper_ptr, chainbase::database& worldstate_db) {
      ULTRAIN_ASSERT(ws_helper_ptr->get_writer() && ws_helper_ptr->get_id_writer(), worldstate_exception, "Ws writer is not exist!");
      ilog("add_contract_tables_to_worldstate");

      ws_helper_ptr->get_id_writer()->write_start_id_section("contract_tables");

      ws_helper_ptr->get_writer()->write_section("contract_tables", [&]( auto& writer_section ){
         if(ws_helper_ptr->get_id_reader() && ws_helper_ptr->get_reader()) {
            ilog("Exist old ws file");
            ws_helper_ptr->get_reader()->read_section("contract_tables", [&]( auto& reader_section ) {
               ws_helper_ptr->get_id_reader()->read_start_id_section("contract_tables");

               bool more = !reader_section.empty();
               bool id_more = !ws_helper_ptr->get_id_reader()->empty();
               ULTRAIN_ASSERT(more == id_more, worldstate_exception, "Restore to backup indices error: the ws data conflict ");

               table_id_object::id_type t_id;
               auto& cache_node = worldstate_db.get_mutable_index<table_id_multi_index>().cache().front();
               ilog("re-add_contract remove/modify/create size: ${s} ${t} ${y}", ("s", cache_node.removed_ids.size())("t", cache_node.modify_values.size())("y", cache_node.new_values.size()));
               ilog("re-add_contract Cache count: ${s}", ("s", worldstate_db.get_mutable_index<table_id_multi_index>().cache().size()));
               ilog("re-add_contract Backup size: ${s}", ("s", worldstate_db.get_mutable_index<table_id_multi_index>().backup_indices().size()));

               while (more) {
                  //1.read 1 table_id row from old ws file, insert to backup indices
                  uint64_t old_id = 0, size = 0;
                  id_more = ws_helper_ptr->get_id_reader()->read_id_row(old_id, size);
                  ilog("add table-id: ${x}", ("x", old_id));
                  index_utils<table_id_multi_index>::create(worldstate_db, [&](auto& row) {
                     row.id._id = old_id;
                     reader_section.read_row(row, db);
                     t_id = row.id;
                     ilog("t_id id ${t}", ("t",row.id._id ));
                  }, true);

                  //2.restore all contract_database_index_set rows from ws old file where its' table_id equal t_id
                  more = restore_contract_database_index(worldstate_db, reader_section, t_id, ws_helper_ptr);

                  //3.process table_id table and all contract_database_index_set
                  auto ret = worldstate_db.get_mutable_index<table_id_multi_index>().process_table();
                  ilog("process_table ret: ${t}", ("t", ret));

                  contract_database_index_set::walk_indices([&]( auto utils ) {
                     using index_t = typename decltype(utils)::index_t;
                     worldstate_db.get_mutable_index<index_t>().process_table([&](auto& item)->bool{
                        return item.second.t_id == t_id;
                     });
                  });
                  ilog("process_table done");

                  //4.store table_id and contract_database_index_set to ws new file
                  store_one_contract_table(worldstate_db, writer_section, ws_helper_ptr);

                  //5.handle done, clear table_id backup indices and contract_database_index_set
                  (const_cast<table_id_multi_index&>(worldstate_db.get_mutable_index<table_id_multi_index>().backup_indices())).clear();
                  contract_database_index_set::walk_indices([&]( auto utils ) {
                     using index_t = typename decltype(utils)::index_t;
                     (const_cast<index_t&>(worldstate_db.get_mutable_index<index_t>().backup_indices())).clear();
                  });
               }

               ws_helper_ptr->get_id_reader()->clear_id_section();
            });
         }

         auto& cache_node = worldstate_db.get_mutable_index<table_id_multi_index>().cache().front();
         do {
            ilog("add_contract remove/modify/create size: ${s} ${t} ${y}", ("s", cache_node.removed_ids.size())("t", cache_node.modify_values.size())("y", cache_node.new_values.size()));
            ilog("add_contract Cache count: ${s}", ("s", worldstate_db.get_mutable_index<table_id_multi_index>().cache().size()));
            ilog("add_contract Backup size: ${s}", ("s", worldstate_db.get_mutable_index<table_id_multi_index>().backup_indices().size()));

            //1.new create row,  insert to backup
            auto result_paire = worldstate_db.get_mutable_index<table_id_multi_index>().process_create();
            ilog("process_create ${x} ${t}", ("x", std::get<0>(result_paire))("t", std::get<1>(result_paire)));
            if(!std::get<0>(result_paire))
               break;

            auto t_id = std::get<1>(result_paire);

            //2. don't need  restore from old ws file, because no any contract_database record in old ws file

            //3.process all contract_database_index_set
            contract_database_index_set::walk_indices([&]( auto utils ) {
               using index_t = typename decltype(utils)::index_t;
               worldstate_db.get_mutable_index<index_t>().process_table([&](auto& item)->bool{
                  return item.second.t_id == t_id;
               });
            });

            //4.store table_id and contract_database_index_set to ws new file
            store_one_contract_table(worldstate_db, writer_section, ws_helper_ptr);

            //5.handle done, clear table_id backup indices and contract_database_index_set
            (const_cast<table_id_multi_index&>(worldstate_db.get_mutable_index<table_id_multi_index>().backup_indices())).clear();
            contract_database_index_set::walk_indices([&]( auto utils ) {
               using index_t = typename decltype(utils)::index_t;
               (const_cast<index_t&>(worldstate_db.get_mutable_index<index_t>().backup_indices())).clear();
            });
         } while(1);
      });
      ws_helper_ptr->get_id_writer()->write_end_id_section();

      worldstate_db.get_mutable_index<table_id_multi_index>().cache().pop_front();
      contract_database_index_set::walk_indices([&]( auto utils ) {
         using index_t = typename decltype(utils)::index_t;
         worldstate_db.get_mutable_index<index_t>().cache().pop_front();
      });
      ilog("add_contract_tables_to_worldstate end");
   }

   void read_contract_tables_from_worldstate( std::shared_ptr<ws_helper> ws_helper_ptr, chainbase::database& worldstate_db ) {
      ULTRAIN_ASSERT(ws_helper_ptr->get_reader(), worldstate_exception, "Ws reaer is not exist!");
      ilog("read_contract_tables_from_worldstate");

      ws_helper_ptr->get_id_writer()->write_start_id_section("contract_tables");
      ws_helper_ptr->get_reader()->read_section("contract_tables", [&]( auto& section ) {
         bool more = !section.empty();
         while (more) {
            // read the row for the table
            table_id_object::id_type t_id;
            index_utils<table_id_multi_index>::create(db, [&](auto& row) {
               ws_helper_ptr->get_id_writer()->write_row_id(row.id._id, 0);

               section.read_row(row, db);
               t_id = row.id;
            });

            // read the size and data rows for each type of table
            contract_database_index_set::walk_indices([&](auto utils) {
               using utils_t = decltype(utils);

               unsigned_int size;
               more = section.read_row(size, db);
               /*Import, insert a ignore value,keep same relative pos with ws file*/
               ws_helper_ptr->get_id_writer()->write_row_id(0, 0);

               for (size_t idx = 0; idx < size.value; idx++) {
                  utils_t::create(db, [&](auto& row) {
                     ws_helper_ptr->get_id_writer()->write_row_id(row.id._id, 0);

                     row.t_id = t_id;
                     more = section.read_row(row, db);
                  });
               }
            });
         }
      });
      ws_helper_ptr->get_id_writer()->write_end_id_section();
      ilog("read_contract_tables_from_worldstate end");
   }

   void add_to_worldstate(chainbase::database& worldstate_db, uint32_t block_height, const block_state& fork_head){
      ilog("start add_to_worldstate blockNum: ${t}",  ("t", block_height));
      if(!ws_manager_ptr) return;

      const static auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
      auto worldstate_interval = ro_api.get_worldstate_interval();
      if (worldstate_interval < WS_INTERVAL * 1) {
          worldstate_interval = WS_INTERVAL * 1;
      }

      ws_info info;
      info.chain_id = self.get_chain_id();
      info.block_height = block_height;

      std::string new_ws_file = ws_manager_ptr->get_file_path_by_info(info.chain_id, info.block_height);
      std::string old_ws_file = worldstate_previous_block > 0 ? ws_manager_ptr->get_file_path_by_info(info.chain_id, worldstate_previous_block) : "";
      if (block_height % worldstate_interval != 0)
         new_ws_file += ".tmp";

      if (worldstate_previous_block > 0 && fc::exists(old_ws_file + ".tmp.ws"))
         old_ws_file += ".tmp";

      if (worldstate_previous_block > 0) {
         ULTRAIN_ASSERT(fc::exists(old_ws_file + ".ws") && fc::exists(old_ws_file + ".id"), worldstate_exception, "Don't exist file: ${s} or ${t}",
            ("s", old_ws_file + ".ws")("t", old_ws_file + ".id"));
      }

      auto ws_helper_ptr = std::make_shared<ws_helper>(old_ws_file, new_ws_file);

      ws_helper_ptr->get_writer()->write_section<chain_worldstate_header>([this,&worldstate_db]( auto &section ){
         section.add_row(chain_worldstate_header(), worldstate_db);
      });

      ws_helper_ptr->get_writer()->write_section<genesis_state>([this,&worldstate_db]( auto &section ){
         section.add_row(conf.genesis, worldstate_db);
      });

      ws_helper_ptr->get_writer()->write_section<block_state>([this,&worldstate_db, fork_head]( auto &section ){
         section.template add_row<block_header_state>(fork_head, worldstate_db);
      });

      worldstate_data_processing = true;

      //How to create new ws file by old ws file and operation cache:
      //1.Read all old rows from old ws file, and then restore to backup indices;
      //2.Squach backup indices and cache,
      //3.store the backup indices records to new ws file
      controller_index_set::walk_indices([this,&worldstate_db, &ws_helper_ptr]( auto utils ){
         using index_t = typename decltype(utils)::index_t;
         using value_t = typename index_t::value_type;

         // skip the table_id_object as its inlined with contract tables section
         if (std::is_same<value_t, table_id_object>::value) {
            return;
         }

         ws_helper_ptr->add_table_to_worldstate<index_t>(worldstate_db);
      });

      add_contract_tables_to_worldstate(ws_helper_ptr,worldstate_db);
      authorization.add_to_worldstate(ws_helper_ptr, worldstate_db);
      resource_limits.add_to_worldstate(ws_helper_ptr, worldstate_db);
      bls_votes.add_to_worldstate(ws_helper_ptr, worldstate_db);

      //save worldstate file
      ws_helper_ptr.reset();
      std::string new_ws_file_name = new_ws_file + ".ws";
      info.file_size = fc::file_size(new_ws_file_name);
      info.hash_string = ws_manager_ptr->calculate_file_hash(new_ws_file_name).str();
      ws_manager_ptr->save_info(info);

      //Remove old ws files if temp
      if (worldstate_previous_block > 0 && worldstate_previous_block % worldstate_interval != 0) {
         fc::remove(ws_manager_ptr->get_file_path_by_info(info.chain_id, worldstate_previous_block)+".info");
         fc::remove(old_ws_file+".id");
         fc::remove(old_ws_file+".ws");
      }

      worldstate_previous_block = block_height;
      ilog("add_to_worldstate ws info: ${info}", ("info", info));
   }

   void read_from_worldstate( const bfs::path& worldstate_path ) {
      ilog("read_from_worldstate ${p}", ("p", worldstate_path.string()));
      ULTRAIN_ASSERT(ws_manager_ptr, worldstate_exception, "ws_manager_ptr is null!");

      auto& worldstate_db = db;
      fc::sha256 tmp_chain_id = self.get_chain_id();
      std::string id_file = ws_manager_ptr->get_file_path_by_info(tmp_chain_id, 0) + ".id";
      auto ws_helper_ptr = std::make_shared<ws_helper>(worldstate_path.string(), id_file, true);

      ws_helper_ptr->get_reader()->read_section<chain_worldstate_header>([this]( auto &section ){
         chain_worldstate_header header;
         section.read_row(header, db);
         header.validate();
      });

      ws_helper_ptr->get_reader()->read_section<block_state>([this]( auto &section ){
          block_header_state head_header_state;
          section.read_row(head_header_state, db);

          auto head_state = std::make_shared<block_state>(head_header_state);
          head_state->block = std::make_shared<signed_block>(head_header_state.header);
          fork_db.set(head_state);
          fork_db.set_validity(head_state, true);
          fork_db.mark_in_current_chain(head_state, true);
          head = head_state;
          worldstate_head_block = head->block_num;
      });

      controller_index_set::walk_indices([&]( auto utils ){
         using index_t = typename decltype(utils)::index_t;
         using value_t = typename index_t::value_type;

         // skip the table_id_object as its inlined with contract tables section
         if (std::is_same<value_t, table_id_object>::value) {
            return;
         }

         ws_helper_ptr->read_table_from_worldstate<index_t>(worldstate_db);
      });

      read_contract_tables_from_worldstate(ws_helper_ptr, worldstate_db);
      authorization.read_from_worldstate(ws_helper_ptr, worldstate_db);
      resource_limits.read_from_worldstate(ws_helper_ptr, worldstate_db);
      bls_votes.read_from_worldstate(ws_helper_ptr, worldstate_db);

      db.set_revision( head->block_num );
      ws_helper_ptr.reset();

      //copy the file, save in local path
      ws_info info;
      info.chain_id = tmp_chain_id;
      info.block_height = head->block_num;
      std::string file_name = ws_manager_ptr->get_file_path_by_info(tmp_chain_id, head->block_num);
      fc::path ws_file(file_name + ".ws");
      if (ws_file != fc::path(worldstate_path)){
         if (fc::exists(ws_file)) fc::remove(ws_file);
         fc::copy(worldstate_path, bfs::path(ws_file) );
      }
      fc::rename(id_file, bfs::path(file_name +".id") );

      info.file_size = fc::file_size(ws_file);
      info.hash_string = ws_manager_ptr->calculate_file_hash(ws_file.string()).str();
      ws_manager_ptr->save_info(info);

      worldstate_previous_block = head->block_num;
      ilog("read_from_worldstate, block_num: ${block_num}", ("block_num", head->block_num));
    }

    sha256 calculate_integrity_hash() const {
      sha256::encoder enc;
      auto hash_writer = std::make_shared<integrity_hash_worldstate_writer>(enc);
      //add_to_worldstate(hash_writer);
      hash_writer->finalize();

      return enc.result();
   }

   /**
    *  Sets fork database head to the genesis state.
    */
   void initialize_fork_db() {
      ilog( " Initializing new blockchain with genesis state                  " );
      block_header_state genheader;
      genheader.header.timestamp      = conf.genesis.initial_timestamp;
      genheader.header.action_mroot   = conf.genesis.compute_chain_id();
      genheader.header.proposer       = name("genesis");
      genheader.id                    = genheader.header.id();
      genheader.block_num             = genheader.header.block_num();

      ilog("genesis block id = ${id}", ("id", genheader.id));
       ilog("genesis1 ${block_cpu}",("block_cpu",conf.genesis.initial_configuration.max_block_cpu_usage));
      head = std::make_shared<block_state>( genheader );
      head->block = std::make_shared<signed_block>(genheader.header);
      fork_db.set( head );
      db.set_revision( head->block_num );

      initialize_database();
   }

   void create_native_account( account_name name, const authority& owner, const authority& active, bool is_privileged = false, bool is_updateable = false ) {
      db.create<account_object>([&](auto& a) {
         a.name = name;
         a.creation_date = conf.genesis.initial_timestamp;
         a.privileged = is_privileged;
         a.updateable = is_updateable;

         if( name == config::system_account_name ) {
            a.set_abi(ultrainio_contract_abi(abi_def()));
         }
      });
      const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
      bool  is_exec_add_account_sequence_object = ro_api.is_exec_patch_code( config::patch_update_version::add_account_sequence_object );
      if( !is_exec_add_account_sequence_object ){
         db.create<account_sequence_object>([&](auto & a) {
            a.name = name;
         });
      }

      const auto& owner_permission  = authorization.create_permission(name, config::owner_name, 0,
                                                                      owner, conf.genesis.initial_timestamp );
      const auto& active_permission = authorization.create_permission(name, config::active_name, owner_permission.id,
                                                                      active, conf.genesis.initial_timestamp );

      resource_limits.initialize_account(name);

      int64_t ram_delta = config::overhead_per_account_ram_bytes;
      ram_delta += 2*config::billable_size_v<permission_object>;
      ram_delta += owner_permission.auth.get_billable_size();
      ram_delta += active_permission.auth.get_billable_size();

      resource_limits.add_pending_ram_usage(name, ram_delta);
      resource_limits.verify_account_ram_usage(name);
   }

   void initialize_database() {
      // Initialize block summary index
      for (int i = 0; i < 0x10000; i++)
         db.create<block_summary_object>([&](block_summary_object&) {});

      const auto& tapos_block_summary = db.get<block_summary_object>(1);
      db.modify( tapos_block_summary, [&]( auto& bs ) {
        bs.block_id = head->id;
      });
      //TODO:all options to one unique pulgin,early than other plugins
      conf.genesis.initial_configuration.validate();
      db.create<global_property_object>([&](auto& gpo ){
        gpo.configuration = conf.genesis.initial_configuration;
      });
      db.create<dynamic_global_property_object>([](auto&){});

      standalone_object();

      authorization.initialize_database();
      resource_limits.initialize_database();
      bls_votes.initialize_database();

      authority system_auth(conf.genesis.initial_key);
      create_native_account( config::system_account_name, system_auth, system_auth, true, true );

      auto empty_authority = authority(1, {}, {});
      create_native_account( config::null_account_name, empty_authority, empty_authority );
   }



   /**
    * @post regardless of the success of commit block there is no active pending block
    */
   void commit_block( bool add_to_fork_db ) {
      auto reset_pending_on_exit = fc::make_scoped_exit([this]{
         pending.reset();
      });


      try {
         if (add_to_fork_db) {
            pending->_pending_block_state->validated = true;
            auto new_bsp = fork_db.add(pending->_pending_block_state);
            emit(self.accepted_block_header, pending->_pending_block_state);
            head = fork_db.head();
            ULTRAIN_ASSERT(new_bsp == head, fork_database_exception, "committed block did not become the new head in fork database");
         }
         emit( self.accepted_block, pending->_pending_block_state );
      } catch (...) {
         // dont bother resetting pending, instead abort the block
         reset_pending_on_exit.cancel();
         abort_block();
         throw;
      }

      bool ws = conf.worldstate_control && (0 == fork_db.head()->block_num % WS_INTERVAL) && worldstate_allowed && !worldstate_thread_running;
      ilog("commit_block ws: ${s} ${t} ${x} ${y}", ("s", ws)("t",fork_db.head()->block_num)("x", worldstate_allowed)("y", !worldstate_thread_running));

      // push the state for pending.
      pending->push(ws);

      // block syncing and listner branch, worldstate allowed for listner but not for syncing
      if ( ws ) {
         create_worldstate();
      }
   }

   // The returned scoped_exit should not exceed the lifetime of the pending which existed when make_block_restore_point was called.
   fc::scoped_exit<std::function<void()>> make_block_restore_point() {
      auto orig_block_transactions_size = pending->_pending_block_state->block->transactions.size();
      auto orig_state_transactions_size = pending->_pending_block_state->trxs.size();
      auto orig_state_actions_size      = pending->_actions.size();

      std::function<void()> callback = [this,
                                        orig_block_transactions_size,
                                        orig_state_transactions_size,
                                        orig_state_actions_size]()
      {
         pending->_pending_block_state->block->transactions.resize(orig_block_transactions_size);
         pending->_pending_block_state->trxs.resize(orig_state_transactions_size);
         pending->_actions.resize(orig_state_actions_size);
      };

      return fc::make_scoped_exit( std::move(callback) );
   }

   fc::scoped_exit<std::function<void()>> make_event_restore_point() {
      auto orig_event_size = event_list.size();

      std::function<void()> callback = [this, orig_event_size]()
      {
         while (event_list.size() > orig_event_size)
         {
             event_list.erase(--event_list.end());
         }
      };

      return fc::make_scoped_exit( std::move(callback) );
   }

   transaction_trace_ptr apply_onerror( const generated_transaction& gtrx,
                                        const packed_generated_transaction& pgtrx,
                                        fc::time_point deadline,
                                        fc::time_point start,
                                        uint32_t& cpu_time_to_bill_us, // only set on failure
                                        uint32_t billed_cpu_time_us,
                                        bool explicit_billed_cpu_time = false,
                                        bool packed_generated_trx_receipt = true,
                                        bool enforce_whiteblacklist = true ) {
      signed_transaction etrx;
      // Deliver onerror action containing the failed deferred transaction directly back to the sender.
      etrx.actions.emplace_back( vector<permission_level>{{gtrx.sender, config::active_name}},
                                 onerror( gtrx.sender_id, gtrx.packed_trx.data(), gtrx.packed_trx.size() ) );
      etrx.expiration = self.pending_block_time() + fc::microseconds(999'999); // Round up to avoid appearing expired
      etrx.set_reference_block( self.head_block_id() );

      transaction_context trx_context( self, etrx, etrx.id(), start );
      trx_context.deadline = deadline;
      trx_context.explicit_billed_cpu_time = explicit_billed_cpu_time;
      trx_context.billed_cpu_time_us = billed_cpu_time_us;
      trx_context.enforce_whiteblacklist = enforce_whiteblacklist;
      transaction_trace_ptr trace = trx_context.trace;
      try {
         trx_context.init_for_implicit_trx();
         trx_context.published = gtrx.published;
         trx_context.trace->action_traces.emplace_back();
         trx_context.dispatch_action( trx_context.trace->action_traces.back(), etrx.actions.back(), gtrx.sender );
         trx_context.finalize(); // Automatically rounds up network and CPU usage in trace and bills payers if successful

         auto restore = make_block_restore_point();
         trace->receipt = push_receipt_for_generated_trx(pgtrx, gtrx.trx_id, packed_generated_trx_receipt,
                                        transaction_receipt::soft_fail, trx_context.billed_cpu_time_us, trace->net_usage );
         fc::move_append( pending->_actions, move(trx_context.executed) );

         emit( self.applied_transaction, trace );

         trx_context.squash();
         restore.cancel();
         return trace;
      } catch( const fc::exception& e ) {
         cpu_time_to_bill_us = trx_context.update_billed_cpu_time( fc::time_point::now() );
         trace->except = e;
         trace->except_ptr = std::current_exception();
      }
      return trace;
   }

   void remove_scheduled_transaction( const generated_transaction_object& gto ) {
      resource_limits.add_pending_ram_usage(
         gto.payer,
         -(config::billable_size_v<generated_transaction_object> + gto.packed_trx.size())
      );
      // No need to verify_account_ram_usage since we are only reducing memory

      db.remove( gto );
   }

   bool failure_is_subjective( const fc::exception& e ) const {
      auto code = e.code();
      return    (code == subjective_block_production_exception::code_value)
             || (code == block_net_usage_exceeded::code_value)
             || (code == greylist_net_usage_exceeded::code_value)
             || (code == block_cpu_usage_exceeded::code_value)
             || (code == deadline_exception::code_value)
             || (code == leeway_deadline_exception::code_value)
             || (code == actor_whitelist_exception::code_value)
             || (code == actor_blacklist_exception::code_value)
             || (code == contract_whitelist_exception::code_value)
             || (code == contract_blacklist_exception::code_value)
             || (code == action_blacklist_exception::code_value)
             || (code == key_blacklist_exception::code_value);
   }

   bool scheduled_failure_is_subjective( const fc::exception& e ) const {
      auto code = e.code();
      return    (code == tx_cpu_usage_exceeded::code_value)
             || failure_is_subjective(e);
   }

   transaction_trace_ptr push_scheduled_transaction( const transaction_id_type& trxid, fc::time_point deadline, uint32_t billed_cpu_time_us, bool explicit_billed_cpu_time = false ) {
      const auto& idx = db.get_index<generated_transaction_multi_index,by_trx_id>();
      auto itr = idx.find( trxid );
      ULTRAIN_ASSERT( itr != idx.end(), unknown_transaction_exception, "unknown transaction" );
      return push_scheduled_transaction( *itr, deadline, billed_cpu_time_us, explicit_billed_cpu_time );
   }

   transaction_trace_ptr push_generated_transaction( const packed_generated_transaction& pgtrx, fc::time_point deadline, uint32_t billed_cpu_time_us, bool explicit_billed_cpu_time = false ) {
      const auto& idx = db.get_index<generated_transaction_multi_index,by_trx_id>();
      auto itr = idx.find( pgtrx.id());
      ULTRAIN_ASSERT( itr != idx.end(), unknown_transaction_exception, "unknown transaction" );
      return push_scheduled_transaction( *itr, deadline, billed_cpu_time_us, explicit_billed_cpu_time );
   }

   transaction_trace_ptr push_scheduled_transaction( const generated_transaction_object& gto, fc::time_point deadline, uint32_t billed_cpu_time_us, bool explicit_billed_cpu_time = false )
   { try {
      auto undo_session = db.start_undo_session(true);
      auto gtrx = generated_transaction(gto);

      // remove the generated transaction object after making a copy
      // this will ensure that anything which affects the GTO multi-index-container will not invalidate
      // data we need to successfully retire this transaction.
      //
      // IF the transaction FAILs in a subjective way, `undo_session` should expire without being squashed
      // resulting in the GTO being restored and available for a future block to retire.
      remove_scheduled_transaction(gto);

      fc::datastream<const char*> ds( gtrx.packed_trx.data(), gtrx.packed_trx.size() );

      ULTRAIN_ASSERT( gtrx.delay_until <= self.pending_block_time(), transaction_exception, "this transaction isn't ready",
                 ("gtrx.delay_until",gtrx.delay_until)("pbt",self.pending_block_time())          );


      signed_transaction dtrx;
      fc::raw::unpack(ds,static_cast<transaction&>(dtrx) );
      packed_generated_transaction pgt(static_cast<transaction&>(dtrx));
      const int patch_version_number = 3;
      const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
      bool new_receipt = ro_api.is_exec_patch_code(patch_version_number);

      if( gtrx.expiration < self.pending_block_time() ) {
         auto trace = std::make_shared<transaction_trace>();
         trace->id = gtrx.trx_id;
         trace->scheduled = false;
         trace->receipt = push_receipt_for_generated_trx( pgt, gtrx.trx_id, new_receipt, transaction_receipt::expired, billed_cpu_time_us, 0 ); // expire the transaction
         undo_session.squash();
         return trace;
      }

      auto reset_in_trx_requiring_checks = fc::make_scoped_exit([old_value=in_trx_requiring_checks,this](){
         in_trx_requiring_checks = old_value;
      });
      in_trx_requiring_checks = true;

      uint32_t cpu_time_to_bill_us = billed_cpu_time_us;

      transaction_context trx_context( self, dtrx, gtrx.trx_id );
      trx_context.leeway =  fc::microseconds(0); // avoid stealing cpu resource
      trx_context.deadline = deadline;
      trx_context.explicit_billed_cpu_time = explicit_billed_cpu_time;
      trx_context.billed_cpu_time_us = billed_cpu_time_us;
      trx_context.enforce_whiteblacklist = true;
      transaction_trace_ptr trace = trx_context.trace;
      try {
         trx_context.init_for_deferred_trx( gtrx.published );

         if( trx_context.enforce_whiteblacklist && pending->_block_status == controller::block_status::incomplete ) {
            check_actor_list( trx_context.auth_actors );
         }

         trx_context.preset_action_ability();
         trace->ability = trx_context.trx.actions_are_pureview() ? action::PureView : action::Normal;
         trx_context.exec();
         trx_context.finalize(); // Automatically rounds up network and CPU usage in trace and bills payers if successful

         auto restore = make_block_restore_point();

         trace->receipt = push_receipt_for_generated_trx( pgt, gtrx.trx_id, new_receipt,
                                        transaction_receipt::executed,
                                        trx_context.billed_cpu_time_us,
                                        trace->net_usage );

         fc::move_append( pending->_actions, move(trx_context.executed) );

         emit( self.applied_transaction, trace );

         trx_context.squash();
         undo_session.squash();
         restore.cancel();
         return trace;
      } catch( const fc::exception& e ) {
         cpu_time_to_bill_us = trx_context.update_billed_cpu_time( fc::time_point::now() );
         trace->except = e;
         trace->except_ptr = std::current_exception();
         trace->elapsed = fc::time_point::now() - trx_context.start;
      }
      trx_context.undo_session.undo();

      // Only subjective OR soft OR hard failure logic below:

      if( gtrx.sender != account_name() && !failure_is_subjective(*trace->except)) {
         // Attempt error handling for the generated transaction.
         dlog("${detail}", ("detail", trace->except->to_detail_string()));
         auto error_trace = apply_onerror( gtrx, pgt, deadline, trx_context.pseudo_start, cpu_time_to_bill_us,
                                      billed_cpu_time_us, explicit_billed_cpu_time, new_receipt,
                                      trx_context.enforce_whiteblacklist);
         error_trace->failed_dtrx_trace = trace;
         trace = error_trace;
         if( !trace->except_ptr ) {
            undo_session.squash();
            return trace;
         }
         trace->elapsed = fc::time_point::now() - trx_context.start;
      }

      // Only subjective OR hard failure logic below:

      // subjectivity changes based on producing vs validating
      bool subjective  = false;
      if (explicit_billed_cpu_time) {
         subjective = failure_is_subjective(*trace->except);
      } else {
         subjective = scheduled_failure_is_subjective(*trace->except);
      }

      if ( !subjective ) {
         // hard failure logic

         if( !explicit_billed_cpu_time ) {
            auto& rl = self.get_mutable_resource_limits_manager();
            rl.update_account_usage( trx_context.bill_to_accounts, block_timestamp_type(self.pending_block_time()).abstime );
            int64_t account_cpu_limit = 0;
            std::tie( std::ignore, account_cpu_limit, std::ignore ) = trx_context.max_bandwidth_billed_accounts_can_pay( true );

            cpu_time_to_bill_us = static_cast<uint32_t>( std::min( std::min( static_cast<int64_t>(cpu_time_to_bill_us),
                                                                             account_cpu_limit                          ),
                                                                   trx_context.initial_objective_duration_limit.count()    ) );
         }

         resource_limits.add_transaction_usage( trx_context.bill_to_accounts, cpu_time_to_bill_us, 0,
                                                block_timestamp_type(self.pending_block_time()).abstime ); // Should never fail

         trace->receipt = push_receipt_for_generated_trx(pgt, gtrx.trx_id, new_receipt, transaction_receipt::hard_fail, cpu_time_to_bill_us, 0);
         emit( self.applied_transaction, trace );
         undo_session.squash();
      }

      return trace;
   } FC_CAPTURE_AND_RETHROW() } /// push_scheduled_transaction


   const transaction_receipt& push_receipt_for_generated_trx(const packed_generated_transaction& trx, const transaction_id_type& trx_id,
                                            bool new_receipt, transaction_receipt_header::status_enum status,
                                            uint64_t cpu_usage_us, uint64_t net_usage) {
       if(new_receipt) {
           return push_receipt(trx, status, cpu_usage_us, net_usage);
       } else {
           return push_receipt(trx_id, status, cpu_usage_us, net_usage);
       }
   }

   /**
    *  Adds the transaction receipt to the pending block and returns it.
    */
   template<typename T>
   const transaction_receipt& push_receipt( const T& trx, transaction_receipt_header::status_enum status,
                                            uint64_t cpu_usage_us, uint64_t net_usage ) {
      uint64_t net_usage_words = net_usage / 8;
      ULTRAIN_ASSERT( net_usage_words*8 == net_usage, transaction_exception, "net_usage is not divisible by 8" );
      pending->_pending_block_state->block->transactions.emplace_back( trx );
      transaction_receipt& r = pending->_pending_block_state->block->transactions.back();
      r.cpu_usage_us         = cpu_usage_us;
      r.net_usage_words      = net_usage_words;
      r.status               = status;
      return r;
   }

   /**
    *  This is the entry point for new transactions to the block state. It will check authorization and
    *  determine whether to execute it now or to delay it. Lastly it inserts a transaction receipt into
    *  the pending block.
    */
   transaction_trace_ptr push_transaction( transaction_metadata_ptr& trx,
                                           fc::time_point deadline,
                                           bool implicit,
                                           uint32_t billed_cpu_time_us,
                                           bool explicit_billed_cpu_time = false )
   {
      ULTRAIN_ASSERT(deadline != fc::time_point(), transaction_exception, "deadline cannot be uninitialized");
      transaction_trace_ptr trace;
      try {
         transaction_context trx_context(self, trx->trx, trx->id);
         if ((bool)subjective_cpu_leeway && pending->_block_status == controller::block_status::incomplete) {
            trx_context.leeway = *subjective_cpu_leeway;
         }
         trx_context.deadline = deadline;
         trx_context.explicit_billed_cpu_time = explicit_billed_cpu_time;
         trx_context.billed_cpu_time_us = billed_cpu_time_us;
         trace = trx_context.trace;
         try {
            if( implicit ) {
               trx_context.init_for_implicit_trx();
               trx_context.enforce_whiteblacklist = false;
            } else {
               trx_context.init_for_input_trx( trx->packed_trx.get_unprunable_size(),
                                               trx->packed_trx.get_prunable_size(),
                                               trx->trx.signatures.size());
            }

            trx_context.delay = fc::seconds(trx->trx.delay_sec);

            if( !self.skip_auth_check() && !implicit ) {
               authorization.check_authorization(
                       trx->trx.actions,
                       trx->recover_keys( chain_id ),
                       {},
                       trx_context.delay,
                       [](){}
                       /*std::bind(&transaction_context::add_cpu_usage_and_check_time, &trx_context,
                                 std::placeholders::_1)*/,
                       false
               );
            }

            auto event_restore = make_event_restore_point();
            trx_context.preset_action_ability();
            trace->ability = trx_context.trx.actions_are_pureview() ? action::PureView : action::Normal;
            trx_context.exec();
            trx_context.finalize(); // Automatically rounds up network and CPU usage in trace and bills payers if successful

            auto restore = make_block_restore_point();

            if (!implicit) {
               transaction_receipt::status_enum s = (trx_context.delay == fc::seconds(0))
                                                    ? transaction_receipt::executed
                                                    : transaction_receipt::delayed;
               trace->receipt = push_receipt(trx->packed_trx, s, trx_context.billed_cpu_time_us, trace->net_usage);
               pending->_pending_block_state->trxs.emplace_back(trx);
            } else {
               transaction_receipt_header r;
               r.status = transaction_receipt::executed;
               r.cpu_usage_us = trx_context.billed_cpu_time_us;
               r.net_usage_words = trace->net_usage / 8;
               trace->receipt = r;
            }

            fc::move_append(pending->_actions, move(trx_context.executed));

            // call the accept signal but only once for this transaction
            if (!trx->accepted && !implicit && emit_signal) {
               emit( self.accepted_transaction, trx);
               trx->accepted = true;
            }

            if(!implicit && emit_signal) {
                emit(self.applied_transaction, trace);
            }

            if ( read_mode != db_read_mode::SPECULATIVE && pending->_block_status == controller::block_status::incomplete ) {
               //this may happen automatically in destructor, but I prefere make it more explicit
               trx_context.undo();
            } else {
               restore.cancel();
               trx_context.squash();
            }

            if (!implicit) {
               unapplied_transactions.erase( trx->signed_id );
            }

            event_restore.cancel();
            return trace;
         } catch (const fc::exception& e) {
            ilog("-----------exception in push_transaction ${e}", ("e", e.to_detail_string()));
            for (const auto& a : trx->trx.actions) {
                ilog("-----------exception in push_transaction account ${a1} ${a2}", ("a1", a.account)("a2", a.name));
            }
            trace->except = e;
            trace->except_ptr = std::current_exception();
         }

         if (!failure_is_subjective(*trace->except)) {
            unapplied_transactions.erase( trx->signed_id );
         }

         return trace;
      } FC_CAPTURE_AND_RETHROW((trace))
   } /// push_transaction


    void start_block( block_timestamp_type when, chain::checksum256_type committee_mroot, std::string sig, controller::block_status s ) {
      ULTRAIN_ASSERT( !pending, block_validate_exception, "pending block is not available" );

      ULTRAIN_ASSERT( db.revision() == head->block_num, database_exception, "db revision is not on par with head block",
                ("db.revision()", db.revision())("controller_head_block", head->block_num)("fork_db_head_block", fork_db.head()->block_num) );

      ilog("----------- START BLOCK");

      auto guard_pending = fc::make_scoped_exit([this](){
         pending.reset();
      });

      pending = db.start_undo_session(true);
      pending->_block_status = s;

      pending->_pending_block_state = std::make_shared<block_state>( *head, when ); // promotes pending schedule (if any) to active
      pending->_pending_block_state->in_current_chain = true;
      pending->_pending_block_state->header.committee_mroot = committee_mroot;
      pending->_pending_block_state->header.signature = sig;

      const auto& wb_object = db.get<whiteblacklist_object>();
      whiteblack_list = whiteblack_type(wb_object.actor_whitelist,wb_object.actor_blacklist,wb_object.contract_whitelist,wb_object.contract_blacklist,wb_object.action_blacklist,wb_object.key_blacklist);

      //modify state in speculative block only if we are speculative reads mode (other wise we need clean state for head or irreversible reads)
      if ( read_mode == db_read_mode::SPECULATIVE || pending->_block_status != controller::block_status::incomplete ) {
         try {
            auto onbtrx = std::make_shared<transaction_metadata>( get_on_block_transaction() );
            auto reset_in_trx_requiring_checks = fc::make_scoped_exit([old_value=in_trx_requiring_checks,this](){
                  in_trx_requiring_checks = old_value;
               });
            in_trx_requiring_checks = true;
            ilog("push onblock transaction");
            push_transaction( onbtrx, fc::time_point::maximum(), true, self.get_global_properties().configuration.min_transaction_cpu_usage, true );
         } catch( const boost::interprocess::bad_alloc& e  ) {
            elog( "on block transaction failed due to a bad allocation" );
            throw;
         } catch( const fc::exception& e ) {
            wlog( "on block transaction failed, but shouldn't impact block generation, system contract needs update" );
            edump((e.to_detail_string()));
         } catch( ... ) {
         }

         clear_expired_input_transactions();
      }

      guard_pending.cancel();
   } // start_block

   void finish_block_hack() {
      const auto& ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
      if (ro_api.is_exec_patch_code(config::patch_update_version::lifecycle_onfinish) && ro_api.is_genesis_finished()) {
          if ( read_mode == db_read_mode::SPECULATIVE || pending->_block_status != controller::block_status::incomplete ) {
              try {
                  auto on_finish_trx = std::make_shared<transaction_metadata>( get_on_finish_transaction() );
                  auto reset_in_trx_requiring_checks = fc::make_scoped_exit([old_value=in_trx_requiring_checks,this](){
                      in_trx_requiring_checks = old_value;
                  });
                  in_trx_requiring_checks = true;
                  ilog("push onfinish transaction");
                  push_transaction( on_finish_trx, fc::time_point::maximum(), true, self.get_global_properties().configuration.min_transaction_cpu_usage, true );
              } catch( const boost::interprocess::bad_alloc& e  ) {
                  elog( "onfinish transaction failed due to a bad allocation" );
                  throw;
              } catch( const fc::exception& e ) {
                  wlog( "onfinish transaction failed, but shouldn't impact block generation, system contract needs update" );
                  edump((e.to_detail_string()));
              } catch( ... ) {
                  elog("onfinish transaction failed due to unknown reason");
              }
          }
      }
   }

   void assign_header_to_block() {
      auto p = pending->_pending_block_state;
      static_cast<signed_block_header&>(*p->block) = p->header;
   } /// assign_header_to_block

    void apply_block( const signed_block_ptr& b, controller::block_status s ) { try {
      try {
         ULTRAIN_ASSERT( b->block_extensions.size() == 0, block_validate_exception, "no supported extensions" );
         start_block( b->timestamp, b->committee_mroot, b->signature, s );

         // We have to copy here.
         chain::signed_block_header* hp = &(pending->_pending_block_state->header);
         hp->proposer = b->proposer;
         hp->header_extensions = b->header_extensions;
         hp->signature = b->signature;
         transaction_trace_ptr trace;

         for( const auto& receipt : b->transactions ) {
            auto num_pending_receipts = pending->_pending_block_state->block->transactions.size();
            bool scheduled = false;
            if( receipt.trx.contains<packed_transaction>() ) {
               auto& pt = receipt.trx.get<packed_transaction>();
               auto mtrx = std::make_shared<transaction_metadata>(pt);
               trace = push_transaction( mtrx, fc::time_point::maximum(), false, receipt.cpu_usage_us, true );
            } else if( receipt.trx.contains<transaction_id_type>() ) {
               trace = push_scheduled_transaction( receipt.trx.get<transaction_id_type>(), fc::time_point::maximum(), receipt.cpu_usage_us, true );
               scheduled = true;
            } else if( receipt.trx.contains<packed_generated_transaction>() ) {
               trace = push_generated_transaction( receipt.trx.get<packed_generated_transaction>(), fc::time_point::maximum(), receipt.cpu_usage_us, true );
               scheduled = true;
            }  else {
               ULTRAIN_ASSERT( false, block_validate_exception, "encountered unexpected receipt type" );
            }

            bool transaction_failed =  trace && trace->except;
            bool transaction_can_fail = (receipt.status == transaction_receipt_header::hard_fail && scheduled);
            if( transaction_failed && !transaction_can_fail) {
               edump((*trace));
               throw *trace->except;
            }

            ULTRAIN_ASSERT( pending->_pending_block_state->block->transactions.size() > 0,
                        block_validate_exception, "expected a receipt",
                        ("block", *b)("expected_receipt", receipt)
                      );
            ULTRAIN_ASSERT( pending->_pending_block_state->block->transactions.size() == num_pending_receipts + 1,
                        block_validate_exception, "expected receipt was not added",
                        ("block", *b)("expected_receipt", receipt)
                      );
            const transaction_receipt_header& r = pending->_pending_block_state->block->transactions.back();
            ULTRAIN_ASSERT( r == static_cast<const transaction_receipt_header&>(receipt),
                        block_validate_exception, "receipt does not match",
                        ("producer_receipt", receipt)("validator_receipt", pending->_pending_block_state->block->transactions.back()) );
         }
         finish_block_hack();

         finalize_block();

         // this implicitly asserts that all header fields (less the signature) are identical
         ULTRAIN_ASSERT(b->id() == pending->_pending_block_state->header.id(),
                   block_validate_exception, "Block ID does not match",
                   ("producer_block_id",b->id())("validator_block_id",pending->_pending_block_state->header.id()));

         assign_header_to_block();

         // We need to fill out the pending block state's block because that gets serialized in the reversible block log
         // in the future we can optimize this by serializing the original and not the copy

         // we can always trust this signature because,
         //   - prior to apply_block, we call fork_db.add which does a signature check IFF the block is untrusted
         //   - OTHERWISE the block is trusted and therefore we trust that the signature is valid
         // Also, as ::sign_block does not lazily calculate the digest of the block, we can just short-circuit to save cycles
         static_cast<signed_block_header&>(*pending->_pending_block_state->block) =  pending->_pending_block_state->header;

         commit_block(false);
         return;
      } catch ( const fc::exception& e ) {
         edump((e.to_detail_string()));
         abort_block();
         throw;
      }
   } FC_CAPTURE_AND_RETHROW() } /// apply_block


   void push_block( const signed_block_ptr& b, controller::block_status s ) {
    //  idump((fc::json::to_pretty_string(*b)));
      ULTRAIN_ASSERT(!pending, block_validate_exception, "it is not valid to push a block when there is a pending block");
      try {
         ULTRAIN_ASSERT( b, block_validate_exception, "trying to push empty block" );
         ULTRAIN_ASSERT( s != controller::block_status::incomplete, block_validate_exception, "invalid block status for a completed block" );
         emit( self.pre_accepted_block, b );
         bool trust = !conf.force_all_checks && (s == controller::block_status::irreversible || s == controller::block_status::validated);
         auto new_header_state = fork_db.add( b, trust );
         emit( self.accepted_block_header, new_header_state );
         // on replay irreversible is not emitted by fork database, so emit it explicitly here
         if( !replaying && s == controller::block_status::irreversible )
            emit( self.irreversible_block, new_header_state );

         if ( read_mode != db_read_mode::IRREVERSIBLE ) {
            maybe_switch_forks( s );
         }
      } FC_LOG_AND_RETHROW( )
   }

   void register_event(const std::string& account, const std::string& post_url) {
      ULTRAIN_ASSERT(can_accept_event_register, event_register_wrong_node, "Can't register event on producing node.");
      fc::url fc_url(post_url);

      auto it = registered_event_map.find(account);
      if (it == registered_event_map.end())
      {
        auto result = registered_event_map.insert(std::pair<account_name, std::list<fc::url> >(account, std::list<fc::url>()));
        result.first->second.emplace_back(fc_url);
      }
      else
      {
        for (auto list_it = it->second.begin(); list_it != it->second.end(); ++list_it)
        {
           ULTRAIN_ASSERT(std::string(*list_it) != post_url, event_register_duplicate, "Duplicate register.");
        }
        it->second.emplace_back(fc_url);
      }
   }

   void unregister_event(const std::string& account, const std::string& post_url) {
      auto it = registered_event_map.find(account);
      if (it != registered_event_map.end())
      {
         for (auto list_it = it->second.begin(); list_it != it->second.end(); ++list_it)
         {
            if (std::string(*list_it) == post_url)
            {
               ilog("found url:", ("url", post_url));
               it->second.erase(list_it);
               return;
            }
         }
      }

      ULTRAIN_ASSERT(false, event_unregister_error, "Wrong account or unregistered url.");
   }

   bool check_event_listener(account_name account) {
      if (!can_accept_event_register) {
         return false;
      }

      auto it = registered_event_map.find(account);
      if (it != registered_event_map.end() && it->second.size() > 0)
      {
         ilog("found registered url");
         return true;
      }
      return false;
   }

   void push_event(account_name act_name, transaction_id_type id, const char* event_name, size_t event_name_size,
      const char* msg, size_t msg_size) {
      if (!can_receive_event) {
         return;
      }
      std::string ename(event_name, event_name_size);
      std::string emsg(msg, msg_size);
      event_list.emplace_back(act_name, id, ename, emsg, self.head_block_num());
      ilog("act name: ${an} trx: ${trx} event name: ${en} msg: ${msg}", ("an", act_name)("trx", id)("en", event_name)("msg", msg));
   }

   void start_receive_event() {
      event_list.clear();
      can_receive_event = true;
   }

   void stop_receive_event() {
      can_receive_event = false;
   }

   // push event to client who registered
   void notify_event()
   {
      for (auto it = event_list.begin(); it != event_list.end(); ++it) {
         auto it_cmp = std::next(it);
         while (it_cmp != event_list.end()) {
            if ((*it).id == (*it_cmp).id && (*it).name == (*it_cmp).name &&
              (*it).event_name == (*it_cmp).event_name && (*it).message == (*it_cmp).message) {
               it_cmp = event_list.erase(it_cmp);
               continue;
            }
            ++it_cmp;
         }

         if ((*it).notified) {// has already been notified, but keep it to avoid duplicate
            continue;
         }

         auto map_it = registered_event_map.find(it->name);
         if (map_it != registered_event_map.end()) {
            for (auto& post_url : map_it->second) {
               fc::variant params;
               fc::to_variant(std::make_pair((*it).event_name, (*it).message), params);
               ilog("post event: ${e} to url: ${url}", ("e", (*it).event_name)("url", post_url));
               if (!self.http_async_post(post_url, params, fc::time_point::now() + fc::microseconds(1000000))) {
                  elog("async post event failed.");
               }
            }
         }
         (*it).notified = true;
      }
   }


   void maybe_switch_forks( controller::block_status s = controller::block_status::complete ) {
      auto new_head = fork_db.head();

      if( new_head->header.previous == head->id ) {
         try {
            apply_block( new_head->block, s );
            fork_db.mark_in_current_chain( new_head, true );//
            fork_db.set_validity( new_head, true );
            head = new_head;
         } catch ( const fc::exception& e ) {
            fork_db.set_validity( new_head, false ); // Removes new_head from fork_db index, so no need to mark it as not in the current chain.
            throw;
         }
      } else if( new_head->id != head->id ) {
         ilog("switching forks from ${current_head_id} (block number ${current_head_num}) to ${new_head_id} (block number ${new_head_num})",
              ("current_head_id", head->id)("current_head_num", head->block_num)("new_head_id", new_head->id)("new_head_num", new_head->block_num) );
         ULTRAIN_ASSERT(false, chain_exception, "should never switch forks");
      }
   } /// push_block

   void abort_block(bool drop_trx = false) {
      if( pending ) {
         if ( read_mode == db_read_mode::SPECULATIVE && !drop_trx) {
             for( const auto& t : pending->_pending_block_state->trxs ) {
               unapplied_transactions[t->signed_id] = t;
             }
         }
         ilog("########### ABORT BLOCK ############# trx size ${s}", ("s",unapplied_transactions.size()));
         pending.reset();
      }
   }

    block_timestamp_type get_proper_next_block_timestamp() const {
        auto time_now = fc::time_point::now();
        auto block_timestamp = chain::block_timestamp_type(time_now);
        if (block_timestamp <= head->header.timestamp) {
            block_timestamp = head->header.timestamp.next();
        }
        return block_timestamp;
    }

   bool should_enforce_runtime_limits()const {
      return false;
   }

   void set_version(uint32_t version) {
      pending->_pending_block_state->header.version = version;
   }

   void set_proposer(account_name proposer) {
      pending->_pending_block_state->header.proposer = proposer;
   }

   void set_action_merkle() {
      vector<digest_type> action_digests;
      action_digests.reserve( pending->_actions.size() );
      for( const auto& a : pending->_actions )
         action_digests.emplace_back( a.digest() );

      pending->_pending_block_state->header.action_mroot = merkle( move(action_digests) );
   }

   void set_trx_merkle() {
      vector<digest_type> trx_digests;
      const auto& trxs = pending->_pending_block_state->block->transactions;
      trx_digests.reserve( trxs.size() );
      for( const auto& a : trxs )
         trx_digests.emplace_back( a.digest() );

      pending->_pending_block_state->header.transaction_mroot = merkle( move(trx_digests) );
   }

   void set_header_extensions(const extensions_type& exts) {
       pending->_pending_block_state->header.header_extensions = exts;
   }

   void add_header_extensions_entry(uint16_t key, const vector<char>& value) {
       auto& exts = pending->_pending_block_state->header.header_extensions;
       for (auto itor = exts.begin(); itor != exts.end(); itor++) {
           if (itor->first == key) {
              exts.erase(itor);
              break;
           }
       }
       exts.emplace_back(key, value);
   }

   void finalize_block()
   {
      ULTRAIN_ASSERT(pending, block_validate_exception, "it is not valid to finalize when there is no pending block");
      try {

      /*
      ilog( "finalize block ${n} (${id}) at ${t} by ${p} (${signing_key}); lib: ${lib} #dtrxs: ${ndtrxs} ${np}",
            ("n",pending->_pending_block_state->block_num)
            ("id",pending->_pending_block_state->header.id())
            ("t",pending->_pending_block_state->header.timestamp)
            ("signing_key", pending->_pending_block_state->block_signing_key)
            ("lib",pending->_pending_block_state->dpos_irreversible_blocknum)
            ("ndtrxs",db.get_index<generated_transaction_multi_index,by_trx_id>().size())
            ("np",pending->_pending_block_state->header.new_producers)
            );
      */

      // Update resource limits:
      resource_limits.process_account_limit_updates();
      const auto& chain_config = self.get_global_properties().configuration;
      uint32_t max_virtual_mult = 1;//1000;  //virtual resources are not currently used
      uint64_t CPU_TARGET = ULTRAIN_PERCENT(chain_config.max_block_cpu_usage, chain_config.target_block_cpu_usage_pct);
      //ilog("set_block_parameters chain_config.max_block_cpu_usage:${c},chain_config.max_block_net_usage:${n},", ("c", chain_config.max_block_cpu_usage)("n", chain_config.max_block_net_usage));
      resource_limits.set_block_parameters(
         { CPU_TARGET, chain_config.max_block_cpu_usage, config::block_cpu_usage_average_window_ms / config::block_interval_ms, max_virtual_mult, {99, 100}, {1000, 999}},
         {ULTRAIN_PERCENT(chain_config.max_block_net_usage, chain_config.target_block_net_usage_pct), chain_config.max_block_net_usage, config::block_size_average_window_ms / config::block_interval_ms, max_virtual_mult, {99, 100}, {1000, 999}}
      );
      resource_limits.process_block_usage(pending->_pending_block_state->block_num);

      set_action_merkle();
      set_trx_merkle();

      auto p = pending->_pending_block_state;
      p->id = p->header.id();
      /*
      ilog("----------finalize block current header is ${t} ${p} ${pk} ${pf} ${v} ${prv} ${ma} ${mt} ${id}",
	   ("t", p->header.timestamp)
	   ("pk", p->header.proposerPk)
	   ("v", p->header.version)
	   ("prv", p->header.previous)
	   ("ma", p->header.transaction_mroot)
	   ("mt", p->header.action_mroot)
	   ("id", p->header.id()));
      */
      create_block_summary(p->id);

   } FC_CAPTURE_AND_RETHROW() }

   void create_block_summary(const block_id_type& id) {
      auto block_num = block_header::num_from_id(id);
      auto sid = block_num & 0xffff;
      db.modify( db.get<block_summary_object,by_id>(sid), [&](block_summary_object& bso ) {
          bso.block_id = id;
      });
   }


   void clear_expired_input_transactions() {
      //Look for expired transactions in the deduplication list, and remove them.
      auto& transaction_idx = db.get_mutable_index<transaction_multi_index>();
      const auto& dedupe_index = transaction_idx.indices().get<by_expiration>();
      auto now = self.pending_block_time();
      while( (!dedupe_index.empty()) && ( now > fc::time_point(dedupe_index.begin()->expiration) ) ) {
         transaction_idx.remove(*dedupe_index.begin());
      }
   }


   void check_actor_list( const flat_set<account_name>& actors )const {
      if( whiteblack_list.actor_whitelist.size() > 0 ) {
         vector<account_name> excluded;
         excluded.reserve( actors.size() );
         set_difference( actors.begin(), actors.end(),
                         whiteblack_list.actor_whitelist.begin(), whiteblack_list.actor_whitelist.end(),
                         std::back_inserter(excluded) );
         if( excluded.size() == 1 && excluded[0] == config::system_account_name )
             return ;
         ULTRAIN_ASSERT( excluded.size() == 0, actor_whitelist_exception,
                     "authorizing actor(s) in transaction are not on the actor whitelist: ${actors}",
                     ("actors", excluded)
                   );
      } else if( whiteblack_list.actor_blacklist.size() > 0 ) {
         vector<account_name> blacklisted;
         blacklisted.reserve( actors.size() );
         set_intersection( actors.begin(), actors.end(),
                           whiteblack_list.actor_blacklist.begin(), whiteblack_list.actor_blacklist.end(),
                           std::back_inserter(blacklisted)
                         );
         ULTRAIN_ASSERT( blacklisted.size() == 0, actor_blacklist_exception,
                     "authorizing actor(s) in transaction are on the actor blacklist: ${actors}",
                     ("actors", blacklisted)
                   );
      }
   }

   void check_contract_list( account_name code )const {
      if ( code == config::system_account_name )
          return ;
      if( whiteblack_list.contract_whitelist.size() > 0 ) {
         ULTRAIN_ASSERT( whiteblack_list.contract_whitelist.find( code ) != whiteblack_list.contract_whitelist.end(),
                     contract_whitelist_exception,
                     "account '${code}' is not on the contract whitelist", ("code", code)
                   );
      } else if( whiteblack_list.contract_blacklist.size() > 0 ) {
         ULTRAIN_ASSERT( whiteblack_list.contract_blacklist.find( code ) == whiteblack_list.contract_blacklist.end(),
                     contract_blacklist_exception,
                     "account '${code}' is on the contract blacklist", ("code", code)
                   );
      }
   }

   void check_action_list( account_name code, action_name action )const {
      if( whiteblack_list.action_blacklist.size() > 0 ) {
         ULTRAIN_ASSERT( whiteblack_list.action_blacklist.find( std::make_pair(code, action) ) == whiteblack_list.action_blacklist.end(),
                     action_blacklist_exception,
                     "action '${code}::${action}' is on the action blacklist",
                     ("code", code)("action", action)
                   );
      }
   }

   void check_key_list( const public_key_type& key )const {
      if( whiteblack_list.key_blacklist.size() > 0 ) {
         ULTRAIN_ASSERT( whiteblack_list.key_blacklist.find( key ) == whiteblack_list.key_blacklist.end(),
                     key_blacklist_exception,
                     "public key '${key}' is on the key blacklist",
                     ("key", key)
                   );
      }
   }

   /*
   bool should_check_tapos()const { return true; }

   void validate_tapos( const transaction& trx )const {
      if( !should_check_tapos() ) return;

      const auto& tapos_block_summary = db.get<block_summary_object>((uint16_t)trx.ref_block_num);

      //Verify TaPoS block summary has correct ID prefix, and that this block's time is not past the expiration
      ULTRAIN_ASSERT(trx.verify_reference_block(tapos_block_summary.block_id), invalid_ref_block_exception,
                 "Transaction's reference block did not match. Is this transaction from a different fork?",
                 ("tapos_summary", tapos_block_summary));
   }
   */


   /**
    *  At the start of each block we notify the system contract with a transaction that passes in
    *  the block header of the prior block (which is currently our head block)
    */
   signed_transaction get_on_block_transaction()
   {
      action on_block_act;
      on_block_act.account = config::system_account_name;
      on_block_act.name = NEX(onblock);
      on_block_act.authorization = vector<permission_level>{{config::system_account_name, config::active_name}};
      on_block_act.data = fc::raw::pack(self.head_block_header());

      signed_transaction trx;
      trx.actions.emplace_back(std::move(on_block_act));
      trx.set_reference_block(self.head_block_id());
      trx.expiration = self.pending_block_time() + fc::microseconds(999'999); // Round up to nearest second to avoid appearing expired
      return trx;
   }

   /**
    *  At the end of each block
    *
    */
   signed_transaction get_on_finish_transaction()
   {
      action on_finish_act;
      on_finish_act.account = config::system_account_name;
      on_finish_act.name = NEX(onfinish);
      on_finish_act.authorization = vector<permission_level>{{config::system_account_name, config::active_name}};

      signed_transaction trx;
      trx.actions.emplace_back(std::move(on_finish_act));
      trx.set_reference_block(self.head_block_id());
      trx.expiration = self.pending_block_time() + fc::microseconds(999'999); // Round up to nearest second to avoid appearing expired
      return trx;
   }

}; /// controller_impl

void controller::set_emit_signal()
{
    my->emit_signal = true;
}
void controller::clear_emit_signal()
{
    my->emit_signal = false;
}
void controller::enable_worldstate_creation()
{
    my->worldstate_allowed = true;
}
void controller::disable_worldstate_creation()
{
    my->worldstate_allowed = false;
}
const resource_limits_manager&   controller::get_resource_limits_manager()const
{
   return my->resource_limits;
}
resource_limits_manager&         controller::get_mutable_resource_limits_manager()
{
   return my->resource_limits;
}

const authorization_manager&   controller::get_authorization_manager()const
{
   return my->authorization;
}
authorization_manager&         controller::get_mutable_authorization_manager()
{
   return my->authorization;
}

bls_votes_manager&             controller::get_bls_votes_manager()
{
   return my->bls_votes;
}

#ifdef ULTRAIN_CONFIG_CONTRACT_PARAMS
uint64_t controller::get_contract_return_length() const {
  return my->conf.contract_return_length;
}

uint64_t controller::get_contract_emit_length() const {
  return my->conf.contract_emit_length;
}
#endif

controller::controller( const controller::config& cfg )
:my( new controller_impl( cfg, *this ) )
{
}

controller::~controller() {
   my->abort_block();
   //close fork_db here, because it can generate "irreversible" signal to this controller,
   //in case if read-mode == IRREVERSIBLE, we will apply latest irreversible block
   //for that we need 'my' to be valid pointer pointing to valid controller_impl.
   my->fork_db.close();
}

void controller::check_actor_list( const flat_set<account_name>& actors )const {
   my->check_actor_list(actors);
}

void controller::add_indices() {
   my->add_indices();
}

void controller::startup( bfs::path worldstate_path ) {
   my->head = my->fork_db.head();
   if( !my->head ) {
      elog( "No head block in fork db, perhaps we need to replay" );
   }
   my->init(worldstate_path);
}

chainbase::database& controller::db()const { return my->db; }

fork_database& controller::fork_db()const { return my->fork_db; }


void controller::start_block( block_timestamp_type when, chain::checksum256_type committee_mroot, std::string sig) {
   validate_db_available_size();
   my->start_block(when, committee_mroot, sig, block_status::incomplete );
}

void controller::finish_block_hack() {
   validate_db_available_size();
   my->finish_block_hack();
}

void controller::finalize_block() {
   validate_db_available_size();
   my->finalize_block();
}

void controller::assign_header_to_block() {
   my->assign_header_to_block();
}

void controller::commit_block() {
   validate_db_available_size();
   my->commit_block(true);
}

void controller::abort_block(bool drop_trx) {
   my->abort_block(drop_trx);
}

void controller::push_block( const signed_block_ptr& b, block_status s ) {
   validate_db_available_size();
   my->push_block( b, s );
}

void controller::register_event(const std::string& account, const std::string& post_url) {
   my->register_event(account, post_url);
}

void controller::unregister_event(const std::string& account, const std::string& post_url) {
   my->unregister_event(account, post_url);
}

bool controller::check_event_listener(account_name account) {
   return my->check_event_listener(account);
}

void controller::push_event(account_name act_name, transaction_id_type id,
         const char* event_name, size_t event_name_size,
         const char* msg, size_t msg_size ) {
   my->push_event(act_name, id, event_name, event_name_size, msg, msg_size);
}

void controller::notify_event() {
   my->notify_event();
}

void controller::start_receive_event() {
   my->start_receive_event();
}

void controller::stop_receive_event() {
   my->stop_receive_event();
}

transaction_trace_ptr controller::push_transaction( transaction_metadata_ptr& trx, fc::time_point deadline, uint32_t billed_cpu_time_us ) {
   validate_db_available_size();
   return my->push_transaction(trx, deadline, false, billed_cpu_time_us, billed_cpu_time_us > 0 );
}

transaction_trace_ptr controller::push_scheduled_transaction( const transaction_id_type& trxid, fc::time_point deadline, uint32_t billed_cpu_time_us )
{
   validate_db_available_size();
   return my->push_scheduled_transaction( trxid, deadline, billed_cpu_time_us, billed_cpu_time_us > 0 );
}

transaction_trace_ptr controller::push_generated_transaction( const packed_generated_transaction& pgtrx, fc::time_point deadline, uint32_t billed_cpu_time_us ) {
   validate_db_available_size();
   return my->push_generated_transaction( pgtrx, deadline, billed_cpu_time_us, billed_cpu_time_us > 0 );
}

block_timestamp_type controller::get_proper_next_block_timestamp() const {
  return my->get_proper_next_block_timestamp();
}

uint32_t controller::first_block_num()const {
   return my->blog.first_block_num();
}

uint32_t controller::head_block_num()const {
   return my->head->block_num;
}


time_point controller::head_block_time()const {
   return my->head->header.timestamp;
}
block_id_type controller::head_block_id()const {
   return my->head->id;
}
const block_header& controller::head_block_header()const {
   return my->head->header;
}
block_state_ptr controller::head_block_state()const {
   return my->head;
}

uint32_t controller::fork_db_head_block_num()const {
   return my->fork_db.head()->block_num;
}
account_name  controller::head_block_proposer()const {
   return my->head->header.proposer;
}
account_name  controller::fork_db_head_block_proposer()const {
   return my->fork_db.head()->header.proposer;
}
block_id_type controller::fork_db_head_block_id()const {
   return my->fork_db.head()->id;
}

time_point controller::fork_db_head_block_time()const {
   return my->fork_db.head()->header.timestamp;
}

block_state_ptr controller::pending_block_state()const {
   if( my->pending ) return my->pending->_pending_block_state;
   return block_state_ptr();
}

block_state_ptr controller::pending_block_state_hack() {
   if( my->pending ) return my->pending->_pending_block_state;
   return block_state_ptr();
}

void controller::set_version(uint32_t version) {
   my->set_version(version);
}

void controller::set_proposer(account_name proposer) {
   my->set_proposer(proposer);
}

void controller::set_action_merkle_hack() {
    my->set_action_merkle();
}
void controller::set_trx_merkle_hack() {
    my->set_trx_merkle();
}

void controller::set_header_extensions(const extensions_type& exts) {
    my->set_header_extensions(exts);
}

void controller::add_header_extensions_entry(uint16_t key, const vector<char>& value) {
    my->add_header_extensions_entry(key, value);
}

time_point controller::pending_block_time()const {
   ULTRAIN_ASSERT( my->pending, block_validate_exception, "no pending block" );
   return my->pending->_pending_block_state->header.timestamp;
}

uint32_t controller::block_interval_seconds()const {
    return chain::config::block_interval_ms / 1000;
}

uint32_t controller::last_irreversible_block_num() const {
   return std::max(my->head->irreversible_blocknum, my->worldstate_head_block);
}

block_id_type controller::last_irreversible_block_id() const {
   auto lib_num = last_irreversible_block_num();
   const auto& tapos_block_summary = db().get<block_summary_object>((uint16_t)lib_num);

   if( block_header::num_from_id(tapos_block_summary.block_id) == lib_num )
      return tapos_block_summary.block_id;

   return fetch_block_by_number(lib_num)->id();

}

const dynamic_global_property_object& controller::get_dynamic_global_properties()const {
  return my->db.get<dynamic_global_property_object>();
}
const global_property_object& controller::get_global_properties()const {
  return my->db.get<global_property_object>();
}

signed_block_ptr controller::fetch_block_by_id( block_id_type id )const {
   auto state = my->fork_db.get_block(id);
   if( state ) return state->block;
   auto bptr = fetch_block_by_number( block_header::num_from_id(id) );
   if( bptr && bptr->id() == id ) return bptr;
   return signed_block_ptr();
}

signed_block_ptr controller::fetch_block_by_number( uint32_t block_num )const  { try {
   auto blk_state = my->fork_db.get_block_in_current_chain_by_num( block_num );
   if( blk_state ) {
      return blk_state->block;
   }

   return my->blog.read_block_by_num(block_num);
} FC_CAPTURE_AND_RETHROW( (block_num) ) }

block_state_ptr controller::fetch_block_state_by_id( block_id_type id )const {
   auto state = my->fork_db.get_block(id);
   return state;
}

block_state_ptr controller::fetch_block_state_by_number( uint32_t block_num )const  { try {
   auto blk_state = my->fork_db.get_block_in_current_chain_by_num( block_num );
   return blk_state;
} FC_CAPTURE_AND_RETHROW( (block_num) ) }

block_id_type controller::get_block_id_for_num( uint32_t block_num )const { try {
   auto blk_state = my->fork_db.get_block_in_current_chain_by_num( block_num );
   if( blk_state ) {
      return blk_state->id;
   }

   auto signed_blk = my->blog.read_block_by_num(block_num);

   ULTRAIN_ASSERT( BOOST_LIKELY( signed_blk != nullptr ), unknown_block_exception,
               "Could not find block: ${block}", ("block", block_num) );

   return signed_blk->id();
} FC_CAPTURE_AND_RETHROW( (block_num) ) }

void controller::read_worldstate( const bfs::path& worldstate_path ) {
    return my->read_from_worldstate(worldstate_path);
}

sha256 controller::calculate_integrity_hash()const { try {
   return my->calculate_integrity_hash();
} FC_LOG_AND_RETHROW() }

bool controller::skip_auth_check()const {
   return my->replaying && !my->conf.force_all_checks && !my->in_trx_requiring_checks;
}

bool controller::contracts_console()const {
   return my->conf.contracts_console;
}

chain_id_type controller::get_chain_id()const {
   return my->chain_id;
}

db_read_mode controller::get_read_mode()const {
   return my->read_mode;
}

const apply_handler* controller::find_apply_handler( account_name receiver, account_name scope, action_name act ) const
{
   auto native_handler_scope = my->apply_handlers.find( receiver );
   if( native_handler_scope != my->apply_handlers.end() ) {
      auto handler = native_handler_scope->second.find( make_pair( scope, act ) );
      if( handler != native_handler_scope->second.end() )
         return &handler->second;
   }
   return nullptr;
}
wasm_interface& controller::get_wasm_interface() {
   return my->wasmif;
}

const account_object& controller::get_account( account_name name )const
{ try {
   return my->db.get<account_object, by_name>(name);
} FC_CAPTURE_AND_RETHROW( (name) ) }

vector<transaction_metadata_ptr> controller::get_unapplied_transactions() {
   vector<transaction_metadata_ptr> result;
   if ( my->read_mode == db_read_mode::SPECULATIVE ) {
      result.reserve(my->unapplied_transactions.size());
      for ( const auto& entry: my->unapplied_transactions ) {
         result.emplace_back(entry.second);
      }
   } else {
      ULTRAIN_ASSERT( my->unapplied_transactions.empty(), transaction_exception, "not empty unapplied_transactions in non-speculative mode" ); //should never happen
   }
   return result;
}

void controller::drop_unapplied_transaction(const transaction_metadata_ptr& trx) {
   my->unapplied_transactions.erase(trx->signed_id);
}

std::list<transaction_metadata_ptr>* controller::get_pending_transactions() {
  return &(my->pending_transactions);
}

std::pair<bool, bool> controller::push_into_pending_transaction(const transaction_metadata_ptr& trx) {
    if (my->pending_transactions.size() > ultrainio::chain::config::default_max_pending_trx_count ||
        my->unapplied_transactions.size() > ultrainio::chain::config::default_max_unapplied_trx_count)
        return std::pair<bool, bool>(true, false);

    if (my->pending_transactions_set.find(trx->signed_id) == my->pending_transactions_set.end()) {
        my->pending_transactions_set.insert(trx->signed_id);
        my->pending_transactions.push_back(trx);
        return std::pair<bool, bool>(false, false);
    }
    // TODO(yufengshen): THis should be removed eventually.
    ULTRAIN_ASSERT( my->pending_transactions_set.size() == my->pending_transactions.size(),
                    chain_exception,
                    "pending trx size ${s1} not equal to pending trx set size ${s2}",
                    ("s1", my->pending_transactions.size())
                    ("s2", my->pending_transactions_set.size()));
    return std::pair<bool, bool>(false, true);

}

void controller::drop_pending_transaction_from_set(const transaction_metadata_ptr& trx) {
    my->pending_transactions_set.erase(trx->signed_id);
}

void controller::clear_unapplied_transaction() {
   my->unapplied_transactions.clear();
}

vector<transaction_id_type> controller::get_scheduled_transactions() {
   const auto& idx = db().get_index<generated_transaction_multi_index,by_delay>();

   vector<transaction_id_type> result;

   static const size_t max_reserve = 64;
   result.reserve(std::min(idx.size(), max_reserve));

   auto itr = idx.begin();
   while( itr != idx.end() && itr->delay_until <= pending_block_time() ) {
      result.emplace_back(itr->trx_id);
      ++itr;
   }
   return result;
}

void controller::check_contract_list( account_name code )const {
   my->check_contract_list( code );
}

void controller::check_action_list( account_name code, action_name action )const {
   my->check_action_list( code, action );
}

void controller::check_key_list( const public_key_type& key )const {
   my->check_key_list( key );
}

bool controller::is_producing_block()const {
   if( !my->pending ) return false;

   return (my->pending->_block_status == block_status::incomplete);
}

void controller::validate_expiration( const transaction& trx )const { try {

   const auto& chain_configuration = get_global_properties().configuration;

   ULTRAIN_ASSERT( time_point(trx.expiration) >= pending_block_time(),
               expired_tx_exception,
               "transaction has expired, "
               "expiration is ${trx.expiration} and pending block time is ${pending_block_time}",
               ("trx.expiration",trx.expiration)("pending_block_time",pending_block_time()));
   ULTRAIN_ASSERT( time_point(trx.expiration) <= pending_block_time() + fc::seconds(chain_configuration.max_transaction_lifetime),
               tx_exp_too_far_exception,
               "Transaction expiration is too far in the future relative to the reference time of ${reference_time}, "
               "expiration is ${trx.expiration} and the maximum transaction lifetime is ${max_til_exp} seconds",
               ("trx.expiration",trx.expiration)("reference_time",pending_block_time())
               ("max_til_exp",chain_configuration.max_transaction_lifetime) );

} FC_CAPTURE_AND_RETHROW((trx)) }

void controller::validate_tapos( const transaction& trx )const { try {

    const auto& tapos_block_summary = db().get<block_summary_object>((uint16_t)trx.ref_block_num);

   //Verify TaPoS block summary has correct ID prefix, and that this block's time is not past the expiration
   ULTRAIN_ASSERT(trx.verify_reference_block(tapos_block_summary.block_id), invalid_ref_block_exception,
              "Transaction's reference block did not match. Is this transaction from a different fork?",
              ("tapos_summary", tapos_block_summary));

} FC_CAPTURE_AND_RETHROW() }

void controller::validate_db_available_size() const {
   const auto free = db().get_segment_manager()->get_free_memory();
   const auto guard = my->conf.state_guard_size;
   ULTRAIN_ASSERT(free >= guard, database_guard_exception, "database free: ${f}, guard size: ${g}", ("f", free)("g",guard));
}

bool controller::is_known_unexpired_transaction( const transaction_id_type& id) const {
   return db().find<transaction_object, by_trx_id>(id);
}

void controller::set_subjective_cpu_leeway(fc::microseconds leeway) {
   my->subjective_cpu_leeway = leeway;
}

void controller::add_resource_greylist(const account_name &name) {
   my->conf.resource_greylist.insert(name);
}

void controller::remove_resource_greylist(const account_name &name) {
   my->conf.resource_greylist.erase(name);
}

bool controller::is_resource_greylisted(const account_name &name) const {
   return my->conf.resource_greylist.find(name) !=  my->conf.resource_greylist.end();
}

void controller::enable_event_register(bool v){
    ilog("controller::enable_event_register ${v}", ("v", v));
    my->can_accept_event_register = v;
}

const flat_set<account_name> &controller::get_resource_greylist() const {
   return  my->conf.resource_greylist;
}

vector<digest_type> controller::merkle_proof_of(const uint32_t& block_number, const digest_type& trx_id, vector<char>& trx_receipt_bytes) const {
   auto make_digests_even = [](vector<digest_type>& vdigests) {
      if (vdigests.size() % 2)
         vdigests.push_back(vdigests.back());
   };

   auto get_next_digest = [](const size_t& pos, const vector<digest_type>& vdigests) -> digest_type {
      if (vdigests.size() % 2 != 0 || pos >= vdigests.size()) return digest_type();

      if (pos % 2 != 0) return make_canonical_left(vdigests[pos - 1]); // left leaf
      else return make_canonical_right(vdigests[pos + 1]); // right leaf
   };

   auto next_loop_merkle_tree = [](vector<digest_type>& ids) {
      if( ids.size() % 2 != 0 )
         ids.push_back(ids.back());

      for (int i = 0; i < ids.size() / 2; i++) {
         ids[i] = digest_type::hash(make_canonical_pair(ids[2 * i], ids[(2 * i) + 1]));
      }

      ids.resize(ids.size() / 2);
   };
   // for debug
   auto print_mt = [](const vector<digest_type>& ids) {
      std::cout << "merkle_proof digest: " << std::endl;
      for (const auto& dt : ids) {
         std::cout <<"\t"<< string(dt) << std::endl;
      }
   };

   signed_block_ptr block;
   try {
         block = fetch_block_by_number(block_number);
   } catch(...) {
      block = nullptr;
   }

   // to generate merkle proof vector
   vector<digest_type> merkle_proof;

   if (block == nullptr) return merkle_proof;

   const vector<transaction_receipt>& trxs = block->transactions;
   if (trxs.empty()) return merkle_proof;

   size_t target_pos = -1;
   for (auto i = 0; i < trxs.size(); i++) {
      if (trxs[i].trx.contains<packed_transaction>() && trxs[i].trx.get<packed_transaction>().id() == trx_id) {
         target_pos = i;
         break;
      } else if (trxs[i].trx.contains<packed_generated_transaction>() && trxs[i].trx.get<packed_generated_transaction>().id() == trx_id) {
         target_pos = i;
         break;
      }
   }

   if (target_pos == -1) return merkle_proof;

   trx_receipt_bytes = fc::raw::pack(trxs[target_pos]);

   vector<digest_type> trx_digests;
   trx_digests.reserve( trxs.size() );
   for( const auto& a : trxs ) {
      trx_digests.emplace_back( a.digest() );
   }
   // for test now, fill the real digest of original tx
   if (target_pos % 2 == 0) merkle_proof.push_back(make_canonical_left(digest_type()));
   else merkle_proof.push_back(make_canonical_right(digest_type()));

   //std::cout << "merkle_proof: " << string(merkle_proof.back()) << std::endl;
   // if there is only one tx, then trx' digest is mroot.
   if (trx_digests.size() == 1) return merkle_proof;

   while (trx_digests.size() > 1) {
      make_digests_even(trx_digests);

      //print_mt(trx_digests);
      auto n = get_next_digest(target_pos, trx_digests);
      merkle_proof.push_back(n);

      target_pos /= 2;
      next_loop_merkle_tree(trx_digests);
      // std::cout << "merkle_proof: " << string(merkle_proof.back()) << std::endl;
   }

   return merkle_proof;
}

// static void print_log(const vector<digest_type>& merkle_proof, const vector<char>& trx_receipt_bytes) {
//    std::cout << "controller::verify_merkle_proof, merkle_proof: " << std::endl;
//    for (const auto& dg : merkle_proof) {
//       std::cout <<"\t" << string(dg) << std::endl;
//    }
//
//    std::cout << "controller::verify_merkle_proof, trx_receipt_bytes.length = "<< trx_receipt_bytes.size() << std::endl;
//    for (auto c: trx_receipt_bytes) std::cout << std::hex << (int(c) & 0xff) << " ";
//    std::cout << std::endl;
// }

bool controller::verify_merkle_proof(const vector<digest_type>& merkle_proof, const digest_type& transaction_mroot, const vector<char>& trx_receipt_bytes) const {

   // print_log(merkle_proof, trx_receipt_bytes);

   if (merkle_proof.size() == 0) return false;
   if (transaction_mroot == digest_type()) return false;

   transaction_receipt trx = fc::raw::unpack<transaction_receipt>(trx_receipt_bytes);
   if (merkle_proof.size() == 1) {
      return trx.digest() == transaction_mroot;
   }

   auto first = merkle_proof[0];
   auto second = merkle_proof[1];

   digest_type left_leaf, right_leaf;
   if (is_canonical_left(first) && is_canonical_right(second)) {
      left_leaf = make_canonical_left(trx.digest());
      right_leaf = second;
   } else if (is_canonical_left(second) && is_canonical_right(first)) {
      left_leaf = second;
      right_leaf = make_canonical_right(trx.digest());
   } else {
      return false;
   }

   auto proof = digest_type::hash(make_canonical_pair(left_leaf, right_leaf));

   for (int i = 2; i < merkle_proof.size(); i++) {
      const digest_type& t = merkle_proof[i];
      if (is_canonical_left(t)) {
         left_leaf = t;
         right_leaf = make_canonical_right(proof);
      } else {
         left_leaf =  make_canonical_left(proof);
         right_leaf = t;
      }

      proof = digest_type::hash(make_canonical_pair(left_leaf, right_leaf));
   }

   return proof == transaction_mroot;
}
} } /// ultrainio::chain
