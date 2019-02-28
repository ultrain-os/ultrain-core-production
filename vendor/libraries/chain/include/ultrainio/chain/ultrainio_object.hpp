#include <ultrainio/chain/types.hpp>

namespace ultrainio { namespace chain {

    const uint64_t master_chain_name = 0;

    // keep this the same as defined in ultrainio.system.hpp;
    struct role_base {
      account_name          owner;
      std::string           producer_key; /// a packed public key objec
      std::string           bls_key;
    };

    // keep this the same as defined in ultrainio.system.hpp;
    struct producer_info : public role_base {
      int64_t               total_cons_staked = 0;
      bool                  is_enabled = false;
      std::string           url;
      uint64_t              unpaid_balance = 0;
      uint64_t              total_produce_block = 0;
      uint64_t              location = 0;
      uint64_t              last_operate_blocknum = 0;
      uint64_t              delegated_cons_blocknum = 0;
      account_name          claim_rewards_account;
      uint64_t              vote_number = 0;
      uint64_t              last_vote_blocknum = 0;
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

   struct updated_committee {
       std::vector<role_base>    deprecated_committee;
       std::vector<account_name> unactivated_committee;
       uint32_t                  take_effect_at_block; //block num of master chain when the committee update takes effect
   };

   struct block_header_digest {
       account_name              proposer;
       block_id_type             block_id;
       uint32_t                  block_number;
       checksum256_type          transaction_mroot;
   };

   struct unconfirmed_block_header : public block_header_digest {
       uint16_t                  fork_id;
       bool                      to_be_paid;    //should block proposer be paid when this block was confirmed
       bool                      is_leaf;
       checksum256_type          committee_mroot;
   };

    struct subchain {
       uint64_t                  chain_name;
       uint64_t                  chain_type;
       chain::block_timestamp    genesis_time;
       chain_resource            global_resource;
       bool                      is_active;
       bool                      is_synced;
       bool                      is_schedulable;
       std::vector<role_base>    committee_members;  //all producers with enough deposit
       updated_committee         updated_info;
       changing_committee        changing_info;
       std::vector<user_info>    recent_users;
       uint32_t                  total_user_num;
       checksum256_type          chain_id;
       checksum256_type          committee_mroot;
       uint32_t                  confirmed_block_number;
       uint32_t                  highest_block_number;
       std::vector<unconfirmed_block_header>  unconfirmed_blocks;
    };

    struct resources_lease {
      account_name             owner;
      uint64_t                 lease_num;
      uint32_t                 start_block_height;
      uint32_t                 end_block_height;
      uint32_t                 modify_block_height;
    };

}} // namespace ultrainio::chain

FC_REFLECT(ultrainio::chain::role_base, (owner)(producer_key)(bls_key) )
FC_REFLECT_DERIVED(ultrainio::chain::producer_info, (ultrainio::chain::role_base), (total_cons_staked)(is_enabled)
                    (url)(unpaid_balance)(total_produce_block)(location)(last_operate_blocknum)(delegated_cons_blocknum)(claim_rewards_account)(vote_number)(last_vote_blocknum))
FC_REFLECT(ultrainio::chain::chain_resource, (max_resources_number)(total_resources_used_number)(max_ram_size)(total_ram_bytes_used) )
FC_REFLECT(ultrainio::chain::user_info, (user_name)(owner_key)(active_key)(emp_time)(is_producer) )
FC_REFLECT(ultrainio::chain::changing_committee, (removed_members)(new_added_members) )
FC_REFLECT(ultrainio::chain::updated_committee, (deprecated_committee)(unactivated_committee)(take_effect_at_block) )
FC_REFLECT(ultrainio::chain::block_header_digest, (proposer)(block_id)(block_number)(transaction_mroot) )
FC_REFLECT_DERIVED(ultrainio::chain::unconfirmed_block_header, (ultrainio::chain::block_header_digest), (fork_id)(to_be_paid)
                    (is_leaf)(committee_mroot) )
FC_REFLECT(ultrainio::chain::subchain, (chain_name)(chain_type)(genesis_time)(global_resource)(is_active)(is_synced)(is_schedulable)
                                       (committee_members)(updated_info)(changing_info)(recent_users)(total_user_num)(chain_id)
                                       (committee_mroot)(confirmed_block_number)(highest_block_number)(unconfirmed_blocks) )
FC_REFLECT(ultrainio::chain::resources_lease, (owner)(lease_num)(start_block_height)(end_block_height)(modify_block_height) )
