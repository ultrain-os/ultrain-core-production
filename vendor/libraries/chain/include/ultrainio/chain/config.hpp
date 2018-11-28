/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <ultrainio/chain/wasm_interface.hpp>
#include <fc/time.hpp>

#pragma GCC diagnostic ignored "-Wunused-variable"

namespace ultrainio { namespace chain { namespace config {

typedef __uint128_t uint128_t;

const static auto default_blocks_dir_name    = "blocks";

const static auto default_state_dir_name     = "state";
const static auto forkdb_filename            = "forkdb.dat";
const static auto default_state_size            = 1*1024*1024*1024ll;
const static auto default_state_guard_size      = 128*1024*1024ll;

#ifdef ULTRAIN_CONFIG_CONTRACT_PARAMS
    const static uint64_t default_contract_return_length = 128;
    const static uint64_t default_contract_emit_length = 128;
#endif

const static uint64_t system_account_name    = N(ultrainio);
const static uint64_t null_account_name      = N(utrio.null);

const static uint64_t ultrainio_auth_scope       = N(utrio.auth);
const static uint64_t ultrainio_all_scope        = N(utrio.all);

const static uint64_t active_name = N(active);
const static uint64_t owner_name  = N(owner);
const static uint64_t ultrainio_any_name = N(utrio.any);
const static uint64_t ultrainio_code_name = N(utrio.code);

extern int      block_interval_ms;
extern int      block_interval_us;

const static uint64_t block_timestamp_epoch = 1514764800000ll; // epoch is year 2000.

/** Percentages are fixed point with a denominator of 10,000 */
const static int percent_100 = 10000;
const static int percent_1   = 100;

const static uint32_t  required_producer_participation = 33 * config::percent_1;

static const uint32_t account_cpu_usage_average_window_ms  = 24*60*60*1000l;
static const uint32_t account_net_usage_average_window_ms  = 24*60*60*1000l;
static const uint32_t block_cpu_usage_average_window_ms    = 60*1000l;
static const uint32_t block_size_average_window_ms         = 60*1000l;

//const static uint64_t   default_max_storage_size       = 10 * 1024;
//const static uint32_t   default_max_trx_runtime        = 10*1000;
//const static uint32_t   default_max_gen_trx_size       = 64 * 1024;

const static uint32_t   rate_limiting_precision        = 1000*1000;

const static uint32_t   default_max_propose_trx_count                 = 12000;
const static uint32_t   default_max_pending_trx_count                 = 50000;
const static uint32_t   default_max_unapplied_trx_count               = 50000;

const static uint32_t   default_max_block_net_usage                 = 1024 * 1024 * 2;
const static uint32_t   default_target_block_net_usage_pct           = 10 * percent_1; /// we target 1000 TPS
const static uint32_t   default_max_transaction_net_usage            = default_max_block_net_usage / 2;
const static uint32_t   default_base_per_transaction_net_usage       = 12;  // 12 bytes (11 bytes for worst case of transaction_receipt_header + 1 byte for static_variant tag)
const static uint32_t   default_net_usage_leeway                     = 500; // TODO: is this reasonable?
const static uint32_t   default_context_free_discount_net_usage_num  = 20; // TODO: is this reasonable?
const static uint32_t   default_context_free_discount_net_usage_den  = 100;
const static uint32_t   transaction_id_net_usage                     = 32; // 32 bytes for the size of a transaction id

// TODO: this should be derived from consensus period.
extern uint32_t default_max_block_cpu_usage;
extern uint32_t default_max_transaction_cpu_usage;
//const static uint32_t   default_max_block_cpu_usage                 = 3'000'000; /// max block cpu usage in microseconds
const static uint32_t   default_target_block_cpu_usage_pct          = 10 * percent_1;
// max single trx cpu is about 500ms ?
//const static uint32_t   default_max_transaction_cpu_usage           = default_max_block_cpu_usage / 6;
const static uint32_t   default_min_transaction_cpu_usage           = 100; /// min trx cpu usage in microseconds (10000 TPS equiv)

const static uint32_t   default_max_trx_lifetime               = 60*60; // 1 hour
const static uint32_t   default_deferred_trx_expiration_window = 10*60; // 10 minutes
const static uint32_t   default_max_trx_delay                  = 45*24*3600; // 45 days
const static uint32_t   default_max_inline_action_size         = 4 * 1024;   // 4 KB
const static uint16_t   default_max_inline_action_depth        = 4;
const static uint16_t   default_max_auth_depth                 = 6;

const static uint32_t   min_net_usage_delta_between_base_and_max_for_trx  = 10*1024;
// Should be large enough to allow recovery from badly set blockchain parameters without a hard fork
// (unless net_usage_leeway is set to 0 and so are the net limits of all accounts that can help with resetting blockchain parameters).

const static uint32_t   fixed_net_overhead_of_packed_trx = 16; // TODO: is this reasonable?

const static uint32_t   fixed_overhead_shared_vector_ram_bytes = 16; ///< overhead accounts for fixed portion of size of shared_vector field
const static uint32_t   overhead_per_row_per_index_ram_bytes = 32;    ///< overhead accounts for basic tracking structures in a row per index
const static uint32_t   overhead_per_account_ram_bytes     = 0; ///< overhead accounts for basic account storage and pre-pays features like account recovery
const static uint32_t   setcode_ram_bytes_multiplier       = 10;     ///< multiplier on contract size to account for multiple copies and cached compilation

const static uint32_t   hashing_checktime_block_size       = 10*1024;  /// call checktime from hashing intrinsic once per this number of bytes

const static ultrainio::chain::wasm_interface::vm_type default_wasm_runtime = ultrainio::chain::wasm_interface::vm_type::wabt;
const static uint32_t   default_abi_serializer_max_time_ms = 15*1000; ///< default deadline for abi serialization methods

const static uint64_t billable_alignment = 16;

template<typename T>
struct billable_size;

template<typename T>
constexpr uint64_t billable_size_v = ((billable_size<T>::value + billable_alignment - 1) / billable_alignment) * billable_alignment;


} } } // namespace ultrainio::chain::config

constexpr uint64_t ULTRAIN_PERCENT(uint64_t value, uint32_t percentage) {
   return (value * percentage) / ultrainio::chain::config::percent_100;
}

template<typename Number>
Number ULTRAIN_PERCENT_CEIL(Number value, uint32_t percentage) {
   return ((value * percentage) + ultrainio::chain::config::percent_100 - ultrainio::chain::config::percent_1)  / ultrainio::chain::config::percent_100;
}
