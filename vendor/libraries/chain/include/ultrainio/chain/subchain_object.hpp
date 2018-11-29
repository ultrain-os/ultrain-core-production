#include <ultrainio/chain/types.hpp>

namespace ultrainio { namespace chain {

    struct role_base {
      account_name          owner;
      std::string           producer_key; /// a packed public key objec
    };

    struct subchain {
       uint64_t                chain_name;
       uint16_t                chain_type;
       bool                    is_active;
       std::vector<role_base>  committee_members;  //all producers with enough deposit
       block_id_type           head_block_id;
       uint32_t                head_block_num;
       checksum256_type        chain_id;
       std::string             genesis_info;
       std::string             network_topology;   //ignore it now, todo, will re-design it after dynamic p2p network feature implemented
       std::vector<role_base>  relayer_candidates; //relayer only with deposit， not in committee list
       std::vector<role_base>  relayer_list;       // choosen from accounts with enough deposit (both producer and non-producer)
    };
}} // namespace ultrainio::chain

FC_REFLECT(ultrainio::chain::role_base, (owner)(producer_key) )
FC_REFLECT(ultrainio::chain::subchain, (chain_name)(chain_type)(is_active)(committee_members)(head_block_id)(head_block_num)
           (chain_id)(genesis_info)(network_topology)(relayer_candidates)(relayer_list) )
