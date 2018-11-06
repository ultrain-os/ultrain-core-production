#include <ultrainio/chain/abi_def.hpp>
#include <fc/utility.hpp>

namespace ultrainio { namespace chain {

vector<type_def> common_type_defs() {
   vector<type_def> types;

   types.push_back( type_def{"account_name", "name"} );
   types.push_back( type_def{"permission_name", "name"} );
   types.push_back( type_def{"action_name", "name_ex"} );
   types.push_back( type_def{"table_name", "name"} );
   types.push_back( type_def{"transaction_id_type", "checksum256"} );
   types.push_back( type_def{"block_id_type", "checksum256"} );
   types.push_back( type_def{"weight_type", "uint16"} );

   return types;
}

abi_def ultrainio_contract_abi(const abi_def& ultrainio_system_abi)
{
   abi_def ultrain_abi(ultrainio_system_abi);

   if( ultrain_abi.version.size() == 0 ) {
      ultrain_abi.version = "ultrainio::abi/1.0";
   }

   fc::move_append(ultrain_abi.types, common_type_defs());

   // transaction
   ultrain_abi.structs.emplace_back( struct_def {
      "permission_level", "", {
         {"actor", "account_name"},
         {"permission", "permission_name"}
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "action", "", {
         {"account", "account_name"},
         {"name", "action_name"},
         {"authorization", "permission_level[]"},
         {"data", "bytes"}
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "extension", "", {
         {"type", "uint16"},
         {"data", "bytes"}
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "transaction_header", "", {
         {"expiration", "time_point_sec"},
         {"ref_block_num", "uint16"},
         {"ref_block_prefix", "uint32"},
         {"max_net_usage_words", "varuint32"},
         {"max_cpu_usage_ms", "uint8"},
         {"delay_sec", "varuint32"}
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "transaction", "transaction_header", {
         {"context_free_actions", "action[]"},
         {"actions", "action[]"},
         {"transaction_extensions", "extension[]"}
      }
   });

   // block_header

   ultrain_abi.structs.emplace_back( struct_def {
      "producer_key", "", {
         {"producer_name", "account_name"},
         {"block_signing_key", "public_key"}
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "producer_schedule", "", {
         {"version", "uint32"},
         {"producers", "producer_key[]"}
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "block_header", "", {
         {"timestamp", "uint32"},
         {"proposer", "account_name"},
         {"confirmed", "uint16"},
         {"previous", "block_id_type"},
         {"transaction_mroot", "checksum256"},
         {"action_mroot", "checksum256"},
         {"header_extensions", "extension[]"}
      }
   });

   // authority
   ultrain_abi.structs.emplace_back( struct_def {
      "key_weight", "", {
         {"key", "public_key"},
         {"weight", "weight_type"}
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "permission_level_weight", "", {
         {"permission", "permission_level"},
         {"weight", "weight_type"}
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "wait_weight", "", {
         {"wait_sec", "uint32"},
         {"weight", "weight_type"}
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "authority", "", {
         {"threshold", "uint32"},
         {"keys", "key_weight[]"},
         {"accounts", "permission_level_weight[]"},
         {"waits", "wait_weight[]"}
      }
   });

   // TODO add any ricardian_clauses
   //
   // ACTION PAYLOADS

   ultrain_abi.structs.emplace_back( struct_def {
      "newaccount", "", {
         {"creator", "account_name"},
         {"name", "account_name"},
         {"owner", "authority"},
         {"active", "authority"},
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "setcode", "", {
         {"account", "account_name"},
         {"vmtype", "uint8"},
         {"vmversion", "uint8"},
         {"code", "bytes"}
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "setabi", "", {
         {"account", "account_name"},
         {"abi",     "bytes"}
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "updateauth", "", {
         {"account", "account_name"},
         {"permission", "permission_name"},
         {"parent", "permission_name"},
         {"auth", "authority"}
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "deleteauth", "", {
         {"account", "account_name"},
         {"permission", "permission_name"},
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "linkauth", "", {
         {"account", "account_name"},
         {"code", "account_name"},
         {"type", "action_name"},
         {"requirement", "permission_name"},
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "unlinkauth", "", {
         {"account", "account_name"},
         {"code", "account_name"},
         {"type", "action_name"},
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
      "canceldelay", "", {
         {"canceling_auth", "permission_level"},
         {"trx_id", "transaction_id_type"},
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
         "onerror", "", {
            {"sender_id", "uint128"},
            {"sent_trx",  "bytes"}
      }
   });

   ultrain_abi.structs.emplace_back( struct_def {
         "onblock", "", {
            {"header", "block_header"}
      }
   });

   // TODO add ricardian contracts
   ultrain_abi.actions.push_back( action_def{action_name("newaccount"), "newaccount",""} );
   ultrain_abi.actions.push_back( action_def{action_name("setcode"), "setcode",""} );
   ultrain_abi.actions.push_back( action_def{action_name("setabi"), "setabi",""} );
   ultrain_abi.actions.push_back( action_def{action_name("updateauth"), "updateauth",""} );
   ultrain_abi.actions.push_back( action_def{action_name("deleteauth"), "deleteauth",""} );
   ultrain_abi.actions.push_back( action_def{action_name("linkauth"), "linkauth",""} );
   ultrain_abi.actions.push_back( action_def{action_name("unlinkauth"), "unlinkauth",""} );
   ultrain_abi.actions.push_back( action_def{action_name("canceldelay"), "canceldelay",""} );
   ultrain_abi.actions.push_back( action_def{action_name("onerror"), "onerror",""} );
   ultrain_abi.actions.push_back( action_def{action_name("onblock"), "onblock",""} );

   return ultrain_abi;
}

} } /// ultrainio::chain
