#pragma once
#include <ultrainiolib/asset.hpp>
#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/singleton.hpp>
#include <ultrainiolib/block_header.hpp>

#include <vector>

namespace ultrainio {
    using namespace ultrainio;

    const uint32_t deposit_threshold = 200000;

    //@abi table 
    struct node_base {
        account_name     my_account;
        std::string      my_pk;
        uint32_t         my_deposit;
        std::string      node_ip;

        auto primary_key()const { return this->my_account; }
    };
    typedef ultrainio::multi_index<N(minersque), node_base> miners_queue;

    //@abi table 
    struct node : public node_base {
        uint64_t         chain_name;
        uint64_t         dest_chain_name;      //if it's different from chain_id, then the node need to move to this dest chain.
        uint32_t         quit_before_block;  //the block num before which node must quit from current chain, and move to dest chain.
                                             //if chain_id == dest_chain_id, quit_before_block should be 0.

        auto primary_key()const { return this->my_account; }
    };
    typedef ultrainio::multi_index<N(nodes), node> nodes_table;

    //@abi table 
    struct subchain_base {
        uint64_t           chain_name;
        int64_t            min_active_stake;
        uint32_t           min_committee_member_num;
        account_name       root_name;
        std::string        root_pk;
        vector<node_base>  committee_members;  //all producers with enough deposit
        uint64_t           total_deposit;
        block_id_type      head_block_id;
        uint32_t           head_block_num;
    };
    typedef ultrainio::singleton<N(pendingchain), subchain_base> pengding_subchain;

    //@abi table 
    struct subchain : public subchain_base {
        checksum256        chain_id;
        std::string        genesis_info;
        std::string        network_topology;   //ignore it now, todo, will re-design it after dynamic p2p network feature implemented
        uint32_t           relayer_stake_threshold;
        vector<node_base>  relayer_candidates; //relayer only with deposit， not in committee list
        vector<node_base>  relayer_list;       // choosen from accounts with enough deposit (both producer and non-producer)

        auto primary_key()const { return chain_name; }
    };
    typedef ultrainio::multi_index<N(subchains), subchain> subchains_table;

    class scheduler : public contract {
      private:
        nodes_table           _nodes;
        subchains_table       _subchains;
        miners_queue          _miners_que;
        pengding_subchain     _pending_subchain;
      public:
        scheduler( account_name self );

        /// @abi action
        void regminer(account_name miner_account_name,
                            const std::string& miner_pk,
                            uint32_t miner_deposit,
                            const std::string& miner_ip);

        /// @abi action
        void regsubchain(uint64_t chain_name,
                                account_name root_user_name,
                                const std::string& root_user_pk,
                                uint32_t sub_chain_deposit,
                                uint32_t node_num);

        /// @abi action
        void acceptheader (uint64_t chain_name,
                           const block_header& header);
//                         const std::string& aggregatedEcho,
//                         const vector<uint32_t>& echo_weight_vector,
//                         const vector<std::string>& echo_account_vector);

        void join_subchain(const std::string& miner_pk,
                           account_name miner_account_name,
                           uint32_t miner_deposit,
                           const std::string& ip,
                           uint64_t chain_name);
        //Register to ba a relayer candidate of a subchain, only for those accounts which are not in the committee list of this subchain.
        //All committee members are also be relayer candidates automatically
        void register_relayer(const std::string& miner_pk,
                              account_name relayer_account_name,
                              uint32_t relayer_deposit,
                              const std::string& ip,
                              uint64_t chain_name);

      private:
        void start_new_chain();
   };
} //namespace ultrainio
