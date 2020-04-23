/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <ultrainio/chain/account_object.hpp>
#include <ultrainio/chain/controller.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <ultrainio/chain/generated_transaction_object.hpp>
#include <ultrainio/chain/global_property_object.hpp>
#include <ultrainio/chain/permission_link_object.hpp>
#include <ultrainio/chain/permission_object.hpp>
#include <ultrainio/chain/resource_limits.hpp>
#include <ultrainio/chain/resource_limits_private.hpp>
#include <ultrainio/chain/trace.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <ultrainio/state_history_plugin/state_history_plugin.hpp>

#include <type_traits>

template <typename T>
struct history_serial_wrapper {
   const chainbase::database& db;
   const T&                   obj;
};

template <typename T>
history_serial_wrapper<std::decay_t<T>> make_history_serial_wrapper(const chainbase::database& db, const T& obj) {
   return {db, obj};
}

template <typename P, typename T>
struct history_context_wrapper {
   const chainbase::database& db;
   const P&                   context;
   const T&                   obj;
};

template <typename P, typename T>
history_context_wrapper<std::decay_t<P>, std::decay_t<T>>
make_history_context_wrapper(const chainbase::database& db, const P& context, const T& obj) {
   return {db, context, obj};
}

namespace fc {

template <typename T>
const T& as_type(const T& x) {
   return x;
}

template <typename ST, typename T>
datastream<ST>& history_serialize_container(datastream<ST>& ds, const chainbase::database& db, const T& v) {
   fc::raw::pack(ds, unsigned_int(v.size()));
   for (auto& x : v)
      ds << make_history_serial_wrapper(db, x);
   return ds;
}

template <typename ST, typename T>
datastream<ST>& history_serialize_container(datastream<ST>& ds, const chainbase::database& db,
                                            const std::vector<std::shared_ptr<T>>& v) {
   fc::raw::pack(ds, unsigned_int(v.size()));
   for (auto& x : v) {
      ULTRAIN_ASSERT(!!x, ultrainio::chain::plugin_exception, "null inside container");
      ds << make_history_serial_wrapper(db, *x);
   }
   return ds;
}

template <typename ST, typename P, typename T>
datastream<ST>& history_context_serialize_container(datastream<ST>& ds, const chainbase::database& db, const P& context,
                                                    const std::vector<T>& v) {
   fc::raw::pack(ds, unsigned_int(v.size()));
   for (const auto& x : v) {
      ds << make_history_context_wrapper(db, context, x);
   }
   return ds;
}

template <typename ST, typename T>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_big_vector_wrapper<T>& obj) {
   FC_ASSERT(obj.obj.size() <= 1024 * 1024 * 1024);
   fc::raw::pack(ds, unsigned_int((uint32_t)obj.obj.size()));
   for (auto& x : obj.obj)
      fc::raw::pack(ds, x);
   return ds;
}

template <typename ST>
inline void history_pack_varuint64(datastream<ST>& ds, uint64_t val) {
   do {
      uint8_t b = uint8_t(val) & 0x7f;
      val >>= 7;
      b |= ((val > 0) << 7);
      ds.write((char*)&b, 1);
   } while (val);
}

template <typename ST>
void history_pack_big_bytes(datastream<ST>& ds, const ultrainio::chain::bytes& v) {
   history_pack_varuint64(ds, v.size());
   if (v.size())
      ds.write(&v.front(), (uint32_t)v.size());
}

template <typename ST>
void history_pack_big_bytes(datastream<ST>& ds, const fc::optional<ultrainio::chain::bytes>& v) {
   fc::raw::pack(ds, v.valid());
   if (v)
      history_pack_big_bytes(ds, *v);
}

template <typename ST, typename T>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_wrapper<std::vector<T>>& obj) {
   return history_serialize_container(ds, obj.db, obj.obj);
}

template <typename ST, typename P, typename T>
datastream<ST>& operator<<(datastream<ST>& ds, const history_context_wrapper<P, std::vector<T>>& obj) {
   return history_context_serialize_container(ds, obj.db, obj.context, obj.obj);
}

template <typename ST, typename First, typename Second>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_wrapper<std::pair<First, Second>>& obj) {
   fc::raw::pack(ds, obj.obj.first);
   fc::raw::pack(ds, obj.obj.second);
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_wrapper<ultrainio::chain::name>& obj) {
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.value));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_wrapper<ultrainio::chain::account_object>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   //pack order should be  name create_data abi, it keep same pos with query_config.json in fill rep
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.name.value));
   fc::raw::pack(ds, as_type<ultrainio::chain::block_timestamp_type>(obj.obj.creation_date));
   fc::raw::pack(ds, as_type<ultrainio::chain::shared_string>(obj.obj.abi));
   fc::raw::pack(ds, as_type<uint8_t>(obj.obj.vm_type));
   fc::raw::pack(ds, as_type<uint8_t>(obj.obj.vm_version));
   fc::raw::pack(ds, as_type<bool>(obj.obj.privileged));
   fc::raw::pack(ds, as_type<bool>(obj.obj.updateable));
   fc::raw::pack(ds, as_type<fc::time_point>(obj.obj.last_code_update));
   fc::raw::pack(ds, as_type<ultrainio::chain::digest_type>(obj.obj.code_version));
   history_serialize_container(ds, obj.db, as_type<ultrainio::chain::shared_vector<ultrainio::chain::name>>(obj.obj.chain_names));
   fc::raw::pack(ds, as_type<ultrainio::chain::shared_string>(obj.obj.code));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_wrapper<ultrainio::chain::table_id_object>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.code.value));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.scope.value));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.table.value));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.payer.value));
   return ds;
}

template <typename ST>
datastream<ST>&
operator<<(datastream<ST>&                                                                               ds,
           const history_context_wrapper<ultrainio::chain::table_id_object, ultrainio::chain::key_value_object>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<uint64_t>(obj.context.code.value));
   fc::raw::pack(ds, as_type<uint64_t>(obj.context.scope.value));
   fc::raw::pack(ds, as_type<uint64_t>(obj.context.table.value));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.primary_key));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.payer.value));
   fc::raw::pack(ds, as_type<ultrainio::chain::shared_string>(obj.obj.value));
   return ds;
}

template <typename ST, typename T>
void serialize_secondary_index_data(datastream<ST>& ds, const T& obj) {
   fc::raw::pack(ds, obj);
}

template <typename ST>
void serialize_secondary_index_data(datastream<ST>& ds, const float64_t& obj) {
   uint64_t i;
   memcpy(&i, &obj, sizeof(i));
   fc::raw::pack(ds, i);
}

template <typename ST>
void serialize_secondary_index_data(datastream<ST>& ds, const float128_t& obj) {
   __uint128_t i;
   memcpy(&i, &obj, sizeof(i));
   fc::raw::pack(ds, i);
}

template <typename ST>
void serialize_secondary_index_data(datastream<ST>& ds, const ultrainio::chain::key256_t& obj) {
   auto rev = [&](__uint128_t x) {
      char* ch = reinterpret_cast<char*>(&x);
      std::reverse(ch, ch + sizeof(x));
      return x;
   };
   fc::raw::pack(ds, rev(obj[0]));
   fc::raw::pack(ds, rev(obj[1]));
}

template <typename ST, typename T>
datastream<ST>& serialize_secondary_index(datastream<ST>& ds, const ultrainio::chain::table_id_object& context,
                                          const T& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<uint64_t>(context.code.value));
   fc::raw::pack(ds, as_type<uint64_t>(context.scope.value));
   fc::raw::pack(ds, as_type<uint64_t>(context.table.value));
   fc::raw::pack(ds, as_type<uint64_t>(obj.primary_key));
   fc::raw::pack(ds, as_type<uint64_t>(obj.payer.value));
   serialize_secondary_index_data(ds, obj.secondary_key);
   return ds;
}

template <typename ST>
datastream<ST>&
operator<<(datastream<ST>&                                                                             ds,
           const history_context_wrapper<ultrainio::chain::table_id_object, ultrainio::chain::index64_object>& obj) {
   return serialize_secondary_index(ds, obj.context, obj.obj);
}

template <typename ST>
datastream<ST>&
operator<<(datastream<ST>&                                                                              ds,
           const history_context_wrapper<ultrainio::chain::table_id_object, ultrainio::chain::index128_object>& obj) {
   return serialize_secondary_index(ds, obj.context, obj.obj);
}

template <typename ST>
datastream<ST>&
operator<<(datastream<ST>&                                                                              ds,
           const history_context_wrapper<ultrainio::chain::table_id_object, ultrainio::chain::index256_object>& obj) {
   return serialize_secondary_index(ds, obj.context, obj.obj);
}

template <typename ST>
datastream<ST>&
operator<<(datastream<ST>&                                                                                  ds,
           const history_context_wrapper<ultrainio::chain::table_id_object, ultrainio::chain::index_double_object>& obj) {
   return serialize_secondary_index(ds, obj.context, obj.obj);
}

template <typename ST>
datastream<ST>& operator<<(
    datastream<ST>&                                                                                       ds,
    const history_context_wrapper<ultrainio::chain::table_id_object, ultrainio::chain::index_long_double_object>& obj) {
   return serialize_secondary_index(ds, obj.context, obj.obj);
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_wrapper<ultrainio::chain::chain_config>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.max_block_net_usage));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.target_block_net_usage_pct));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.max_transaction_net_usage));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.base_per_transaction_net_usage));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.net_usage_leeway));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.context_free_discount_net_usage_num));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.context_free_discount_net_usage_den));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.max_block_cpu_usage));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.target_block_cpu_usage_pct));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.max_transaction_cpu_usage));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.min_transaction_cpu_usage));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.max_transaction_lifetime));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.deferred_trx_expiration_window));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.max_transaction_delay));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.max_inline_action_size));
   fc::raw::pack(ds, as_type<uint16_t>(obj.obj.max_inline_action_depth));
   fc::raw::pack(ds, as_type<uint16_t>(obj.obj.max_authority_depth));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>&                                                     ds,
                           const history_serial_wrapper<ultrainio::chain::global_property_object>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   // fc::raw::pack(ds, as_type<optional<ultrainio::chain::block_num_type>>(obj.obj.proposed_schedule_block_num));
   // fc::raw::pack(ds, make_history_serial_wrapper(
   //                       obj.db, as_type<ultrainio::chain::shared_producer_schedule_type>(obj.obj.proposed_schedule)));
   fc::raw::pack(ds, make_history_serial_wrapper(obj.db, as_type<ultrainio::chain::chain_config>(obj.obj.configuration)));

   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>&                                                           ds,
                           const history_serial_wrapper<ultrainio::chain::generated_transaction_object>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.sender.value));
   fc::raw::pack(ds, as_type<__uint128_t>(obj.obj.sender_id));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.payer.value));
   fc::raw::pack(ds, as_type<ultrainio::chain::transaction_id_type>(obj.obj.trx_id));
   fc::raw::pack(ds, as_type<ultrainio::chain::shared_string>(obj.obj.packed_trx));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_wrapper<ultrainio::chain::key_weight>& obj) {
   fc::raw::pack(ds, as_type<ultrainio::chain::public_key_type>(obj.obj.key));
   fc::raw::pack(ds, as_type<uint16_t>(obj.obj.weight));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_wrapper<ultrainio::chain::permission_level>& obj) {
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.actor.value));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.permission.value));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>&                                                      ds,
                           const history_serial_wrapper<ultrainio::chain::permission_level_weight>& obj) {
   fc::raw::pack(ds, make_history_serial_wrapper(obj.db, as_type<ultrainio::chain::permission_level>(obj.obj.permission)));
   fc::raw::pack(ds, as_type<uint16_t>(obj.obj.weight));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_wrapper<ultrainio::chain::wait_weight>& obj) {
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.wait_sec));
   fc::raw::pack(ds, as_type<uint16_t>(obj.obj.weight));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_wrapper<ultrainio::chain::shared_authority>& obj) {
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.threshold));
   history_serialize_container(ds, obj.db, obj.obj.keys);
   history_serialize_container(ds, obj.db, obj.obj.accounts);
   history_serialize_container(ds, obj.db, obj.obj.waits);
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_wrapper<ultrainio::chain::permission_object>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.owner.value));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.name.value));
   if (obj.obj.parent._id) {
      auto&       index  = obj.db.get_index<ultrainio::chain::permission_index>();
      const auto* parent = index.find(obj.obj.parent);
      if (!parent) {
         auto& undo = index.stack().back();
         auto  it   = undo.removed_values.find(obj.obj.parent);
         ULTRAIN_ASSERT(it != undo.removed_values.end(), ultrainio::chain::plugin_exception,
                    "can not find parent of permission_object");
         parent = &it->second;
      }
      fc::raw::pack(ds, as_type<uint64_t>(parent->name.value));
   } else {
      fc::raw::pack(ds, as_type<uint64_t>(0));
   }
   fc::raw::pack(ds, as_type<fc::time_point>(obj.obj.last_updated));
   fc::raw::pack(ds, make_history_serial_wrapper(obj.db, as_type<ultrainio::chain::shared_authority>(obj.obj.auth)));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>&                                                     ds,
                           const history_serial_wrapper<ultrainio::chain::permission_link_object>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.account.value));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.code.value));
   fc::raw::pack(ds, as_type<ultrainio::chain::name_ex>(obj.obj.message_type)); //message_type is name_ex, have 2 uint64
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.required_permission.value));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>&                                                                      ds,
                           const history_serial_wrapper<ultrainio::chain::resource_limits::resource_limits_object>& obj) {
   ULTRAIN_ASSERT(!obj.obj.pending, ultrainio::chain::plugin_exception,
              "accepted_block sent while resource_limits_object in pending state");
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.owner.value));
   fc::raw::pack(ds, as_type<int64_t>(obj.obj.net_weight));
   fc::raw::pack(ds, as_type<int64_t>(obj.obj.cpu_weight));
   fc::raw::pack(ds, as_type<int64_t>(obj.obj.ram_bytes));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>&                                                                 ds,
                           const history_serial_wrapper<ultrainio::chain::resource_limits::usage_accumulator>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.last_ordinal));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.value_ex));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.consumed));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>&                                                                     ds,
                           const history_serial_wrapper<ultrainio::chain::resource_limits::resource_usage_object>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.owner.value));
   fc::raw::pack(ds, make_history_serial_wrapper(
                         obj.db, as_type<ultrainio::chain::resource_limits::usage_accumulator>(obj.obj.net_usage)));
   fc::raw::pack(ds, make_history_serial_wrapper(
                         obj.db, as_type<ultrainio::chain::resource_limits::usage_accumulator>(obj.obj.cpu_usage)));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.ram_usage));
   return ds;
}

template <typename ST>
datastream<ST>&
operator<<(datastream<ST>&                                                                            ds,
           const history_serial_wrapper<ultrainio::chain::resource_limits::resource_limits_state_object>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, make_history_serial_wrapper(obj.db, as_type<ultrainio::chain::resource_limits::usage_accumulator>(
                                                             obj.obj.average_block_net_usage)));
   fc::raw::pack(ds, make_history_serial_wrapper(obj.db, as_type<ultrainio::chain::resource_limits::usage_accumulator>(
                                                             obj.obj.average_block_cpu_usage)));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.total_net_weight));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.total_cpu_weight));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.total_ram_bytes));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.virtual_net_limit));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.virtual_cpu_limit));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>&                                                     ds,
                           const history_serial_wrapper<ultrainio::chain::resource_limits::ratio>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.numerator));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.denominator));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>&                                                                        ds,
                           const history_serial_wrapper<ultrainio::chain::resource_limits::elastic_limit_parameters>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.target));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.max));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.periods));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.max_multiplier));
   fc::raw::pack(
       ds, make_history_serial_wrapper(obj.db, as_type<ultrainio::chain::resource_limits::ratio>(obj.obj.contract_rate)));
   fc::raw::pack(
       ds, make_history_serial_wrapper(obj.db, as_type<ultrainio::chain::resource_limits::ratio>(obj.obj.expand_rate)));
   return ds;
}

template <typename ST>
datastream<ST>&
operator<<(datastream<ST>&                                                                             ds,
           const history_serial_wrapper<ultrainio::chain::resource_limits::resource_limits_config_object>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(
       ds, make_history_serial_wrapper(
               obj.db, as_type<ultrainio::chain::resource_limits::elastic_limit_parameters>(obj.obj.cpu_limit_parameters)));
   fc::raw::pack(
       ds, make_history_serial_wrapper(
               obj.db, as_type<ultrainio::chain::resource_limits::elastic_limit_parameters>(obj.obj.net_limit_parameters)));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.account_cpu_usage_average_window));
   fc::raw::pack(ds, as_type<uint32_t>(obj.obj.account_net_usage_average_window));
   return ds;
};

template <typename ST>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_wrapper<ultrainio::chain::action>& obj) {
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.account.value));
   fc::raw::pack(ds, as_type<ultrainio::chain::name_ex>(obj.obj.name));////name is name_ex, have 2 uint64
   history_serialize_container(ds, obj.db, as_type<std::vector<ultrainio::chain::permission_level>>(obj.obj.authorization));
   fc::raw::pack(ds, as_type<ultrainio::bytes>(obj.obj.data));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>& ds, const history_serial_wrapper<ultrainio::chain::action_receipt>& obj) {
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.receiver.value));
   fc::raw::pack(ds, as_type<ultrainio::chain::digest_type>(obj.obj.act_digest));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.global_sequence));
   fc::raw::pack(ds, as_type<uint64_t>(obj.obj.recv_sequence));
   history_serialize_container(ds, obj.db, as_type<flat_map<ultrainio::name, uint64_t>>(obj.obj.auth_sequence));
   fc::raw::pack(ds, as_type<fc::unsigned_int>(obj.obj.code_sequence));
   fc::raw::pack(ds, as_type<fc::unsigned_int>(obj.obj.abi_sequence));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>& ds, const history_context_wrapper<bool, ultrainio::chain::action_trace>& obj) {
   bool  debug_mode = obj.context;
   fc::raw::pack(ds, fc::unsigned_int(0));
   // if (obj.obj.receipt) {
      fc::raw::pack(ds, make_history_serial_wrapper(obj.db, as_type<ultrainio::chain::action_receipt>(obj.obj.receipt)));
   // }
   fc::raw::pack(ds, make_history_serial_wrapper(obj.db, as_type<ultrainio::chain::action>(obj.obj.act)));
   fc::raw::pack(ds, as_type<int64_t>(debug_mode ? obj.obj.elapsed.count() : 0));
   if (debug_mode)
      fc::raw::pack(ds, as_type<std::string>(obj.obj.console));
   else
      fc::raw::pack(ds, std::string{});

   history_context_serialize_container(ds, obj.db, debug_mode,
                                       as_type<std::vector<ultrainio::chain::action_trace>>(obj.obj.inline_traces));
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>&                                                    ds,
                           const history_context_wrapper<std::pair<uint8_t, bool>,
                                                         ultrainio::augmented_transaction_trace>& obj) {
   auto& trace = *obj.obj.trace;
   bool  debug_mode = obj.context.second;
   fc::raw::pack(ds, fc::unsigned_int(0));
   fc::raw::pack(ds, as_type<ultrainio::chain::transaction_id_type>(trace.id));
   if (trace.receipt) {
      if (trace.failed_dtrx_trace && trace.receipt->status.value == ultrainio::chain::transaction_receipt_header::soft_fail)
         fc::raw::pack(ds, uint8_t(ultrainio::chain::transaction_receipt_header::executed));
      else
         fc::raw::pack(ds, as_type<uint8_t>(trace.receipt->status.value));
      fc::raw::pack(ds, as_type<uint32_t>(trace.receipt->cpu_usage_us));
      fc::raw::pack(ds, as_type<fc::unsigned_int>(trace.receipt->net_usage_words));
   } else {
      fc::raw::pack(ds, uint8_t(obj.context.first));
      fc::raw::pack(ds, uint32_t(0));
      fc::raw::pack(ds, fc::unsigned_int(0));
   }
   fc::raw::pack(ds, as_type<int64_t>(debug_mode ? trace.elapsed.count() : 0));
   fc::raw::pack(ds, as_type<uint64_t>(trace.net_usage));
   fc::raw::pack(ds, as_type<bool>(trace.scheduled));
   history_context_serialize_container(ds, obj.db, debug_mode,
                                       as_type<std::vector<ultrainio::chain::action_trace>>(trace.action_traces));

   fc::optional<std::string> e;
   if (trace.except) {
      if (debug_mode)
         e = trace.except->to_string();
      else
         e = "Y";
   }
   fc::raw::pack(ds, as_type<fc::optional<std::string>>(e));

   fc::raw::pack(ds, bool(trace.failed_dtrx_trace));
   if (trace.failed_dtrx_trace) {
      uint8_t stat = ultrainio::chain::transaction_receipt_header::hard_fail;
      if (trace.receipt && trace.receipt->status.value == ultrainio::chain::transaction_receipt_header::soft_fail)
         stat = ultrainio::chain::transaction_receipt_header::soft_fail;
      std::pair<uint8_t, bool> context = std::make_pair(stat, debug_mode);
      fc::raw::pack(
          ds, make_history_context_wrapper(
                  obj.db, context, ultrainio::augmented_transaction_trace{trace.failed_dtrx_trace, obj.obj.partial}));
   }

   bool include_partial = obj.obj.partial && !trace.failed_dtrx_trace;
   fc::raw::pack(ds, include_partial);
   if (include_partial) {
      auto& partial = *obj.obj.partial;
      fc::raw::pack(ds, fc::unsigned_int(0));
      fc::raw::pack(ds, as_type<ultrainio::chain::time_point_sec>(partial.expiration));
      fc::raw::pack(ds, as_type<uint16_t>(partial.ref_block_num));
      fc::raw::pack(ds, as_type<uint32_t>(partial.ref_block_prefix));
      fc::raw::pack(ds, as_type<fc::unsigned_int>(partial.max_net_usage_words));
      fc::raw::pack(ds, as_type<uint8_t>(partial.max_cpu_usage_ms));
      fc::raw::pack(ds, as_type<fc::unsigned_int>(partial.delay_sec));
      fc::raw::pack(ds, as_type<ultrainio::chain::extensions_type>(partial.transaction_extensions));
      fc::raw::pack(ds, as_type<std::vector<ultrainio::chain::signature_type>>(partial.signatures));
      fc::raw::pack(ds, as_type<std::vector<ultrainio::bytes>>(partial.context_free_data));
   }

   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>&                                                           ds,
                           const history_context_wrapper<bool, ultrainio::augmented_transaction_trace>&  obj) {
   std::pair<uint8_t, bool> context = std::make_pair(ultrainio::chain::transaction_receipt_header::hard_fail, obj.context);
   ds << make_history_context_wrapper(obj.db, context, obj.obj);
   return ds;
}

template <typename ST>
datastream<ST>& operator<<(datastream<ST>& ds, const ultrainio::get_blocks_result_v0& obj) {
   fc::raw::pack(ds, obj.head);
   fc::raw::pack(ds, obj.last_irreversible);
   fc::raw::pack(ds, obj.this_block);
   fc::raw::pack(ds, obj.prev_block);
   history_pack_big_bytes(ds, obj.block);
   history_pack_big_bytes(ds, obj.traces);
   history_pack_big_bytes(ds, obj.deltas);
   return ds;
}

} // namespace fc
