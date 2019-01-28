#include <ultrainio/chain/types.hpp>

namespace ultrainio { namespace chain {

    const uint64_t master_chain_name = 0;

    struct role_base {
      account_name          owner;
      std::string           producer_key; /// a packed public key objec
      std::string           bls_key;
    };

    struct producer_info : public role_base {
      int64_t               total_cons_staked = 0;
      bool                  is_active = true;
      bool                  is_enabled = false;
      bool                  hasenabled = false;
      std::string           url;
      uint64_t              unpaid_blocks = 0;
      uint64_t              total_produce_block = 0;
      uint64_t              location = 0;
      uint64_t              vote_number = 0;
      uint64_t              last_vote_blocknum = 0;
   };

   struct chain_resource {
       uint16_t             max_resources_size = 10000;
       uint16_t             total_resources_staked = 0;
       uint64_t             max_ram_size = 12ll*1024 * 1024 * 1024;
       uint64_t             total_ram_bytes_reserved = 0;
   };

   struct user_info {
      account_name      user_name;
      std::string       owner_key;
      std::string       active_key;
      uint64_t          emp_time;
      uint32_t          block_num; ////block num in master chain when this info added
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

    struct subchain {
       uint64_t                  chain_name;
       uint64_t                  chain_type;
       chain::block_timestamp    genesis_time;
       chain_resource            global_resource;
       bool                      is_active;
       bool                      is_synced;
       std::vector<role_base>    committee_members;  //all producers with enough deposit
       updated_committee         updated_info;
       changing_committee        changing_info;
       block_id_type             head_block_id;
       uint32_t                  head_block_num;
       std::vector<user_info>    users;
       checksum256_type          chain_id;
//       std::string             genesis_info;
//       std::string             network_topology;   //ignore it now, todo, will re-design it after dynamic p2p network feature implemented
//       std::vector<role_base>  relayer_candidates; //relayer only with deposit， not in committee list
//       std::vector<role_base>  relayer_list;       // choosen from accounts with enough deposit (both producer and non-producer)
    };

    struct resources_lease {
      account_name             owner;
      uint64_t                 lease_num;
      time_point_sec           start_time;
      time_point_sec           end_time;
    };

}} // namespace ultrainio::chain

FC_REFLECT(ultrainio::chain::role_base, (owner)(producer_key)(bls_key) )
FC_REFLECT_DERIVED(ultrainio::chain::producer_info, (ultrainio::chain::role_base), (total_cons_staked)(is_active)(is_enabled)
                    (hasenabled)(url)(unpaid_blocks)(total_produce_block)(location)(vote_number)(last_vote_blocknum))
FC_REFLECT(ultrainio::chain::chain_resource, (max_resources_size)(total_resources_staked)(max_ram_size)(total_ram_bytes_reserved) )
FC_REFLECT(ultrainio::chain::user_info, (user_name)(owner_key)(active_key)(emp_time)(block_num) )
FC_REFLECT(ultrainio::chain::changing_committee, (removed_members)(new_added_members) )
FC_REFLECT(ultrainio::chain::updated_committee, (deprecated_committee)(unactivated_committee)(take_effect_at_block) )
FC_REFLECT(ultrainio::chain::subchain, (chain_name)(chain_type)(genesis_time)(global_resource)(is_active)(is_synced)(committee_members)
                                       (updated_info)(changing_info)(head_block_id)(head_block_num)(users)(chain_id) )
                                 //(genesis_info)(network_topology)(relayer_candidates)(relayer_list) )
FC_REFLECT(ultrainio::chain::resources_lease, (owner)(lease_num)(start_time)(end_time) )
