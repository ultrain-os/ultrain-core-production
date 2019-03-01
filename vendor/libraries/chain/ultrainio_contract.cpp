/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/chain/ultrainio_contract.hpp>
#include <ultrainio/chain/contract_table_objects.hpp>

#include <ultrainio/chain/controller.hpp>
#include <ultrainio/chain/transaction_context.hpp>
#include <ultrainio/chain/apply_context.hpp>
#include <ultrainio/chain/transaction.hpp>
#include <ultrainio/chain/exceptions.hpp>

#include <ultrainio/chain/account_object.hpp>
#include <ultrainio/chain/permission_object.hpp>
#include <ultrainio/chain/permission_link_object.hpp>
#include <ultrainio/chain/global_property_object.hpp>
#include <ultrainio/chain/contract_types.hpp>
#include <ultrainio/chain/producer_object.hpp>

#include <ultrainio/chain/wasm_interface.hpp>
#include <ultrainio/chain/abi_serializer.hpp>

#include <ultrainio/chain/authorization_manager.hpp>
#include <ultrainio/chain/resource_limits.hpp>

namespace ultrainio { namespace chain {



uint128_t transaction_id_to_sender_id( const transaction_id_type& tid ) {
   fc::uint128_t _id(tid._hash[3], tid._hash[2]);
   return (unsigned __int128)_id;
}

void validate_authority_precondition( const apply_context& context, const authority& auth ) {
   for(const auto& a : auth.accounts) {
      auto* acct = context.db.find<account_object, by_name>(a.permission.actor);
      ULTRAIN_ASSERT( acct != nullptr, action_validate_exception,
                  "account '${account}' does not exist",
                  ("account", a.permission.actor)
                );

      if( a.permission.permission == config::owner_name || a.permission.permission == config::active_name )
         continue; // account was already checked to exist, so its owner and active permissions should exist

      if( a.permission.permission == config::ultrainio_code_name ) // virtual utrio.code permission does not really exist but is allowed
         continue;

      try {
         context.control.get_authorization_manager().get_permission({a.permission.actor, a.permission.permission});
      } catch( const permission_query_exception& ) {
         ULTRAIN_THROW( action_validate_exception,
                    "permission '${perm}' does not exist",
                    ("perm", a.permission)
                  );
      }
   }

   if( context.control.is_producing_block() ) {
      for( const auto& p : auth.keys ) {
         context.control.check_key_list( p.key );
      }
   }
}

/**
 *  This method is called assuming precondition_system_newaccount succeeds a
 */
void apply_ultrainio_newaccount(apply_context& context) {
   auto create = context.act.data_as<newaccount>();
   try {
   context.require_authorization(create.creator);
//   context.require_write_lock( config::ultrainio_auth_scope );
   auto& authorization = context.control.get_mutable_authorization_manager();

   ULTRAIN_ASSERT( validate(create.owner), action_validate_exception, "Invalid owner authority");
   ULTRAIN_ASSERT( validate(create.active), action_validate_exception, "Invalid active authority");

   auto& db = context.db;

   auto name_str = name(create.name).to_string();

   ULTRAIN_ASSERT( !create.name.empty(), action_validate_exception, "account name cannot be empty" );

   auto p1 = name_str.find('.');
   bool at_begin = (p1 != std::string::npos) && (p1 == 0);
   auto p2 = name_str.rfind('.');
   bool at_end   = (p2 != std::string::npos) && (p2 == name_str.length() - 1);
   ULTRAIN_ASSERT( !(at_begin || at_end), action_validate_exception, "account name can't start/end with ." );

   // Check if the creator is privileged
   const auto &creator = db.get<account_object, by_name>(create.creator);
   if( !creator.privileged ) {
      ULTRAIN_ASSERT( name_str.size() == 12, action_validate_exception, "account names can only be 12 chars long" );
      ULTRAIN_ASSERT( name_str.find( "utrio." ) != 0, action_validate_exception,
                  "only privileged accounts can have names that start with 'utrio.'" );
   } else {
      ULTRAIN_ASSERT( name_str.size() <= 12, action_validate_exception, "account names must be less than only 12 chars long" );
   }

   auto existing_account = db.find<account_object, by_name>(create.name);
   ULTRAIN_ASSERT(existing_account == nullptr, account_name_exists_exception,
              "Cannot create account named ${name}, as that name is already taken",
              ("name", create.name));

   const auto& new_account = db.create<account_object>([&](auto& a) {
      a.name = create.name;
      a.updateable = create.updateable;
      a.creation_date = context.control.pending_block_time();
   });

   db.create<account_sequence_object>([&](auto& a) {
      a.name = create.name;
   });

   for( const auto& auth : { create.owner, create.active } ){
      validate_authority_precondition( context, auth );
   }

   const auto& owner_permission  = authorization.create_permission( create.name, config::owner_name, 0,
                                                                    std::move(create.owner) );
   const auto& active_permission = authorization.create_permission( create.name, config::active_name, owner_permission.id,
                                                                    std::move(create.active) );

   context.control.get_mutable_resource_limits_manager().initialize_account(create.name);

   int64_t ram_delta = config::overhead_per_account_ram_bytes;
   ram_delta += 2*config::billable_size_v<permission_object>;
   ram_delta += owner_permission.auth.get_billable_size();
   ram_delta += active_permission.auth.get_billable_size();

   context.trx_context.add_ram_usage( N(ultrainio), ram_delta ); //create.creator

} FC_CAPTURE_AND_RETHROW( (create) ) }

void apply_ultrainio_setcode(apply_context& context) {
   const auto& cfg = context.control.get_global_properties().configuration;

   auto& db = context.db;
   auto  act = context.act.data_as<setcode>();
   context.require_authorization(act.account);

   ULTRAIN_ASSERT( act.vmtype == 0, invalid_contract_vm_type, "code should be 0" );
   ULTRAIN_ASSERT( act.vmversion == 0, invalid_contract_vm_version, "version should be 0" );

   fc::sha256 code_id; /// default ID == 0

   if( act.code.size() > 0 ) {
     code_id = fc::sha256::hash( act.code.data(), (uint32_t)act.code.size() );
     wasm_interface::validate(context.control, act.code);
   }

    const auto& account = db.get<account_object, by_name>(act.account);
    // if account has been deployed, then check updateable or not.
    if (account.code.size() > 0) {
        // std::cout << "set code account updateable : " << account.updateable << std::endl;
         if(context.has_authorization(N(ultrainio)))
            context.require_authorization(N(ultrainio));
         else
            FC_ASSERT(account.updateable, "contract account has deployed and it is not updateable.");
    }

   int64_t code_size = (int64_t)act.code.size();
   int64_t old_size  = (int64_t)account.code.size() * config::setcode_ram_bytes_multiplier;
   int64_t new_size  = code_size * config::setcode_ram_bytes_multiplier;

   ULTRAIN_ASSERT( account.code_version != code_id, set_exact_code, "contract is already running this version of code" );

   db.modify( account, [&]( auto& a ) {
      /** TODO: consider whether a microsecond level local timestamp is sufficient to detect code version changes*/
      // TODO: update setcode message to include the hash, then validate it in validate
      a.last_code_update = context.control.pending_block_time();
      a.code_version = code_id;
      a.code.resize( code_size );
      if( code_size > 0 )
         memcpy( a.code.data(), act.code.data(), code_size );

   });

   const auto& account_sequence = db.get<account_sequence_object, by_name>(act.account);
   db.modify( account_sequence, [&]( auto& aso ) {
      aso.code_sequence += 1;
   });

   if (new_size != old_size) {
      context.trx_context.add_ram_usage( act.account, new_size - old_size );
   }
}

void apply_ultrainio_setabi(apply_context& context) {
   auto& db  = context.db;
   auto  act = context.act.data_as<setabi>();

   context.require_authorization(act.account);

    const auto& account = db.get<account_object, by_name>(act.account);
    // if account has been deployed, then check updateable or not.
    if (account.abi.size() > 0) {
        // std::cout << "set abi account updateable : " << account.updateable << std::endl;
         if(context.has_authorization(N(ultrainio)))
            context.require_authorization(N(ultrainio));
         else
            FC_ASSERT(account.updateable, "contract account has deployed and it is not updateable.");
    }

   int64_t abi_size = act.abi.size();

   int64_t old_size = (int64_t)account.abi.size();
   int64_t new_size = abi_size;

   db.modify( account, [&]( auto& a ) {
      a.abi.resize( abi_size );
      if( abi_size > 0 )
         memcpy( a.abi.data(), act.abi.data(), abi_size );
   });

   const auto& account_sequence = db.get<account_sequence_object, by_name>(act.account);
   db.modify( account_sequence, [&]( auto& aso ) {
      aso.abi_sequence += 1;
   });

   if (new_size != old_size) {
      context.trx_context.add_ram_usage( act.account, new_size - old_size );
   }
}

void apply_ultrainio_updateauth(apply_context& context) {

   auto update = context.act.data_as<updateauth>();
   context.require_authorization(update.account); // only here to mark the single authority on this action as used

   auto& authorization = context.control.get_mutable_authorization_manager();
   auto& db = context.db;

   ULTRAIN_ASSERT(!update.permission.empty(), action_validate_exception, "Cannot create authority with empty name");
   ULTRAIN_ASSERT( update.permission.to_string().find( "utrio." ) != 0, action_validate_exception,
               "Permission names that start with 'utrio.' are reserved" );
   ULTRAIN_ASSERT(update.permission != update.parent, action_validate_exception, "Cannot set an authority as its own parent");
   db.get<account_object, by_name>(update.account);
   ULTRAIN_ASSERT(validate(update.auth), action_validate_exception,
              "Invalid authority: ${auth}", ("auth", update.auth));
   if( update.permission == config::active_name )
      ULTRAIN_ASSERT(update.parent == config::owner_name, action_validate_exception, "Cannot change active authority's parent from owner", ("update.parent", update.parent) );
   if (update.permission == config::owner_name)
      ULTRAIN_ASSERT(update.parent.empty(), action_validate_exception, "Cannot change owner authority's parent");
   else
      ULTRAIN_ASSERT(!update.parent.empty(), action_validate_exception, "Only owner permission can have empty parent" );

   if( update.auth.waits.size() > 0 ) {
      auto max_delay = context.control.get_global_properties().configuration.max_transaction_delay;
      ULTRAIN_ASSERT( update.auth.waits.back().wait_sec <= max_delay, action_validate_exception,
                  "Cannot set delay longer than max_transacton_delay, which is ${max_delay} seconds",
                  ("max_delay", max_delay) );
   }

   validate_authority_precondition(context, update.auth);



   auto permission = authorization.find_permission({update.account, update.permission});

   // If a parent_id of 0 is going to be used to indicate the absence of a parent, then we need to make sure that the chain
   // initializes permission_index with a dummy object that reserves the id of 0.
   authorization_manager::permission_id_type parent_id = 0;
   if( update.permission != config::owner_name ) {
      auto& parent = authorization.get_permission({update.account, update.parent});
      parent_id = parent.id;
   }

   if( permission ) {
      ULTRAIN_ASSERT(parent_id == permission->parent, action_validate_exception,
                 "Changing parent authority is not currently supported");


      int64_t old_size = (int64_t)(config::billable_size_v<permission_object> + permission->auth.get_billable_size());

      authorization.modify_permission( *permission, update.auth );

      int64_t new_size = (int64_t)(config::billable_size_v<permission_object> + permission->auth.get_billable_size());

      context.trx_context.add_ram_usage( config::system_account_name, new_size - old_size );//permission->owner
   } else {
      const auto& p = authorization.create_permission( update.account, update.permission, parent_id, update.auth );

      int64_t new_size = (int64_t)(config::billable_size_v<permission_object> + p.auth.get_billable_size());

      context.trx_context.add_ram_usage( config::system_account_name, new_size );//system_account_name  update.account
   }
}

void apply_ultrainio_deleteauth(apply_context& context) {
//   context.require_write_lock( config::ultrainio_auth_scope );

   auto remove = context.act.data_as<deleteauth>();
   context.require_authorization(remove.account); // only here to mark the single authority on this action as used

   ULTRAIN_ASSERT(remove.permission != config::active_name, action_validate_exception, "Cannot delete active authority");
   ULTRAIN_ASSERT(remove.permission != config::owner_name, action_validate_exception, "Cannot delete owner authority");

   auto& authorization = context.control.get_mutable_authorization_manager();
   auto& db = context.db;



   { // Check for links to this permission
      const auto& index = db.get_index<permission_link_index, by_permission_name>();
      auto range = index.equal_range(boost::make_tuple(remove.account, remove.permission));
      ULTRAIN_ASSERT(range.first == range.second, action_validate_exception,
                 "Cannot delete a linked authority. Unlink the authority first. This authority is linked to ${code}::${type}.",
                 ("code", string(range.first->code))("type", string(range.first->message_type)));
   }

   const auto& permission = authorization.get_permission({remove.account, remove.permission});
   int64_t old_size = config::billable_size_v<permission_object> + permission.auth.get_billable_size();

   authorization.remove_permission( permission );

   context.trx_context.add_ram_usage( config::system_account_name, -old_size ); //remove.account

}

void apply_ultrainio_linkauth(apply_context& context) {
//   context.require_write_lock( config::ultrainio_auth_scope );

   auto requirement = context.act.data_as<linkauth>();
   try {
      ULTRAIN_ASSERT(!requirement.requirement.empty(), action_validate_exception, "Required permission cannot be empty");

      context.require_authorization(requirement.account); // only here to mark the single authority on this action as used

      auto& db = context.db;
      const auto *account = db.find<account_object, by_name>(requirement.account);
      ULTRAIN_ASSERT(account != nullptr, account_query_exception,
                 "Failed to retrieve account: ${account}", ("account", requirement.account)); // Redundant?
      const auto *code = db.find<account_object, by_name>(requirement.code);
      ULTRAIN_ASSERT(code != nullptr, account_query_exception,
                 "Failed to retrieve code for account: ${account}", ("account", requirement.code));
      if( requirement.requirement != config::ultrainio_any_name ) {
         const auto *permission = db.find<permission_object, by_name>(requirement.requirement);
         ULTRAIN_ASSERT(permission != nullptr, permission_query_exception,
                    "Failed to retrieve permission: ${permission}", ("permission", requirement.requirement));
      }

      auto link_key = boost::make_tuple(requirement.account, requirement.code, requirement.type);
      auto link = db.find<permission_link_object, by_action_name>(link_key);

      if( link ) {
         ULTRAIN_ASSERT(link->required_permission != requirement.requirement, action_validate_exception,
                    "Attempting to update required authority, but new requirement is same as old");
         db.modify(*link, [requirement = requirement.requirement](permission_link_object& link) {
             link.required_permission = requirement;
         });
      } else {
         const auto& l =  db.create<permission_link_object>([&requirement](permission_link_object& link) {
            link.account = requirement.account;
            link.code = requirement.code;
            link.message_type = requirement.type;
            link.required_permission = requirement.requirement;
         });

         context.trx_context.add_ram_usage(
            l.account,
            (int64_t)(config::billable_size_v<permission_link_object>)
         );
      }

  } FC_CAPTURE_AND_RETHROW((requirement))
}

void apply_ultrainio_unlinkauth(apply_context& context) {
//   context.require_write_lock( config::ultrainio_auth_scope );

   auto& db = context.db;
   auto unlink = context.act.data_as<unlinkauth>();

   context.require_authorization(unlink.account); // only here to mark the single authority on this action as used

   auto link_key = boost::make_tuple(unlink.account, unlink.code, unlink.type);
   auto link = db.find<permission_link_object, by_action_name>(link_key);
   ULTRAIN_ASSERT(link != nullptr, action_validate_exception, "Attempting to unlink authority, but no link found");
   context.trx_context.add_ram_usage(
      link->account,
      -(int64_t)(config::billable_size_v<permission_link_object>)
   );

   db.remove(*link);
}

void apply_ultrainio_canceldelay(apply_context& context) {
   auto cancel = context.act.data_as<canceldelay>();
   context.require_authorization(cancel.canceling_auth.actor); // only here to mark the single authority on this action as used

   const auto& trx_id = cancel.trx_id;

   context.cancel_deferred_transaction(transaction_id_to_sender_id(trx_id), account_name());
}

} } // namespace ultrainio::chain
