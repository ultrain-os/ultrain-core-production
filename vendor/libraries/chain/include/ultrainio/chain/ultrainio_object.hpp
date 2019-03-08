#include <ultrainio/chain/types.hpp>

namespace ultrainio { namespace chain {

    const uint64_t master_chain_name = 0;

    // keep this the same as defined in ultrainio.system.hpp;
    struct producer_brief {
      account_name          owner;
      uint64_t              location;
      bool                  in_disable = true;
    };
    struct role_base {
      account_name          owner;
      std::string           producer_key; /// a packed public key objec
      std::string           bls_key;
    };

    struct disabled_producer : public role_base {
      int64_t               total_cons_staked = 0;
      std::string           url;
      uint64_t              total_produce_block = 0;
      uint64_t              last_operate_blocknum = 0;
      uint64_t              delegated_cons_blocknum = 0;
      account_name          claim_rewards_account;
    };

    // keep this the same as defined in ultrainio.system.hpp;
    struct producer_info : public disabled_producer {
      uint64_t              unpaid_balance = 0;
      uint64_t              vote_number = 0;
      uint64_t              last_vote_blocknum = 0;
      extensions_type           table_extension;
   };

   struct chain_resource {
       uint16_t             max_resources_number = 10000;
       uint16_t             total_resources_used_number = 0;
       uint64_t             max_ram_size = 12ll*1024 * 1024 * 1024;
       uint64_t             total_ram_bytes_used = 0;
   };

   struct user_info {
      account_name      user_name;
      std::string       owner_key;
      std::string       active_key;
      time_point_sec    emp_time;
      bool              is_producer;
   };

   struct changing_committee {
       std::vector<role_base> removed_members;
       std::vector<role_base> new_added_members;
   };

   struct block_header_digest {
       account_name              proposer;
       block_id_type             block_id;
       uint32_t                  block_number;
       checksum256_type          transaction_mroot;
       std::vector<checksum256_type>  trx_hashs;
       extensions_type           table_extension;
   };

   struct unconfirmed_block_header : public block_header_digest {
       uint16_t                  fork_id;
       bool                      to_be_paid;    //should block proposer be paid when this block was confirmed
       bool                      is_leaf;
       bool                      is_synced;
       checksum256_type          committee_mroot;
       std::string               committee_info;
   };

    struct subchain {
       uint64_t                  chain_name;
       uint64_t                  chain_type;
       chain::block_timestamp    genesis_time;
       chain_resource            global_resource;
       bool                      is_synced;
       bool                      is_schedulable;
       uint16_t                  committee_num;
       std::vector<role_base>    deprecated_committee;//keep history producers for un-synced chain, clear it once synced
       changing_committee        changing_info;
       uint32_t                  changing_block_num; //last block number in master when changing its committee
       std::vector<user_info>    recent_users;
       uint32_t                  total_user_num;
       checksum256_type          chain_id;
       checksum256_type          committee_mroot;
       uint32_t                  confirmed_block_number;
       uint32_t                  highest_block_number;
       std::vector<unconfirmed_block_header>  unconfirmed_blocks;
       extensions_type           table_extension;
    };

    struct resources_lease {
      account_name             owner;
      uint64_t                 lease_num;
      uint32_t                 start_block_height;
      uint32_t                 end_block_height;
      uint32_t                 modify_block_height;
    };

}} // namespace ultrainio::chain

FC_REFLECT(ultrainio::chain::producer_brief, (owner)(location)(in_disable) )
FC_REFLECT(ultrainio::chain::role_base, (owner)(producer_key)(bls_key) )
FC_REFLECT_DERIVED(ultrainio::chain::disabled_producer, (ultrainio::chain::role_base), (total_cons_staked)(url)(total_produce_block)
                    (last_operate_blocknum)(delegated_cons_blocknum)(claim_rewards_account) )
FC_REFLECT_DERIVED(ultrainio::chain::producer_info, (ultrainio::chain::disabled_producer),
                    (unpaid_balance)(vote_number)(last_vote_blocknum)(table_extension))
FC_REFLECT(ultrainio::chain::chain_resource, (max_resources_number)(total_resources_used_number)(max_ram_size)(total_ram_bytes_used) )
FC_REFLECT(ultrainio::chain::user_info, (user_name)(owner_key)(active_key)(emp_time)(is_producer) )
FC_REFLECT(ultrainio::chain::changing_committee, (removed_members)(new_added_members) )
FC_REFLECT(ultrainio::chain::block_header_digest, (proposer)(block_id)(block_number)(transaction_mroot)(trx_hashs)(table_extension) )
FC_REFLECT_DERIVED(ultrainio::chain::unconfirmed_block_header, (ultrainio::chain::block_header_digest), (fork_id)(to_be_paid)
                    (is_leaf)(is_synced)(committee_mroot)(committee_info) )
FC_REFLECT(ultrainio::chain::subchain, (chain_name)(chain_type)(genesis_time)(global_resource)(is_synced)(is_schedulable)
                    (committee_num)(deprecated_committee)(changing_info)(changing_block_num)(recent_users)(total_user_num)(chain_id)
                    (committee_mroot)(confirmed_block_number)(highest_block_number)(unconfirmed_blocks)(table_extension) )
FC_REFLECT(ultrainio::chain::resources_lease, (owner)(lease_num)(start_block_height)(end_block_height)(modify_block_height) )
