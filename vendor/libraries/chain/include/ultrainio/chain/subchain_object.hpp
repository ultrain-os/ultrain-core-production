#include <ultrainio/chain/types.hpp>

namespace ultrainio { namespace chain {

    struct role_base {
      account_name          owner;
      std::string           producer_key; /// a packed public key objec
    };

    struct subchain_base {
        uint64_t                 chain_name;
        int64_t                  min_active_stake;
        uint32_t                 min_committee_member_num;
        account_name             root_name;
        std::string              root_pk;
        std::vector<role_base>   committee_members;  //all producers with enough deposit
        uint64_t                 total_deposit;
        block_id_type            head_block_id;
        uint32_t                 head_block_num;
    };

    struct subchain : public subchain_base {
        checksum256_type        chain_id;
        std::string             genesis_info;
        std::string             network_topology;   //ignore it now, todo, will re-design it after dynamic p2p network feature implemented
        uint32_t                relayer_stake_threshold;
        std::vector<role_base>  relayer_candidates; //relayer only with deposit， not in committee list
        std::vector<role_base>  relayer_list;       // choosen from accounts with enough deposit (both producer and non-producer)
    };
}} // namespace ultrainio::chain

FC_REFLECT(ultrainio::chain::role_base, (owner)(producer_key) )
FC_REFLECT(ultrainio::chain::subchain_base, (chain_name)(min_active_stake)(min_committee_member_num)(root_name)(root_pk)
           (committee_members)(total_deposit)(head_block_id)(head_block_num) ) 
FC_REFLECT_DERIVED(ultrainio::chain::subchain, (ultrainio::chain::subchain_base), (chain_id)(genesis_info)(network_topology)
                   (relayer_stake_threshold)(relayer_candidates)(relayer_list) )
