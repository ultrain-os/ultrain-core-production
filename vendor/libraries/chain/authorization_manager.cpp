/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include <ultrainio/chain/authorization_manager.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <ultrainio/chain/permission_object.hpp>
#include <ultrainio/chain/permission_link_object.hpp>
#include <ultrainio/chain/authority_checker.hpp>
#include <ultrainio/chain/controller.hpp>
#include <ultrainio/chain/global_property_object.hpp>
#include <ultrainio/chain/contract_types.hpp>
#include <ultrainio/chain/generated_transaction_object.hpp>
#include <ultrainio/chain/worldstate_file_manager.hpp>

namespace ultrainio { namespace chain {
   using authorization_index_set = index_set<
      permission_index,
      permission_usage_index,
      permission_link_index
   >;

   authorization_manager::authorization_manager(controller& c, database& d)
   :_control(c),_db(d){}

   void authorization_manager::add_indices(chainbase::database& db) {
      authorization_index_set::add_indices(db);
   }

   void authorization_manager::initialize_database() {
      _db.create<permission_object>([](auto&){}); /// reserve perm 0 (used else where)
   }

   namespace detail {
      template<>
      struct worldstate_row_traits<permission_object> {
         using value_type = permission_object;
         using worldstate_type = worldstate_permission_object;

         static worldstate_permission_object to_worldstate_row(const permission_object& value, const chainbase::database& db, void* data) {
            worldstate_permission_object res;
            res.name = value.name;
            res.owner = value.owner;
            res.last_updated = value.last_updated;
            res.auth = value.auth.to_authority();

            // lookup parent name
            const auto& idx = db.get_index<permission_index>().backup_indices();
            auto itr = idx.find( value.parent );
            ULTRAIN_ASSERT( itr != idx.end(), worldstate_exception, "Not find the expected parent: ${s}", ("s", value.parent._id));

            const auto& parent = *itr;
            res.parent = parent.name;

            // lookup the usage object
            if(value.id != 0){
               const auto& idx_usage = db.get_index<permission_usage_index>().backup_indices();
               auto itr_usage = idx_usage.find( value.usage_id );
               ULTRAIN_ASSERT( itr_usage != idx_usage.end(), worldstate_exception, "Not find the expected idx_usage: ${s}", ("s", value.usage_id._id));

               const auto& usage = *itr_usage;
               res.last_used = usage.last_used;

               ULTRAIN_ASSERT( data != nullptr, worldstate_exception, "Data is nullptr, request data!");
               ws_helper* ptr = (ws_helper*)data;
               ptr->get_id_writer()->write_row_id(usage.id._id, 0);
            } else {
               res.last_used = time_point();
            }
            return res;
         }

         static void from_worldstate_row(worldstate_permission_object&& row, permission_object& value, chainbase::database& db, bool backup, void* data = nullptr) {
            value.name = row.name;
            value.owner = row.owner;
            value.last_updated = row.last_updated;
            value.auth = row.auth;
            value.parent = 0;

            if (value.id == 0) {
               ULTRAIN_ASSERT(row.parent == permission_name(), worldstate_exception, "Unexpected parent name on reserved permission 0");
               ULTRAIN_ASSERT(row.name == permission_name(), worldstate_exception, "Unexpected permission name on reserved permission 0");
               ULTRAIN_ASSERT(row.owner == name(), worldstate_exception, "Unexpected owner name on reserved permission 0");
               ULTRAIN_ASSERT(row.auth.accounts.size() == 0,  worldstate_exception, "Unexpected auth accounts on reserved permission 0");
               ULTRAIN_ASSERT(row.auth.keys.size() == 0,  worldstate_exception, "Unexpected auth keys on reserved permission 0");
               ULTRAIN_ASSERT(row.auth.waits.size() == 0,  worldstate_exception, "Unexpected auth waits on reserved permission 0");
               ULTRAIN_ASSERT(row.auth.threshold == 0,  worldstate_exception, "Unexpected auth threshold on reserved permission 0");
               ULTRAIN_ASSERT(row.last_updated == time_point(),  worldstate_exception, "Unexpected auth last updated on reserved permission 0");
               value.parent = 0;
            } else if ( row.parent != permission_name()){
               if(!backup) {
                  const auto& parent = db.get<permission_object, by_owner>(boost::make_tuple(row.owner, row.parent));

                  ULTRAIN_ASSERT(parent.id != 0, worldstate_exception, "Unexpected mapping to reserved permission 0");
                  value.parent = parent.id;
               } else {
                  const auto& idx = db.get_index<permission_index>().backup_indices().template get<by_owner>();
                  auto itr = idx.find(boost::make_tuple(row.owner, row.parent));
                  ULTRAIN_ASSERT( itr != idx.end(), worldstate_exception, "Not find the expected parent: ${s}", ("s", row.parent));

                  const auto& parent = *itr;
                  ULTRAIN_ASSERT(parent.id != 0, worldstate_exception, "Unexpected mapping to reserved permission 0");
                  value.parent = parent.id;
               }
            }

            if (value.id != 0) {
               ULTRAIN_ASSERT( data != nullptr, worldstate_exception, "Data is nullptr, request data!");
               ws_helper* ptr = (ws_helper*)data;

               if(!backup) {// create the usage object
                  const auto& usage = db.create<permission_usage_object>([&](auto& p) {
                     p.last_used = row.last_used;
                  });
                  ptr->get_id_writer()->write_row_id(usage.id._id, 0);
                  value.usage_id = usage.id;
               } else {
                  uint64_t old_id = 0, size = 0;
                  ptr->get_id_reader()->read_id_row(old_id, size);
                  const auto& usage = db.backup_create<permission_usage_object>([&](auto& p) {
                     p.last_used = row.last_used;
                     p.id._id = old_id;
                  });
                  value.usage_id = usage.id;
               }
            } else {
               value.usage_id = 0;
            }
         }
      };
   }

   void authorization_manager::add_to_worldstate( std::shared_ptr<ws_helper> ws_helper_ptr, chainbase::database& worldstate_db) {
      authorization_index_set::walk_indices([&]( auto utils ){
         using index_t = typename decltype(utils)::index_t;
         using value_t = typename index_t::value_type;

         // skip the permission_usage_index as its inlined with permission_index
         if (std::is_same<value_t, permission_usage_object>::value) {
            return;
         }

         auto& cache_node = worldstate_db.get_mutable_index<index_t>().cache().front();
         ilog("index: ${t}", ("t", boost::core::demangle(typeid(value_t).name())));
         ilog("remove/modify/create size: ${s} ${t} ${y}", ("s", cache_node.removed_ids.size())("t", cache_node.modify_values.size())("y", cache_node.new_values.size()));
         ilog("Cache count: ${s}", ("s", worldstate_db.get_mutable_index<index_t>().cache().size()));
         ilog("Backup size: ${s}", ("s", worldstate_db.get_mutable_index<index_t>().backup_indices().size()));

         //1:  add to backup if exit old ws file
         ws_helper_ptr->restore_backup_indices<index_t>(worldstate_db, true, (void*)ws_helper_ptr.get());

         //2:  squach backup and cache
         worldstate_db.get_mutable_index<index_t>().process_cache();
         if (std::is_same<value_t, permission_object>::value) {
            worldstate_db.get_mutable_index<permission_usage_index>().process_cache();
         }

         //3. read all record from backup, write to new ws file
         ws_helper_ptr->store_backup_indices<index_t>(worldstate_db, (void*)ws_helper_ptr.get());

         // 4. clear backup_indices
         (const_cast<index_t&>(worldstate_db.get_mutable_index<index_t>().backup_indices())).clear();
         if (std::is_same<value_t, permission_object>::value) {
            (const_cast<permission_usage_index&>(worldstate_db.get_mutable_index<permission_usage_index>().backup_indices())).clear();
         }
         ilog("done");
      });
   }

   void authorization_manager::read_from_worldstate( std::shared_ptr<ws_helper> ws_helper_ptr, chainbase::database& worldstate_db ) {
      authorization_index_set::walk_indices([&]( auto utils ){
         using index_t = typename decltype(utils)::index_t;
         using section_t = typename decltype(utils)::index_t::value_type;

         // skip the permission_usage_index as its inlined with permission_index
         if (std::is_same<section_t, permission_usage_object>::value) {
            return;
         }

         ws_helper_ptr->read_table_from_worldstate<index_t>(worldstate_db, (void*)ws_helper_ptr.get());
      });
   }


   const permission_object& authorization_manager::create_permission( account_name account,
                                                                      permission_name name,
                                                                      permission_id_type parent,
                                                                      const authority& auth,
                                                                      time_point initial_creation_time
                                                                    )
   {
      auto creation_time = initial_creation_time;
      if( creation_time == time_point() ) {
         creation_time = _control.pending_block_time();
      }

      const auto& perm_usage = _db.create<permission_usage_object>([&](auto& p) {
         p.last_used = creation_time;
      });

      const auto& perm = _db.create<permission_object>([&](auto& p) {
         p.usage_id     = perm_usage.id;
         p.parent       = parent;
         p.owner        = account;
         p.name         = name;
         p.last_updated = creation_time;
         p.auth         = auth;
      });
      return perm;
   }

   const permission_object& authorization_manager::create_permission( account_name account,
                                                                      permission_name name,
                                                                      permission_id_type parent,
                                                                      authority&& auth,
                                                                      time_point initial_creation_time
                                                                    )
   {
      auto creation_time = initial_creation_time;
      if( creation_time == time_point() ) {
         creation_time = _control.pending_block_time();
      }

      const auto& perm_usage = _db.create<permission_usage_object>([&](auto& p) {
         p.last_used = creation_time;
      });

      const auto& perm = _db.create<permission_object>([&](auto& p) {
         p.usage_id     = perm_usage.id;
         p.parent       = parent;
         p.owner        = account;
         p.name         = name;
         p.last_updated = creation_time;
         p.auth         = std::move(auth);
      });
      return perm;
   }

   void authorization_manager::modify_permission( const permission_object& permission, const authority& auth ) {
      _db.modify( permission, [&](permission_object& po) {
         po.auth = auth;
         po.last_updated = _control.pending_block_time();
      });
   }

   void authorization_manager::remove_permission( const permission_object& permission ) {
      const auto& index = _db.template get_index<permission_index, by_parent>();
      auto range = index.equal_range(permission.id);
      ULTRAIN_ASSERT( range.first == range.second, action_validate_exception,
                  "Cannot remove a permission which has children. Remove the children first.");

      _db.get_mutable_index<permission_usage_index>().remove_object( permission.usage_id._id );
      _db.remove( permission );
   }

   void authorization_manager::update_permission_usage( const permission_object& permission ) {
      const auto& puo = _db.get<permission_usage_object, by_id>( permission.usage_id );
      _db.modify( puo, [&](permission_usage_object& p) {
         p.last_used = _control.pending_block_time();
      });
   }

   fc::time_point authorization_manager::get_permission_last_used( const permission_object& permission )const {
      return _db.get<permission_usage_object, by_id>( permission.usage_id ).last_used;
   }

   const permission_object*  authorization_manager::find_permission( const permission_level& level )const
   { try {
      ULTRAIN_ASSERT( !level.actor.empty() && !level.permission.empty(), invalid_permission, "Invalid permission" );
      return _db.find<permission_object, by_owner>( boost::make_tuple(level.actor,level.permission) );
   } ULTRAIN_RETHROW_EXCEPTIONS( chain::permission_query_exception, "Failed to retrieve permission: ${level}", ("level", level) ) }

   const permission_object&  authorization_manager::get_permission( const permission_level& level )const
   { try {
      ULTRAIN_ASSERT( !level.actor.empty() && !level.permission.empty(), invalid_permission, "Invalid permission" );
      return _db.get<permission_object, by_owner>( boost::make_tuple(level.actor,level.permission) );
   } ULTRAIN_RETHROW_EXCEPTIONS( chain::permission_query_exception, "Failed to retrieve permission: ${level}", ("level", level) ) }

   const vector<permission_name>  authorization_manager::get_all_permission_name( const account_name& account )const
   { try {
      vector<permission_name> permission_list;
      const auto& permissions = _db.get_index<permission_index,by_owner>();
      auto perm = permissions.lower_bound( boost::make_tuple( account ) );
      while( perm != permissions.end() && perm->owner == account ) {
         name parent;
         if( perm->parent._id ) {
            const auto* p = _db.find<permission_object,by_id>( perm->parent );
            if( p ) {
               ULTRAIN_ASSERT(perm->owner == p->owner, invalid_parent_permission, "Invalid parent permission");
               parent = p->name;
            }
         }

         permission_list.push_back( perm->name );
         ++perm;
      }
      return permission_list;
   } ULTRAIN_RETHROW_EXCEPTIONS( chain::permission_query_exception, "Failed to retrieve permission account: ${account}", ("account", account) ) }

   optional<permission_name> authorization_manager::lookup_linked_permission( account_name authorizer_account,
                                                                              account_name scope,
                                                                              action_name act_name
                                                                            )const
   {
      try {
         // First look up a specific link for this message act_name
         auto key = boost::make_tuple(authorizer_account, scope, act_name);
         auto link = _db.find<permission_link_object, by_action_name>(key);
         // If no specific link found, check for a contract-wide default
         if (link == nullptr) {
            boost::get<2>(key) = "";
            link = _db.find<permission_link_object, by_action_name>(key);
         }

         // If no specific or default link found, use active permission
         if (link != nullptr) {
            return link->required_permission;
         }
         return optional<permission_name>();

       //  return optional<permission_name>();
      } FC_CAPTURE_AND_RETHROW((authorizer_account)(scope)(act_name))
   }

   optional<permission_name> authorization_manager::lookup_minimum_permission( account_name authorizer_account,
                                                                               account_name scope,
                                                                               action_name act_name
                                                                             )const
   {
      // Special case native actions cannot be linked to a minimum permission, so there is no need to check.
      if( scope == config::system_account_name ) {
          ULTRAIN_ASSERT( act_name != updateauth::get_name() &&
                     act_name != deleteauth::get_name() &&
                     act_name != linkauth::get_name() &&
                     act_name != unlinkauth::get_name() &&
                     act_name != canceldelay::get_name(),
                     unlinkable_min_permission_action,
                     "cannot call lookup_minimum_permission on native actions that are not allowed to be linked to minimum permissions" );
      }

      try {
         optional<permission_name> linked_permission = lookup_linked_permission(authorizer_account, scope, act_name);
         if( !linked_permission )
            return config::active_name;

         if( *linked_permission == config::ultrainio_any_name )
            return optional<permission_name>();

         return linked_permission;
      } FC_CAPTURE_AND_RETHROW((authorizer_account)(scope)(act_name))
   }

   void authorization_manager::check_updateauth_authorization( const updateauth& update,
                                                               const vector<permission_level>& auths
                                                             )const
   {
      ULTRAIN_ASSERT( auths.size() == 1, irrelevant_auth_exception,
                  "updateauth action should only have one declared authorization" );
      const auto& auth = auths[0];
      ULTRAIN_ASSERT( auth.actor == update.account, irrelevant_auth_exception,
                  "the owner of the affected permission needs to be the actor of the declared authorization" );

      const auto* min_permission = find_permission({update.account, update.permission});
      if( !min_permission ) { // creating a new permission
         min_permission = &get_permission({update.account, update.parent});
      }

      ULTRAIN_ASSERT( get_permission(auth).satisfies( *min_permission,
                                                  _db.get_index<permission_index>().indices() ),
                  irrelevant_auth_exception,
                  "updateauth action declares irrelevant authority '${auth}'; minimum authority is ${min}",
                  ("auth", auth)("min", permission_level{update.account, min_permission->name}) );
   }

   void authorization_manager::check_deleteauth_authorization( const deleteauth& del,
                                                               const vector<permission_level>& auths
                                                             )const
   {
      ULTRAIN_ASSERT( auths.size() == 1, irrelevant_auth_exception,
                  "deleteauth action should only have one declared authorization" );
      const auto& auth = auths[0];
      ULTRAIN_ASSERT( auth.actor == del.account, irrelevant_auth_exception,
                  "the owner of the permission to delete needs to be the actor of the declared authorization" );

      const auto& min_permission = get_permission({del.account, del.permission});

      ULTRAIN_ASSERT( get_permission(auth).satisfies( min_permission,
                                                  _db.get_index<permission_index>().indices() ),
                  irrelevant_auth_exception,
                  "updateauth action declares irrelevant authority '${auth}'; minimum authority is ${min}",
                  ("auth", auth)("min", permission_level{min_permission.owner, min_permission.name}) );
   }

   void authorization_manager::check_linkauth_authorization( const linkauth& link,
                                                             const vector<permission_level>& auths
                                                           )const
   {
      ULTRAIN_ASSERT( auths.size() == 1, irrelevant_auth_exception,
                  "link action should only have one declared authorization" );
      const auto& auth = auths[0];
      ULTRAIN_ASSERT( auth.actor == link.account, irrelevant_auth_exception,
                  "the owner of the linked permission needs to be the actor of the declared authorization" );

      ULTRAIN_ASSERT( link.type != updateauth::get_name(),  action_validate_exception,
                  "Cannot link ultrainio::updateauth to a minimum permission" );
      ULTRAIN_ASSERT( link.type != deleteauth::get_name(),  action_validate_exception,
                  "Cannot link ultrainio::deleteauth to a minimum permission" );
      ULTRAIN_ASSERT( link.type != linkauth::get_name(),    action_validate_exception,
                  "Cannot link ultrainio::linkauth to a minimum permission" );
      ULTRAIN_ASSERT( link.type != unlinkauth::get_name(),  action_validate_exception,
                  "Cannot link ultrainio::unlinkauth to a minimum permission" );
      ULTRAIN_ASSERT( link.type != canceldelay::get_name(), action_validate_exception,
                  "Cannot link ultrainio::canceldelay to a minimum permission" );

      const auto linked_permission_name = lookup_minimum_permission(link.account, link.code, link.type);

      if( !linked_permission_name ) // if action is linked to utrio.any permission
         return;

      ULTRAIN_ASSERT( get_permission(auth).satisfies( get_permission({link.account, *linked_permission_name}),
                                                  _db.get_index<permission_index>().indices()              ),
                  irrelevant_auth_exception,
                  "link action declares irrelevant authority '${auth}'; minimum authority is ${min}",
                  ("auth", auth)("min", permission_level{link.account, *linked_permission_name}) );
   }

   void authorization_manager::check_unlinkauth_authorization( const unlinkauth& unlink,
                                                               const vector<permission_level>& auths
                                                             )const
   {
      ULTRAIN_ASSERT( auths.size() == 1, irrelevant_auth_exception,
                  "unlink action should only have one declared authorization" );
      const auto& auth = auths[0];
      ULTRAIN_ASSERT( auth.actor == unlink.account, irrelevant_auth_exception,
                  "the owner of the linked permission needs to be the actor of the declared authorization" );

      const auto unlinked_permission_name = lookup_linked_permission(unlink.account, unlink.code, unlink.type);
      ULTRAIN_ASSERT( unlinked_permission_name.valid(), transaction_exception,
                  "cannot unlink non-existent permission link of account '${account}' for actions matching '${code}::${action}'",
                  ("account", unlink.account)("code", unlink.code)("action", unlink.type) );

      if( *unlinked_permission_name == config::ultrainio_any_name )
         return;

      ULTRAIN_ASSERT( get_permission(auth).satisfies( get_permission({unlink.account, *unlinked_permission_name}),
                                                  _db.get_index<permission_index>().indices()                  ),
                  irrelevant_auth_exception,
                  "unlink action declares irrelevant authority '${auth}'; minimum authority is ${min}",
                  ("auth", auth)("min", permission_level{unlink.account, *unlinked_permission_name}) );
   }

   fc::microseconds authorization_manager::check_canceldelay_authorization( const canceldelay& cancel,
                                                                            const vector<permission_level>& auths
                                                                          )const
   {
      ULTRAIN_ASSERT( auths.size() == 1, irrelevant_auth_exception,
                  "canceldelay action should only have one declared authorization" );
      const auto& auth = auths[0];

      ULTRAIN_ASSERT( get_permission(auth).satisfies( get_permission(cancel.canceling_auth),
                                                  _db.get_index<permission_index>().indices() ),
                  irrelevant_auth_exception,
                  "canceldelay action declares irrelevant authority '${auth}'; specified authority to satisfy is ${min}",
                  ("auth", auth)("min", cancel.canceling_auth) );

      const auto& trx_id = cancel.trx_id;

      const auto& generated_transaction_idx = _control.db().get_index<generated_transaction_multi_index>();
      const auto& generated_index = generated_transaction_idx.indices().get<by_trx_id>();
      const auto& itr = generated_index.lower_bound(trx_id);
      ULTRAIN_ASSERT( itr != generated_index.end() && itr->sender == account_name() && itr->trx_id == trx_id,
                  tx_not_found,
                 "cannot cancel trx_id=${tid}, there is no deferred transaction with that transaction id",
                 ("tid", trx_id) );

      auto trx = fc::raw::unpack<transaction>(itr->packed_trx.data(), itr->packed_trx.size());
      bool found = false;
      for( const auto& act : trx.actions ) {
         for( const auto& auth : act.authorization ) {
            if( auth == cancel.canceling_auth ) {
               found = true;
               break;
            }
         }
         if( found ) break;
      }

      ULTRAIN_ASSERT( found, action_validate_exception,
                  "canceling_auth in canceldelay action was not found as authorization in the original delayed transaction" );

      return (itr->delay_until - itr->published);
   }

   void noop_checktime() {}

   std::function<void()> authorization_manager::_noop_checktime{&noop_checktime};

   void
   authorization_manager::check_authorization( const vector<action>&                actions,
                                               const flat_set<public_key_type>&     provided_keys,
                                               const flat_set<permission_level>&    provided_permissions,
                                               fc::microseconds                     provided_delay,
                                               const std::function<void()>&         _checktime,
                                               bool                                 allow_unused_keys,
                                               const flat_set<permission_level>&    satisfied_authorizations
                                             )const
   {
      const auto& checktime = ( static_cast<bool>(_checktime) ? _checktime : _noop_checktime );

      auto delay_max_limit = fc::seconds( _control.get_global_properties().configuration.max_transaction_delay );

      auto effective_provided_delay =  (provided_delay >= delay_max_limit) ? fc::microseconds::maximum() : provided_delay;

      auto checker = make_auth_checker( [&](const permission_level& p){ return get_permission(p).auth; },
                                        _control.get_global_properties().configuration.max_authority_depth,
                                        provided_keys,
                                        provided_permissions,
                                        effective_provided_delay,
                                        checktime
                                      );

      map<permission_level, fc::microseconds> permissions_to_satisfy;

      for( const auto& act : actions ) {
         bool special_case = false;
         fc::microseconds delay = effective_provided_delay;

         if( act.account == config::system_account_name ) {
            special_case = true;

            if( act.name == updateauth::get_name() ) {
               check_updateauth_authorization( act.data_as<updateauth>(), act.authorization );
            } else if( act.name == deleteauth::get_name() ) {
               check_deleteauth_authorization( act.data_as<deleteauth>(), act.authorization );
            } else if( act.name == linkauth::get_name() ) {
               check_linkauth_authorization( act.data_as<linkauth>(), act.authorization );
            } else if( act.name == unlinkauth::get_name() ) {
               check_unlinkauth_authorization( act.data_as<unlinkauth>(), act.authorization );
            } else if( act.name ==  canceldelay::get_name() ) {
               delay = std::max( delay, check_canceldelay_authorization(act.data_as<canceldelay>(), act.authorization) );
            } else {
               special_case = false;
            }
         }

         for( const auto& declared_auth : act.authorization ) {

            checktime();

            if( !special_case ) {
               auto min_permission_name = lookup_minimum_permission(declared_auth.actor, act.account, act.name);
               if( min_permission_name ) { // since special cases were already handled, it should only be false if the permission is utrio.any
                  const auto& min_permission = get_permission({declared_auth.actor, *min_permission_name});
                  ULTRAIN_ASSERT( get_permission(declared_auth).satisfies( min_permission,
                                                                       _db.get_index<permission_index>().indices() ),
                              irrelevant_auth_exception,
                              "action declares irrelevant authority '${auth}'; minimum authority is ${min}",
                              ("auth", declared_auth)("min", permission_level{min_permission.owner, min_permission.name}) );
               }
            }

            if( satisfied_authorizations.find( declared_auth ) == satisfied_authorizations.end() ) {
               auto res = permissions_to_satisfy.emplace( declared_auth, delay );
               if( !res.second && res.first->second > delay) { // if the declared_auth was already in the map and with a higher delay
                  res.first->second = delay;
               }
            }
         }
      }

      // Now verify that all the declared authorizations are satisfied:

      // Although this can be made parallel (especially for input transactions) with the optimistic assumption that the
      // CPU limit is not reached, because of the CPU limit the protocol must officially specify a sequential algorithm
      // for checking the set of declared authorizations.
      // The permission_levels are traversed in ascending order, which is:
      // ascending order of the actor name with ties broken by ascending order of the permission name.
      for( const auto& p : permissions_to_satisfy ) {
         checktime(); // TODO: this should eventually move into authority_checker instead
         ULTRAIN_ASSERT( checker.satisfied( p.first, p.second ), unsatisfied_authorization,
                     "transaction declares authority '${auth}', "
                     "but does not have signatures for it under a provided delay of ${provided_delay} ms, "
                     "provided permissions ${provided_permissions}, and provided keys ${provided_keys}",
                     ("auth", p.first)
                     ("provided_delay", provided_delay.count()/1000)
                     ("provided_permissions", provided_permissions)
                     ("provided_keys", provided_keys)
                     ("delay_max_limit_ms", delay_max_limit.count()/1000)
                   );

      }

      if( !allow_unused_keys ) {
         ULTRAIN_ASSERT( checker.all_keys_used(), tx_irrelevant_sig,
                     "transaction bears irrelevant signatures from these keys: ${keys}",
                     ("keys", checker.unused_keys()) );
      }
   }

   void
   authorization_manager::check_authorization( account_name                         account,
                                               permission_name                      permission,
                                               const flat_set<public_key_type>&     provided_keys,
                                               const flat_set<permission_level>&    provided_permissions,
                                               fc::microseconds                     provided_delay,
                                               const std::function<void()>&         _checktime,
                                               bool                                 allow_unused_keys
                                             )const
   {
      const auto& checktime = ( static_cast<bool>(_checktime) ? _checktime : _noop_checktime );

      auto delay_max_limit = fc::seconds( _control.get_global_properties().configuration.max_transaction_delay );

      auto checker = make_auth_checker( [&](const permission_level& p){ return get_permission(p).auth; },
                                        _control.get_global_properties().configuration.max_authority_depth,
                                        provided_keys,
                                        provided_permissions,
                                        ( provided_delay >= delay_max_limit ) ? fc::microseconds::maximum() : provided_delay,
                                        checktime
                                      );

      ULTRAIN_ASSERT( checker.satisfied({account, permission}), unsatisfied_authorization,
                  "permission '${auth}' was not satisfied under a provided delay of ${provided_delay} ms, "
                  "provided permissions ${provided_permissions}, and provided keys ${provided_keys}",
                  ("auth", permission_level{account, permission})
                  ("provided_delay", provided_delay.count()/1000)
                  ("provided_permissions", provided_permissions)
                  ("provided_keys", provided_keys)
                  ("delay_max_limit_ms", delay_max_limit.count()/1000)
                );

      if( !allow_unused_keys ) {
         ULTRAIN_ASSERT( checker.all_keys_used(), tx_irrelevant_sig,
                     "irrelevant keys provided: ${keys}",
                     ("keys", checker.unused_keys()) );
      }
   }

   flat_set<public_key_type> authorization_manager::get_required_keys( const transaction& trx,
                                                                       const flat_set<public_key_type>& candidate_keys,
                                                                       fc::microseconds provided_delay
                                                                     )const
   {
      auto checker = make_auth_checker( [&](const permission_level& p){ return get_permission(p).auth; },
                                        _control.get_global_properties().configuration.max_authority_depth,
                                        candidate_keys,
                                        {},
                                        provided_delay,
                                        _noop_checktime
                                      );

      for (const auto& act : trx.actions ) {
         for (const auto& declared_auth : act.authorization) {
            ULTRAIN_ASSERT( checker.satisfied(declared_auth), unsatisfied_authorization,
                        "transaction declares authority '${auth}', but does not have signatures for it.",
                        ("auth", declared_auth) );
         }
      }

      return checker.used_keys();
   }

} } /// namespace ultrainio::chain
