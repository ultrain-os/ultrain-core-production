#include <algorithm>
#include <ultrainio/chain/apply_context.hpp>
#include <ultrainio/chain/controller.hpp>
#include <ultrainio/chain/transaction_context.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <ultrainio/chain/wasm_interface.hpp>
#include <ultrainio/chain/generated_transaction_object.hpp>
#include <ultrainio/chain/authorization_manager.hpp>
#include <ultrainio/chain/resource_limits.hpp>
#include <ultrainio/chain/account_object.hpp>
#include <ultrainio/chain/global_property_object.hpp>
#include <appbase/application.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/iterator/reverse_iterator.hpp>

using boost::container::flat_set;
static const uint64_t calc_num_limit = 5000;
static const uint64_t calc_delram_bytes_limit = 10*1024*1024;  //Remove the ram maximum byte limit

namespace ultrainio { namespace chain {

static inline void print_debug(account_name receiver, const action_trace& ar) {
   if (!ar.console.empty()) {
      auto prefix = fc::format_string(
                                      "\n[(${a},${n})->${r}]",
                                      fc::mutable_variant_object()
                                      ("a", ar.act.account)
                                      ("n", ar.act.name)
                                      ("r", receiver));
      dlog(prefix + ": CONSOLE OUTPUT BEGIN =====================\n"
           + ar.console
           + prefix + ": CONSOLE OUTPUT END   =====================" );
   }
}

action_trace apply_context::exec_one()
{
   auto start = fc::time_point::now();

   const auto& cfg = control.get_global_properties().configuration;
   try {
      const auto& a = control.get_account( receiver );
      privileged = a.privileged;
      auto native = control.find_apply_handler( receiver, act.account, act.name );
      if( native ) {
         if( trx_context.enforce_whiteblacklist && control.is_producing_block()) {
            control.check_contract_list( receiver );
            control.check_action_list( act.account, act.name );
         }
         (*native)( *this );
      }

      if( a.code.size() > 0
          && !(act.account == config::system_account_name && (act.name == NEX( setcode ) || act.name == NEX( setabi )) &&
               receiver == config::system_account_name)) {
         if( trx_context.enforce_whiteblacklist && control.is_producing_block()) {
            control.check_contract_list( receiver );
            control.check_action_list( act.account, act.name );
         }
         try {
            control.get_wasm_interface().apply( a.code_version, a.code, *this );
         } catch( const wasm_exit& ) {}
      }

   } FC_RETHROW_EXCEPTIONS(warn, "pending console output: ${console}", ("console", _pending_console_output.str()))

   action_receipt r;
   r.receiver         = receiver;
   r.act_digest       = digest_type::hash(act);
   r.global_sequence  = next_global_sequence();
   r.recv_sequence    = next_recv_sequence( receiver );

   const auto& account_sequence = db.get<account_sequence_object, by_name>(act.account);
   r.code_sequence    = account_sequence.code_sequence;
   r.abi_sequence     = account_sequence.abi_sequence;

   for( const auto& auth : act.authorization ) {
      r.auth_sequence[auth.actor] = next_auth_sequence( auth.actor );
   }

   action_trace t(r);
   t.trx_id = trx_context.id;
   t.act = act;
   t.console = _pending_console_output.str();
   t.return_value = trace.return_value;

   trx_context.executed.emplace_back( move(r) );

   if ( control.contracts_console() ) {
      print_debug(receiver, t);
   }

   reset_console();

   t.elapsed = fc::time_point::now() - start;
   return t;
}

void apply_context::exec()
{
   _notified.push_back(receiver);
   trace = exec_one();
   for( uint32_t i = 1; i < _notified.size(); ++i ) {
      receiver = _notified[i];
      trace.inline_traces.emplace_back( exec_one() );
   }

   if( _cfa_inline_actions.size() > 0 || _inline_actions.size() > 0 ) {
      ULTRAIN_ASSERT( recurse_depth < control.get_global_properties().configuration.max_inline_action_depth,
                  transaction_exception, "inline action recursion depth reached" );
   }

   for( const auto& inline_action : _cfa_inline_actions ) {
      trace.inline_traces.emplace_back();
      trx_context.dispatch_action( trace.inline_traces.back(), inline_action, inline_action.account, true, recurse_depth + 1 );
   }

   for( const auto& inline_action : _inline_actions ) {
      trace.inline_traces.emplace_back();
      trx_context.dispatch_action( trace.inline_traces.back(), inline_action, inline_action.account, false, recurse_depth + 1 );
   }

} /// exec()

bool apply_context::is_account( const account_name& account )const {
   return nullptr != db.find<account_object,by_name>( account );
}

void apply_context::require_authorization( const account_name& account ) {
   for( uint32_t i=0; i < act.authorization.size(); i++ ) {
     if( act.authorization[i].actor == account ) {
        used_authorizations[i] = true;
        return;
     }
   }
   ULTRAIN_ASSERT( false, missing_auth_exception, "missing authority of ${account}", ("account",account));
}

bool apply_context::has_authorization( const account_name& account )const {
   for( const auto& auth : act.authorization )
     if( auth.actor == account )
        return true;
  return false;
}

void apply_context::require_authorization(const account_name& account,
                                          const permission_name& permission) {
  for( uint32_t i=0; i < act.authorization.size(); i++ )
     if( act.authorization[i].actor == account ) {
        if( act.authorization[i].permission == permission ) {
           used_authorizations[i] = true;
           return;
        }
     }
  ULTRAIN_ASSERT( false, missing_auth_exception, "missing authority of ${account}/${permission}",
              ("account",account)("permission",permission) );
}

bool apply_context::has_recipient( account_name code )const {
   for( auto a : _notified )
      if( a == code )
         return true;
   return false;
}

void apply_context::require_recipient( account_name recipient ) {
   if( !has_recipient(recipient) ) {
      _notified.push_back(recipient);
   }
}

void apply_context::update_action_ability(action& a) {
   auto* code = control.db().find<account_object, by_name>(a.account);
   ULTRAIN_ASSERT( code != nullptr, action_validate_exception,
               "inline action's code account ${account} does not exist", ("account", a.account) );
   // if current action is a pureview action, then all inline or deferred action must be pureview.
   if (act.ability == action::PureView) {
      a.ability = action::PureView;
   } else {
      const auto& actionDefs = code->get_abi().actions;
      const auto& t = std::find_if(actionDefs.begin(), actionDefs.end(), [&](const action_def& ad) {
         return a.name == ad.name;
      });
      ULTRAIN_ASSERT( t != actionDefs.end(), action_validate_exception,
                     "account ${account} does not define action '${action}'",
                     ("account", a.account)("action", a.name) );
      a.ability = (t->ability == "normal") ? (action::Normal) : (action::PureView);
   }
}
/**
 *  This will execute an action after checking the authorization. Inline transactions are
 *  implicitly authorized by the current receiver (running code). This method has significant
 *  security considerations and several options have been considered:
 *
 *  1. priviledged accounts (those marked as such by block producers) can authorize any action
 *  2. all other actions are only authorized by 'receiver' which means the following:
 *         a. the user must set permissions on their account to allow the 'receiver' to act on their behalf
 *
 *  Discarded Implemenation:  at one point we allowed any account that authorized the current transaction
 *   to implicitly authorize an inline transaction. This approach would allow privelege escalation and
 *   make it unsafe for users to interact with certain contracts.  We opted instead to have applications
 *   ask the user for permission to take certain actions rather than making it implicit. This way users
 *   can better understand the security risk.
 */
void apply_context::execute_inline( action&& a ) {
   update_action_ability(a);
   bool enforce_actor_whitelist_blacklist = trx_context.enforce_whiteblacklist && control.is_producing_block();
   flat_set<account_name> actors;

   bool disallow_send_to_self_bypass = false; // eventually set to whether the appropriate protocol feature has been activated
   bool send_to_self = (a.account == receiver);
   bool inherit_parent_authorizations = (!disallow_send_to_self_bypass && send_to_self && (receiver == act.account) && control.is_producing_block());

   flat_set<permission_level> inherited_authorizations;
   if( inherit_parent_authorizations ) {
      inherited_authorizations.reserve( a.authorization.size() );
   }

   for( const auto& auth : a.authorization ) {
      auto* actor = control.db().find<account_object, by_name>(auth.actor);
      ULTRAIN_ASSERT( actor != nullptr, action_validate_exception,
                  "inline action's authorizing actor ${account} does not exist", ("account", auth.actor) );
      ULTRAIN_ASSERT( control.get_authorization_manager().find_permission(auth) != nullptr, action_validate_exception,
                  "inline action's authorizations include a non-existent permission: ${permission}",
                  ("permission", auth) );
      if( enforce_actor_whitelist_blacklist )
         actors.insert( auth.actor );

      if( inherit_parent_authorizations && std::find(act.authorization.begin(), act.authorization.end(), auth) != act.authorization.end() ) {
         inherited_authorizations.insert( auth );
      }
   }

   if( enforce_actor_whitelist_blacklist ) {
      control.check_actor_list( actors );
   }

   // No need to check authorization if replaying irreversible blocks or contract is privileged
   if( !control.skip_auth_check() && !privileged ) {
      try {
         control.get_authorization_manager()
                .check_authorization( {a},
                                      {},
                                      {{receiver, config::ultrainio_code_name}},
                                      control.pending_block_time() - trx_context.published,
                                      std::bind(&transaction_context::checktime, &this->trx_context),
                                      false,
                                      inherited_authorizations
                                    );

         //QUESTION: Is it smart to allow a deferred transaction that has been delayed for some time to get away
         //          with sending an inline action that requires a delay even though the decision to send that inline
         //          action was made at the moment the deferred transaction was executed with potentially no forewarning?
      } catch( const fc::exception& e ) {
         if( disallow_send_to_self_bypass || !send_to_self ) {
            throw;
         } else if( control.is_producing_block() ) {
            subjective_block_production_exception new_exception(FC_LOG_MESSAGE( error, "Authorization failure with inline action sent to self"));
            for (const auto& log: e.get_log()) {
               new_exception.append_log(log);
            }
            throw new_exception;
         }
      } catch( ... ) {
         if( disallow_send_to_self_bypass || !send_to_self ) {
            throw;
         } else if( control.is_producing_block() ) {
            ULTRAIN_THROW(subjective_block_production_exception, "Unexpected exception occurred validating inline action sent to self");
         }
      }
   }

   _inline_actions.emplace_back( move(a) );
}

void apply_context::execute_context_free_inline( action&& a ) {
   update_action_ability(a);
   ULTRAIN_ASSERT( a.authorization.size() == 0, action_validate_exception,
               "context-free actions cannot have authorizations" );

   _cfa_inline_actions.emplace_back( move(a) );
}


void apply_context::schedule_deferred_transaction( const uint128_t& sender_id, account_name payer, transaction&& trx, bool replace_existing ) {
   ULTRAIN_ASSERT(act.account == config::system_account_name || act.account == N(utrio.msig) || act.account == config::resource_account_name, action_validate_exception, "only ultrainio,utrio.msig,utrio.res contract can send deferred transaction" );
   ULTRAIN_ASSERT( trx.context_free_actions.size() == 0, cfa_inside_generated_tx, "context free actions are not currently allowed in generated transactions" );
   trx.expiration = control.pending_block_time() + fc::microseconds(999'999); // Rounds up to nearest second (makes expiration check unnecessary)
   trx.set_reference_block(control.head_block_id()); // No TaPoS check necessary

   bool enforce_actor_whitelist_blacklist = trx_context.enforce_whiteblacklist && control.is_producing_block();
   trx_context.validate_referenced_accounts( trx, enforce_actor_whitelist_blacklist );

   // Charge ahead of time for the additional net usage needed to retire the deferred transaction
   // whether that be by successfully executing, soft failure, hard failure, or expiration.
   const auto& cfg = control.get_global_properties().configuration;
   trx_context.add_net_usage( static_cast<uint64_t>(cfg.base_per_transaction_net_usage)
                               + static_cast<uint64_t>(config::transaction_id_net_usage) ); // Will exit early if net usage cannot be payed.

   auto delay = fc::seconds(trx.delay_sec);

   if( !control.skip_auth_check() && !privileged ) { // Do not need to check authorization if replayng irreversible block or if contract is privileged
      if( payer != receiver ) {
         require_authorization(payer); /// uses payer's storage
      }

      // Originally this code bypassed authorization checks if a contract was deferring only actions to itself.
      // The idea was that the code could already do whatever the deferred transaction could do, so there was no point in checking authorizations.
      // But this is not true. The original implementation didn't validate the authorizations on the actions which allowed for privilege escalation.
      // It would make it possible to bill RAM to some unrelated account.
      // Furthermore, even if the authorizations were forced to be a subset of the current action's authorizations, it would still violate the expectations
      // of the signers of the original transaction, because the deferred transaction would allow billing more CPU and network bandwidth than the maximum limit
      // specified on the original transaction.
      // So, the deferred transaction must always go through the authorization checking if it is not sent by a privileged contract.
      // However, the old logic must still be considered because it cannot objectively change until a consensus protocol upgrade.

      bool disallow_send_to_self_bypass = false; // eventually set to whether the appropriate protocol feature has been activated

      auto is_sending_only_to_self = [&trx]( const account_name& self ) {
         bool send_to_self = true;
         for( const auto& act : trx.actions ) {
            if( act.account != self ) {
               send_to_self = false;
               break;
            }
         }
         return send_to_self;
      };

      try {
         control.get_authorization_manager()
                .check_authorization( trx.actions,
                                      {},
                                      {{receiver, config::ultrainio_code_name}},
                                      delay,
                                      std::bind(&transaction_context::checktime, &this->trx_context),
                                      false
                                    );
      } catch( const fc::exception& e ) {
         if( disallow_send_to_self_bypass || !is_sending_only_to_self(receiver) ) {
            throw;
         } else if( control.is_producing_block() ) {
            subjective_block_production_exception new_exception(FC_LOG_MESSAGE( error, "Authorization failure with sent deferred transaction consisting only of actions to self"));
            for (const auto& log: e.get_log()) {
               new_exception.append_log(log);
            }
            throw new_exception;
         }
      } catch( ... ) {
         if( disallow_send_to_self_bypass || !is_sending_only_to_self(receiver) ) {
            throw;
         } else if( control.is_producing_block() ) {
            ULTRAIN_THROW(subjective_block_production_exception, "Unexpected exception occurred validating sent deferred transaction consisting only of actions to self");
         }
      }
   }

   uint32_t trx_size = 0;
   auto& d = control.db();
   if ( auto ptr = d.find<generated_transaction_object,by_sender_id>(boost::make_tuple(receiver, sender_id)) ) {
      ULTRAIN_ASSERT( replace_existing, deferred_tx_duplicate, "deferred transaction with the same sender_id and payer already exists" );

      // TODO: Remove the following subjective check when the deferred trx replacement RAM bug has been fixed with a hard fork.
      ULTRAIN_ASSERT( !control.is_producing_block(), subjective_block_production_exception,
                  "Replacing a deferred transaction is temporarily disabled." );

      // TODO: The logic of the next line needs to be incorporated into the next hard fork.
      // trx_context.add_ram_usage( ptr->payer, -(config::billable_size_v<generated_transaction_object> + ptr->packed_trx.size()) );

      transaction_id_type trx_id_for_new_obj;
      trx_id_for_new_obj = ptr->trx_id;

      d.remove( *ptr );
      d.create<generated_transaction_object>([&]( auto& gtx ) {
            gtx.trx_id      = trx_id_for_new_obj;
            gtx.sender      = receiver;
            gtx.sender_id   = sender_id;
            gtx.payer       = payer;
            gtx.published   = control.pending_block_time();
            gtx.delay_until = gtx.published + delay;
            gtx.expiration  = gtx.delay_until + fc::seconds(control.get_global_properties().configuration.deferred_trx_expiration_window);

            trx_size = gtx.set( trx );
         });
   } else {
      d.create<generated_transaction_object>( [&]( auto& gtx ) {
            gtx.trx_id      = trx.id();
            gtx.sender      = receiver;
            gtx.sender_id   = sender_id;
            gtx.payer       = payer;
            gtx.published   = control.pending_block_time();
            gtx.delay_until = gtx.published + delay;
            gtx.expiration  = gtx.delay_until + fc::seconds(control.get_global_properties().configuration.deferred_trx_expiration_window);

            trx_size = gtx.set( trx );
         });
   }

   trx_context.add_ram_usage( payer, (config::billable_size_v<generated_transaction_object> + trx_size) );
}

bool apply_context::cancel_deferred_transaction( const uint128_t& sender_id, account_name sender ) {
   auto& generated_transaction_idx = db.get_mutable_index<generated_transaction_multi_index>();
   const auto* gto = db.find<generated_transaction_object,by_sender_id>(boost::make_tuple(sender, sender_id));
   if ( gto ) {
      trx_context.add_ram_usage( gto->payer, -(config::billable_size_v<generated_transaction_object> + gto->packed_trx.size()) );
      generated_transaction_idx.remove(*gto);
   }
   return gto;
}

const table_id_object* apply_context::find_table( name code, name scope, name table ) {
   return db.find<table_id_object, by_code_scope_table>(boost::make_tuple(code, scope, table));
}

const table_id_object& apply_context::find_or_create_table( name code, name scope, name table, const account_name &payer ) {
   const auto* existing_tid =  db.find<table_id_object, by_code_scope_table>(boost::make_tuple(code, scope, table));
   if (existing_tid != nullptr) {
      return *existing_tid;
   }

   update_db_usage(payer, config::billable_size_v<table_id_object>);

   return db.create<table_id_object>([&](table_id_object &t_id){
      t_id.code = code;
      t_id.scope = scope;
      t_id.table = table;
      t_id.payer = payer;
   });
}

void apply_context::remove_table(account_name payer, const table_id_object& tid ) {
   update_db_usage(payer, - config::billable_size_v<table_id_object>);
   db.remove(tid);
}

void apply_context::reset_console() {
   _pending_console_output = std::ostringstream();
   _pending_console_output.setf( std::ios::scientific, std::ios::floatfield );
}

bytes apply_context::get_packed_transaction() {
   auto r = fc::raw::pack( static_cast<const transaction&>(trx_context.trx) );
   return r;
}

void apply_context::update_db_usage( const account_name& payer, int64_t delta ) {
   auto p = receiver;
   //if(delta < 0 && (receiver == N(ultrainio) || receiver == config::resource_account_name)){
   if(delta < 0 && receiver == N(ultrainio) ) {
      p = payer;
   }
//    if( delta > 0 ) {
//       if( !(privileged || payer == account_name(receiver)) ) {
//          require_authorization( payer );
//       }
//    }
   trx_context.add_ram_usage( p, delta);
}


int apply_context::get_action( uint32_t type, uint32_t index, char* buffer, size_t buffer_size )const
{
   const auto& trx = trx_context.trx;
   const action* act_ptr = nullptr;

   if( type == 0 ) {
      if( index >= trx.context_free_actions.size() )
         return -1;
      act_ptr = &trx.context_free_actions[index];
   }
   else if( type == 1 ) {
      if( index >= trx.actions.size() )
         return -1;
      act_ptr = &trx.actions[index];
   }

   ULTRAIN_ASSERT(act_ptr, action_not_found_exception, "action is not found" );

   auto ps = fc::raw::pack_size( *act_ptr );
   if( ps <= buffer_size ) {
      fc::datastream<char*> ds(buffer, buffer_size);
      fc::raw::pack( ds, *act_ptr );
   }
   return ps;
}

int apply_context::get_context_free_data( uint32_t index, char* buffer, size_t buffer_size )const
{
   const auto& trx = trx_context.trx;

   if( index >= trx.context_free_data.size() ) return -1;

   auto s = trx.context_free_data[index].size();
   if( buffer_size == 0 ) return s;

   auto copy_size = std::min( buffer_size, s );
   memcpy( buffer, trx.context_free_data[index].data(), copy_size );

   return copy_size;
}

void apply_context::check_rw_db_ability() const {
   // dlog("check_rw_db_ability: receiver = ${receiver}, action.account = ${account}, action.name = ${name}, action.ability = ${ability}", ("receiver", receiver)("account", act.account)("name", act.name)("ability", act.ability));
   ULTRAIN_ASSERT( act.ability == action::Normal, table_access_violation, "pureview action can not store, modify or erase item(s) from table.");
}

int apply_context::db_store_i64( uint64_t scope, uint64_t table, const account_name& payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   return db_store_i64( receiver, scope, table, payer, id, buffer, buffer_size);
}

int apply_context::db_store_i64( uint64_t code, uint64_t scope, uint64_t table, account_name payer, uint64_t id, const char* buffer, size_t buffer_size ) {
//   require_write_lock( scope );
   payer = code;
   const auto& tab = find_or_create_table( code, scope, table, payer );
   auto tableid = tab.id;

   ULTRAIN_ASSERT( payer != account_name(), invalid_table_payer, "must specify a valid account to pay for new record" );

   const auto& obj = db.create<key_value_object>( [&]( auto& o ) {
      o.t_id        = tableid;
      o.primary_key = id;
      o.value.resize( buffer_size );
      o.payer       = payer;
      memcpy( o.value.data(), buffer, buffer_size );
   });

   db.modify( tab, [&]( auto& t ) {
     ++t.count;
   });

   int64_t billable_size = (int64_t)(buffer_size + config::billable_size_v<key_value_object>);
   update_db_usage( payer, billable_size);

   keyval_cache.cache_table( tab );
   return keyval_cache.add( obj );
}

void apply_context::db_update_i64( int iterator, account_name payer, const char* buffer, size_t buffer_size ) {
   const key_value_object& obj = keyval_cache.get( iterator );
   payer = receiver;
   const auto& table_obj = keyval_cache.get_table( obj.t_id );
   ULTRAIN_ASSERT( table_obj.code == receiver, table_access_violation, "db access violation" );

//   require_write_lock( table_obj.scope );

   const int64_t overhead = config::billable_size_v<key_value_object>;
   int64_t old_size = (int64_t)(obj.value.size() + overhead);
   int64_t new_size = (int64_t)(buffer_size + overhead);

   if( payer == account_name() ) payer = obj.payer;

   if( account_name(obj.payer) != payer ) {
      // refund the existing payer
      update_db_usage( obj.payer,  -(old_size) );
      // charge the new payer
      update_db_usage( payer,  (new_size));
   } else if(old_size != new_size) {
      // charge/refund the existing payer the difference
      update_db_usage( obj.payer, new_size - old_size);
   }

   db.modify( obj, [&]( auto& o ) {
     o.value.resize( buffer_size );
     memcpy( o.value.data(), buffer, buffer_size );
     o.payer = payer;
   });
}

void apply_context::db_remove_i64( int iterator ) {
   const key_value_object& obj = keyval_cache.get( iterator );

   const auto& table_obj = keyval_cache.get_table( obj.t_id );
   ULTRAIN_ASSERT( table_obj.code == receiver, table_access_violation, "db access violation" );

//   require_write_lock( table_obj.scope );

   update_db_usage( receiver,  -(obj.value.size() + config::billable_size_v<key_value_object>) );

   db.modify( table_obj, [&]( auto& t ) {
      --t.count;
   });
   db.remove( obj );

   if (table_obj.count == 0) {
      remove_table(receiver, table_obj);
   }

   keyval_cache.remove( iterator );
}

int apply_context::db_get_i64( int iterator, char* buffer, size_t buffer_size ) {
   const key_value_object& obj = keyval_cache.get( iterator );

   auto s = obj.value.size();
   if( buffer_size == 0 ) return s;

   auto copy_size = std::min( buffer_size, s );
   memcpy( buffer, obj.value.data(), copy_size );

   return copy_size;
}

int apply_context::db_next_i64( int iterator, uint64_t& primary ) {
   if( iterator < -1 ) return -1; // cannot increment past end iterator of table

   const auto& obj = keyval_cache.get( iterator ); // Check for iterator != -1 happens in this call
   const auto& idx = db.get_index<key_value_index, by_scope_primary>();

   auto itr = idx.iterator_to( obj );
   ++itr;

   if( itr == idx.end() || itr->t_id != obj.t_id ) return keyval_cache.get_end_iterator_by_table_id(obj.t_id);

   primary = itr->primary_key;
   return keyval_cache.add( *itr );
}

int apply_context::db_previous_i64( int iterator, uint64_t& primary ) {
   const auto& idx = db.get_index<key_value_index, by_scope_primary>();

   if( iterator < -1 ) // is end iterator
   {
      auto tab = keyval_cache.find_table_by_end_iterator(iterator);
      ULTRAIN_ASSERT( tab, invalid_table_iterator, "not a valid end iterator" );

      auto itr = idx.upper_bound(tab->id);
      if( idx.begin() == idx.end() || itr == idx.begin() ) return -1; // Empty table

      --itr;

      if( itr->t_id != tab->id ) return -1; // Empty table

      primary = itr->primary_key;
      return keyval_cache.add(*itr);
   }

   const auto& obj = keyval_cache.get(iterator); // Check for iterator != -1 happens in this call

   auto itr = idx.iterator_to(obj);
   if( itr == idx.begin() ) return -1; // cannot decrement past beginning iterator of table

   --itr;

   if( itr->t_id != obj.t_id ) return -1; // cannot decrement past beginning iterator of table

   primary = itr->primary_key;
   return keyval_cache.add(*itr);
}

int apply_context::db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   //require_read_lock( code, scope ); // redundant?

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const key_value_object* obj = db.find<key_value_object, by_scope_primary>( boost::make_tuple( tab->id, id ) );
   if( !obj ) return table_end_itr;

   return keyval_cache.add( *obj );
}

int apply_context::db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   //require_read_lock( code, scope ); // redundant?

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const auto& idx = db.get_index<key_value_index, by_scope_primary>();
   auto itr = idx.lower_bound( boost::make_tuple( tab->id, id ) );
   if( itr == idx.end() ) return table_end_itr;
   if( itr->t_id != tab->id ) return table_end_itr;

   return keyval_cache.add( *itr );
}

int apply_context::db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
   //require_read_lock( code, scope ); // redundant?

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const auto& idx = db.get_index<key_value_index, by_scope_primary>();
   auto itr = idx.upper_bound( boost::make_tuple( tab->id, id ) );
   if( itr == idx.end() ) return table_end_itr;
   if( itr->t_id != tab->id ) return table_end_itr;

   return keyval_cache.add( *itr );
}

int apply_context::db_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
   //require_read_lock( code, scope ); // redundant?

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   return keyval_cache.cache_table( *tab );
}

uint64_t apply_context::db_iterator_i64(uint64_t code, uint64_t scope, uint64_t table) {
   uint64_t result = 0ULL;
   const auto* tab = find_table( code, scope, table );
   if( !tab ) return result;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const auto* t_id = db.find<table_id_object, by_code_scope_table>(boost::make_tuple(code, scope, table));
   if (t_id != nullptr) {
       const auto& idx = db.get_index<key_value_index, by_scope_primary>();
       decltype(t_id->id) next_tid(t_id->id._id + 1);
       auto lower = idx.lower_bound(boost::make_tuple(t_id->id));
       auto upper = idx.lower_bound(boost::make_tuple(next_tid));

       uint32_t count = std::distance(lower, upper);

       auto first = keyval_cache.add(*lower);
       std::for_each(++lower, upper, [&](auto& item) {
         keyval_cache.add(item);
       });

       result |= count;
       result = (result << 32) | first;
   }
   return result;
}

int apply_context::db_iterator_i64_v2(uint64_t code, uint64_t scope, uint64_t table, char* buffer, size_t buffer_size) {
   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const auto* t_id = db.find<table_id_object, by_code_scope_table>(boost::make_tuple(code, scope, table));
   if (t_id != nullptr) {
       const auto& idx = db.get_index<key_value_index, by_scope_primary>();
       decltype(t_id->id) next_tid(t_id->id._id + 1);
       auto lower = idx.lower_bound(boost::make_tuple(t_id->id));
       auto upper = idx.lower_bound(boost::make_tuple(next_tid));

       //  uint32_t count = std::distance(lower, upper);
       std::vector<int> pos;
       std::for_each(lower, upper, [&](auto& item) {
         auto k = keyval_cache.add(item);
         pos.push_back(k);
       });

       size_t capacity = fc::raw::pack_size(pos);
       if (buffer_size < capacity) return capacity;

       fc::datastream<char *> ds(buffer, buffer_size);
       fc::raw::pack(ds, pos);
       return 0;
   }
   return -1;
}

int apply_context::db_counts_i64(uint64_t code, uint64_t scope, uint64_t table) {
   int count = 0;
   const auto* tab = find_table( code, scope, table );
   if( !tab ) return count;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const auto* t_id = db.find<table_id_object, by_code_scope_table>(boost::make_tuple(code, scope, table));
   if (t_id != nullptr) {
       const auto& idx = db.get_index<key_value_index, by_scope_primary>();
       decltype(t_id->id) next_tid(t_id->id._id + 1);
       auto lower = idx.lower_bound(boost::make_tuple(t_id->id));
       auto upper = idx.lower_bound(boost::make_tuple(next_tid));

       count = std::distance(lower, upper);
   }
   return count;
}

template<typename IndexType, typename ObjectType>
int apply_context::db_drop_secondary_index(const ultrainio::chain::table_id_object * t_id, uint64_t& calc_num, uint64_t& calc_delram_bytes) {
   if(!t_id || ((calc_num > calc_num_limit || calc_delram_bytes > calc_delram_bytes_limit)) ) {
       dlog("calc_num ${calc_num} or calc_delram_bytes ${calc_delram_bytes} beyond the limit",("calc_num", calc_num)("calc_delram_bytes", calc_delram_bytes));
       return -1;
   }
   account_name systemname(config::system_account_name);
   account_name resourcename(config::resource_account_name);
   ULTRAIN_ASSERT( systemname == receiver || resourcename == receiver, table_access_violation, "db access violation" );
   const auto& idx = db.template get_index<IndexType, by_primary>();
   decltype(t_id->id) next_tid(t_id->id._id + 1);
   auto lower = idx.lower_bound(boost::make_tuple(t_id->id, 0));
   auto upper = idx.lower_bound(boost::make_tuple(next_tid, 0));

   uint32_t count = std::distance(lower, upper);
   uint32_t i = 0;
   if(lower == upper) return 0;

   int64_t usage_delta = 0LL;
   while(i++<count) {
      auto &obj = *lower;
      lower++;
      if(obj.t_id != t_id->id) {
           dlog("tid error ${o_tid}, ${tid}", ("o_tid", obj.t_id)("tid", t_id->id));
           continue;
       }
      usage_delta = config::billable_size_v<ObjectType>;
      calc_delram_bytes += usage_delta;
      update_db_usage( obj.payer, -( config::billable_size_v<ObjectType> ));
      db.remove(obj);
      calc_num++;
      if(calc_num > calc_num_limit || calc_delram_bytes > calc_delram_bytes_limit){
          return -1;
      }
   }
   return 0;
}
int apply_context::db_drop_i64(uint64_t code, uint64_t scope, uint64_t table) {
   const auto* table_obj = find_table( code, scope, table );
   if( !table_obj ) return -1;
   ULTRAIN_ASSERT( table_obj->code == receiver, table_access_violation, "db access violation" );

   const auto& idx = db.get_index<key_value_index, by_scope_primary>();
   decltype(table_obj->id) next_tid(table_obj->id._id + 1);
   auto lower = idx.lower_bound(boost::make_tuple(table_obj->id));
   auto upper = idx.lower_bound(boost::make_tuple(next_tid));

   if (lower == upper) return 0;

   auto count = std::distance(lower, upper);
   for (const auto& it = boost::make_reverse_iterator(upper); count > 0; --count) {
      const key_value_object& obj = *it;
      int64_t usage_delta = (obj.value.size() + config::billable_size_v<key_value_object>);
      update_db_usage( receiver,  -(usage_delta) );
      db.remove(obj);
   };

   keyval_cache.purge_table_cache(table_obj->id);
   remove_table(receiver,*table_obj);

   return 0;
}
int apply_context::db_drop_table(uint64_t code) {
   const auto&  table_idx = db.get_index<table_id_multi_index , by_code_scope_table>();
   account_name systemname(config::system_account_name);
   account_name resourcename(config::resource_account_name);
   ULTRAIN_ASSERT( systemname == receiver ||  resourcename == receiver, table_access_violation, "db access violation" );
   auto table_lower = table_idx.lower_bound(boost::make_tuple(code, 0, 0));
   auto table_upper = table_idx.lower_bound(boost::make_tuple(code+1, 0, 0));
   const auto& idx = db.get_index<key_value_index, by_scope_primary>();
   int res = -1;

   uint32_t count_t = std::distance(table_lower, table_upper);
   uint32_t i_t = 0;
   uint64_t calc_num = 0;
   uint64_t calc_delram_bytes = 0;
   while(i_t++<count_t) {
       calc_num++;
       if(calc_num > calc_num_limit || calc_delram_bytes > calc_delram_bytes_limit){
          dlog("db_drop_table:scope i_t = ${i_t}, calc_num = ${calc_num}, calc_delram_bytes = ${calc_delram_bytes}, code = ${code}",
              ("i_t", i_t)("calc_num", calc_num)("calc_delram_bytes", calc_delram_bytes)("code", name{code}.to_string()));
          return -1;
       }
       auto & table_obj = *table_lower;
       table_lower++;
       if(table_obj.code != code) {
           dlog("code error ${t_code}, ${code}", ("t_code", name{table_obj.code}.to_string())("code", name{code}.to_string()));
           continue;
       }
       decltype(table_obj.id) next_tid(table_obj.id._id + 1);
       auto lower = idx.lower_bound(boost::make_tuple(table_obj.id));
       auto upper = idx.lower_bound(boost::make_tuple(next_tid));

       uint32_t count = std::distance(lower, upper);
       uint32_t i = 0;

       int64_t usage_delta = 0LL;
       while(i++<count) {
          calc_num++;
          if(calc_num > calc_num_limit || calc_delram_bytes > calc_delram_bytes_limit){
              dlog("db_drop_table: lower_bound i = ${i}，i_t = ${i_t}, count = ${count}, calc_num = ${calc_num}, calc_delram_bytes = ${calc_delram_bytes}, code = ${code}",
                  ("i", i)("i_t", i_t)("count", count)("calc_num", calc_num)("calc_delram_bytes", calc_delram_bytes)("code", name{code}.to_string()));
              return -1;
          }

          auto &obj = *lower;
          lower++;
          if(obj.t_id != table_obj.id) {
              dlog("tid error ${o_tid}, ${tid}", ("o_tid", obj.t_id)("tid", table_obj.id));
              continue;
          }
          usage_delta = (obj.value.size() + config::billable_size_v<key_value_object>);
          calc_delram_bytes += usage_delta;
          update_db_usage( code,  -(usage_delta) );
          db.remove(obj);
       }
       res = db_drop_secondary_index<index64_index,index64_object>(&table_obj, calc_num, calc_delram_bytes);
       res = db_drop_secondary_index<index128_index,index128_object>(&table_obj, calc_num, calc_delram_bytes);
       res = db_drop_secondary_index<index256_index,index256_object>(&table_obj, calc_num, calc_delram_bytes);
       res = db_drop_secondary_index<index_double_index,index_double_object>(&table_obj, calc_num, calc_delram_bytes);
       res = db_drop_secondary_index<index_long_double_index,index_long_double_object>(&table_obj, calc_num, calc_delram_bytes);
       if(res == -1) {
           return -1;
       }
       calc_delram_bytes += config::billable_size_v<table_id_object>;
       remove_table(code, table_obj);
   }
   return 0;
}

uint64_t apply_context::next_global_sequence() {
   const auto& p = control.get_dynamic_global_properties();
   db.modify( p, [&]( auto& dgp ) {
      ++dgp.global_action_sequence;
   });
   return p.global_action_sequence;
}

uint64_t apply_context::next_recv_sequence( account_name receiver ) {
    auto const* sequence_obj_itr = db.find<account_sequence_object, by_name>(receiver);
    if( !sequence_obj_itr ){
       db.create<account_sequence_object>([&](auto & a) {
          a.name = receiver;
          a.recv_sequence++;
       });
       return 1;
    } else {
       db.modify( *sequence_obj_itr, [&]( auto& mrs ) {
          ++mrs.recv_sequence;
       });
       return sequence_obj_itr->recv_sequence;
    }
}
uint64_t apply_context::next_auth_sequence( account_name actor ) {
    auto const* auth_sequence_obj_itr = db.find<auth_sequence_object, by_name>(actor);
    if( !auth_sequence_obj_itr ){
       db.create<auth_sequence_object>([&](auto & a) {
          a.name = actor;
          ++a.auth_sequence;
       });
       return 1;
    } else {
       db.modify( *auth_sequence_obj_itr, [&](auto& a ){
          if( a.auth_sequence >= std::numeric_limits<uint8_t>::max() )
             a.auth_sequence = 0;
          else
             ++a.auth_sequence;
       });
       return auth_sequence_obj_itr->auth_sequence;
    }
}


} } /// ultrainio::chain
