#include <ultrainio/chain/types.hpp>

namespace ultrainio { namespace chain {

    const int num_rate = 7;
    const uint64_t master_chain_name = 0;

    struct role_base {
      account_name          owner;
      std::string           producer_key; /// a packed public key objec
    };

    struct producer_info : public role_base {
      int64_t               total_cons_staked = 0;
      bool                  is_active = true;
      bool                  is_enabled = false;
      bool                  hasactived = false;
      std::string           url;
      uint64_t              unpaid_blocks[num_rate] {};
      uint64_t              total_produce_block = 0;
      uint64_t              last_claim_time = 0;
      uint64_t              location = 0;
   };

   struct user_info {
      account_name      user_name;
      std::string       owner_key;
      std::string       active_key;
      uint64_t          emp_time;
      uint32_t          block_num; ////block num in master chain when this info added
   };

   struct changed_committee {
       std::vector<role_base> deprecated_members;
       std::vector<role_base> new_added_members;
       uint32_t               block_num = 0;  //block num of master chain when this change was comfirmed,
                                              //0 indicate all changed info has not bee confirmed.
   };

    struct subchain {
       uint64_t                  chain_name;
       uint16_t                  chain_type;
       bool                      is_active;
       bool                      is_synced;
       std::vector<role_base>    committee_members;  //all producers with enough deposit
       std::vector<role_base>    deprecated_committee;
       std::vector<account_name> unactivated_committee;
       changed_committee         changed_info;
       block_id_type             head_block_id;
       uint32_t                  head_block_num;
       std::vector<user_info>    users;
//       checksum256_type        chain_id;
//       std::string             genesis_info;
//       std::string             network_topology;   //ignore it now, todo, will re-design it after dynamic p2p network feature implemented
//       std::vector<role_base>  relayer_candidates; //relayer only with deposit， not in committee list
//       std::vector<role_base>  relayer_list;       // choosen from accounts with enough deposit (both producer and non-producer)
    };

}} // namespace ultrainio::chain

FC_REFLECT(ultrainio::chain::role_base, (owner)(producer_key) )
FC_REFLECT_DERIVED(ultrainio::chain::producer_info, (ultrainio::chain::role_base), (total_cons_staked)(is_active)(is_enabled)
                    (hasactived)(url)(unpaid_blocks)(total_produce_block)(last_claim_time)(location))
FC_REFLECT(ultrainio::chain::user_info, (user_name)(owner_key)(active_key)(emp_time)(block_num) )
FC_REFLECT(ultrainio::chain::changed_committee, (deprecated_members)(new_added_members)(block_num) )
FC_REFLECT(ultrainio::chain::subchain, (chain_name)(chain_type)(is_active)(is_synced)(committee_members)(deprecated_committee)
                                       (unactivated_committee)(changed_info)(head_block_id)(head_block_num)(users) )
           //(chain_id)(genesis_info)(network_topology)(relayer_candidates)(relayer_list) )
