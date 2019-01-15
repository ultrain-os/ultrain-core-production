/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <ultrainio.system/native.hpp>
#include <ultrainiolib/asset.hpp>
#include <ultrainiolib/time.hpp>
#include <ultrainiolib/privileged.hpp>
#include <ultrainiolib/singleton.hpp>
#include <ultrainio.system/exchange_state.hpp>
#include <ultrainiolib/block_header.hpp>
#include <ultrainiolib/ultrainio.hpp>
#include <string>
#include <vector>

namespace ultrainiosystem {

   using ultrainio::asset;
   using ultrainio::indexed_by;
   using ultrainio::const_mem_fun;
   using ultrainio::block_timestamp;

   const uint64_t master_chain_name = 0;
   const uint64_t pending_queue = std::numeric_limits<uint64_t>::max();
   const uint64_t default_chain_name = N(default);  //default chain, will be assigned by system.

   struct name_bid {
     account_name            newname;
     account_name            high_bidder;
     int64_t                 high_bid = 0; ///< negative high_bid == closed auction waiting to be claimed
     uint64_t                last_bid_time = 0;

     auto     primary_key()const { return newname;                          }
     uint64_t by_high_bid()const { return static_cast<uint64_t>(-high_bid); }
   };

   typedef ultrainio::multi_index< N(namebids), name_bid,
                               indexed_by<N(highbid), const_mem_fun<name_bid, uint64_t, &name_bid::by_high_bid>  >
                               >  name_bid_table;


   struct ultrainio_global_state : ultrainio::blockchain_parameters {
      uint64_t free_ram()const { return max_ram_size - total_ram_bytes_reserved; }
      uint64_t             max_ram_size = 12ll*1024 * 1024 * 1024;
      int64_t              min_activated_stake   = 150'000'000'0000;
      uint32_t             min_committee_member = 1000;
      uint32_t             min_committee_member_number = 4;
      uint64_t             total_ram_bytes_reserved = 0;
      int64_t              total_ram_stake = 0;

      uint64_t             start_block =0;
      uint64_t             last_pervote_bucket_fill = 0;
      int64_t              pervote_bucket = 0;
      int64_t              perblock_bucket = 0;
      uint64_t             total_unpaid_blocks = 0; /// all blocks which have been produced but not paid
      int64_t              total_activated_stake = 0;
      uint64_t             thresh_activated_stake_time = 0;
      double               total_producer_vote_weight = 0; /// the sum of all producer votes
      block_timestamp      last_name_close;
      uint16_t             max_resources_size = 10000;    //set the resource combo to 10000
      uint16_t             total_resources_staked = 0;
      uint64_t             defer_trx_nextid = 0;
      time                 last_check_resexpiretime = 0;
      time                 last_vote_expiretime = 0;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE_DERIVED( ultrainio_global_state, ultrainio::blockchain_parameters,
                                (max_ram_size)(min_activated_stake)(min_committee_member)(min_committee_member_number)
                                (total_ram_bytes_reserved)(total_ram_stake)(start_block)(last_pervote_bucket_fill)
                                (pervote_bucket)(perblock_bucket)(total_unpaid_blocks)(total_activated_stake)(thresh_activated_stake_time)
                                (total_producer_vote_weight)(last_name_close)(max_resources_size)(total_resources_staked)(defer_trx_nextid)(last_check_resexpiretime)(last_vote_expiretime) )
   };

   struct chain_resource {
       uint16_t             max_resources_size = 10000;
       uint16_t             total_resources_staked = 0;
       uint64_t             max_ram_size = 12ll*1024 * 1024 * 1024;
       uint64_t             total_ram_bytes_reserved = 0;

       ULTRAINLIB_SERIALIZE(chain_resource, (max_resources_size)(total_resources_staked)(max_ram_size)(total_ram_bytes_reserved) )
   };

   struct role_base {
      account_name          owner;
      std::string           producer_key; /// a packed public key objec

      ULTRAINLIB_SERIALIZE(role_base, (owner)(producer_key) )
   };

   struct producer_info : public role_base {
      int64_t               total_cons_staked = 0;
      bool                  is_active = true;
      bool                  is_enabled = false;
      bool                  hasactived = false;
      std::string           url;
      uint64_t              unpaid_blocks = 0;
      uint64_t              total_produce_block;
      uint64_t              last_claim_time = 0;
      uint64_t              location = 0;
      uint64_t              last_operate_blocknum = 0;
      account_name          claim_rewards_account;
      uint64_t primary_key()const { return owner;                                   }
      double   by_votes()const    { return is_active ? -total_cons_staked : total_cons_staked;  }
      bool     active()const      { return is_active;                               }
      void     deactivate()       { producer_key = std::string(); is_active = false; }
      bool     is_on_master_chain() const  {return location == master_chain_name;}
      bool     is_in_pending_queue() const  {return location == pending_queue;}
      bool     is_on_subchain() const      {return location != master_chain_name && location != pending_queue;}

      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE_DERIVED( producer_info, role_base, (total_cons_staked)(is_active)(is_enabled)(hasactived)(url)
                        (unpaid_blocks)(total_produce_block)(last_claim_time)(location)(last_operate_blocknum)(claim_rewards_account) )
   };

   struct pending_miner {
            account_name                               owner;
            std::vector<ultrainio::proposeminer_info>  proposal_miner;
            std::vector<ultrainio::provided_proposer>  provided_approvals;
            auto primary_key()const { return owner; }
            ULTRAINLIB_SERIALIZE(pending_miner, (owner)(proposal_miner)(provided_approvals) )
         };
   struct pending_acc {
         account_name                               owner;
         std::vector<ultrainio::proposeaccount_info>  proposal_account;
         std::vector<ultrainio::provided_proposer>  provided_approvals;
         auto primary_key()const { return owner; }
         ULTRAINLIB_SERIALIZE(pending_acc, (owner)(proposal_account)(provided_approvals) )
      };
   struct pending_res {
         account_name                               owner;
         std::vector<ultrainio::proposeresource_info>  proposal_resource;
         std::vector<ultrainio::provided_proposer>  provided_approvals;
         auto primary_key()const { return owner; }
         ULTRAINLIB_SERIALIZE(pending_res, (owner)(proposal_resource)(provided_approvals) )
      };
   struct hash_vote {
       hash_vote(checksum256 hash, uint64_t vote):hash(hash), votes(vote){}
       hash_vote(){}
       checksum256      hash;
       uint64_t         votes;
       ULTRAINLIB_SERIALIZE(hash_vote , (hash)(votes) )
   };

   static constexpr uint32_t default_worldstate_interval = 30;
   static constexpr uint32_t MAX_WS_COUNT                = 5;

   struct subchain_ws_hash {
       uint64_t             block_num;
       std::vector<hash_vote>    hash_v;
       std::vector<account_name> accounts;
       uint64_t  primary_key()const { return block_num; }
       ULTRAINLIB_SERIALIZE( subchain_ws_hash , (block_num)(hash_v)(accounts) )
   };
   typedef ultrainio::multi_index<N(pendingminer),pending_miner> pendingminers;
   typedef ultrainio::multi_index<N(pendingacc),pending_acc> pendingaccounts;
   typedef ultrainio::multi_index<N(pendingres),pending_res> pendingresource;
   typedef ultrainio::multi_index< N(producers), producer_info,
                               indexed_by<N(prototalvote), const_mem_fun<producer_info, double, &producer_info::by_votes>  >
                               >  producers_table;

   typedef ultrainio::singleton<N(global), ultrainio_global_state> global_state_singleton;

   typedef ultrainio::singleton<N(pendingque), std::vector<role_base>> pending_queue_singleton;
   typedef ultrainio::multi_index< N(wshash), subchain_ws_hash>      subchain_hash_table;

   struct chaintype {
       uint16_t id;
       uint32_t min_producers;
       uint32_t max_producers;
       uint16_t consensus_period;

       auto primary_key() const { return uint64_t(id); }

       ULTRAINLIB_SERIALIZE(chaintype, (id)(min_producers)(max_producers)(consensus_period) );
   };
   typedef ultrainio::multi_index< N(chaintype), chaintype > chaintypes_table;

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
       uint16_t                  chain_type;
       time                      genesis_time;
       chain_resource            global_resource;
       bool                      is_active;
       bool                      is_synced;
       std::vector<role_base>    committee_members;
       updated_committee         updated_info;
       changing_committee        changing_info;
       block_id_type             head_block_id;
       uint32_t                  head_block_num;
       std::vector<user_info>    users;
//       checksum256             chain_id;
//       std::string             genesis_info;
//       std::string             network_topology;   //ignore it now, todo, will re-design it after dynamic p2p network feature implemented
//       std::vector<role_base>  relayer_candidates; //relayer only with deposit， not in committee list
//       std::vector<role_base>  relayer_list;       // choosen from accounts with enough deposit (both producer and non-producer)

       auto primary_key()const { return chain_name; }

       uint32_t get_subchain_min_miner_num() const { return chain_type == 1 ? 20 : 5;}
       uint32_t get_subchain_max_miner_num() const {return chain_type == 1 ? 1000 : 20;}

       ULTRAINLIB_SERIALIZE(subchain, (chain_name)(chain_type)(genesis_time)(global_resource)(is_active)(is_synced)(committee_members)
                                      (updated_info)(changing_info)(head_block_id)(head_block_num)(users) )
   };
   typedef ultrainio::multi_index<N(subchains), subchain> subchains_table;

   struct resources_lease {
      account_name   owner;
      int64_t        lease_num = 0;
      time           start_time = 0;
      time           end_time;

      uint64_t  primary_key()const { return owner; }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE( resources_lease, (owner)(lease_num)(start_time)(end_time) )
   };
   typedef ultrainio::multi_index< N(reslease), resources_lease>      resources_lease_table;
/*
   struct empower_info {
      uint64_t          chain_name;
      std::vector<user_info>  users;

      auto primary_key()const { return chain_name; }
   };
   typedef ultrainio::multi_index<N(users), empower_info> user_table;
*/
   //   static constexpr uint32_t     max_inflation_rate = 5;  // 5% annual inflation
   static constexpr int64_t  consweight_per_subaccount = 1000'000'0000;
   static constexpr uint32_t seconds_per_day       = 24 * 3600;
   static constexpr uint32_t seconds_per_year      = 52*7*24*3600;
   static constexpr uint64_t useconds_per_day      = 24 * 3600 * uint64_t(1000000);
   static constexpr uint64_t seconds_per_halfhour  = 30 * 60;
   static constexpr uint64_t useconds_per_year     = seconds_per_year*1000000ll;

   static constexpr uint64_t     system_token_symbol = CORE_SYMBOL;

   class system_contract : public native {
      private:
         producers_table        _producers;
         global_state_singleton _global;

         ultrainio_global_state   _gstate;
         rammarket                _rammarket;
         pending_queue_singleton  _pending_que;
         subchains_table          _subchains;
         pendingminers            _pendingminer;
         pendingaccounts          _pendingaccount;
         pendingresource          _pendingres;
//         user_table               _users;

      public:
         system_contract( account_name s );
         ~system_contract();

         // Actions:
         void onblock( block_timestamp timestamp, account_name producer );
                      // const block_header& header ); /// only parse first 3 fields of block header
         // functions defined in delegate_bandwidth.cpp

         void resourcelease( account_name from, account_name receiver,
                          int64_t combosize, int64_t days, uint64_t location = master_chain_name);

         void delegatecons( account_name from, account_name receiver,asset stake_net_quantity);
         void undelegatecons( account_name from, account_name receiver);

         /**
          *  This action is called after the delegation-period to claim all pending
          *  unstaked tokens belonging to owner
          */
         void refundcons( account_name owner );

         // functions defined in voting.cpp

         void regproducer( const account_name producer, const std::string& producer_key, const std::string& url, uint64_t location, account_name rewards_account );

         void unregprod( const account_name producer );

         void setram( uint64_t max_ram_size );

         void setparams( const ultrainio::blockchain_parameters& params );

         // functions defined in producer_pay.cpp
         void claimrewards();

         void setpriv( account_name account, uint8_t ispriv );

         void rmvproducer( account_name producer );

         void bidname( account_name bidder, account_name newname, asset bid );

         void updateactiveminers(const ultrainio::proposeminer_info& miners );

         void add_subchain_account(const ultrainio::proposeaccount_info& newacc );
        // functions defined in scheduler.cpp
         void regsubchain(uint64_t chain_name, uint16_t chain_type, time genesis_time);
         void reportsubchainhash(uint64_t subchain, uint64_t blocknum, checksum256 hash);

         void acceptheader (uint64_t chain_name,
                            const std::vector<ultrainio::block_header>& headers);
//                          const std::string& aggregatedEcho,
//                          const std::vector<uint32_t>& echo_weight_vector,
//                          const std::vector<std::string>& echo_account_vector);

         void clearchain(uint64_t chain_name, bool users_only);
         void empoweruser(account_name user, const std::string& owner_pk, const std::string& active_pk, uint64_t chain_name);
         //Register to ba a relayer candidate of a subchain, only for those accounts which are not in the committee list of this subchain.
         //All committee members are also be relayer candidates automatically
/*         void register_relayer(const std::string& miner_pk,
                               account_name relayer_account_name,
                               uint32_t relayer_deposit,
                               const std::string& ip,
                               uint64_t chain_name); */
         void regchaintype(uint16_t type_id, uint32_t min_producer_num, uint32_t max_producer_num, uint16_t consensus_period);

         void votecommittee();

         void voteaccount();

         void voteresourcelease();

         void recycleresource(const account_name owner ,uint64_t lease_num);
      private:
         inline void update_activated_stake(int64_t stake);

         // Implementation details:

         //defind in delegate_bandwidth.cpp
         void change_cons( account_name from, account_name receiver, asset stake_cons_quantity);

         //defined in voting.hpp
         static ultrainio_global_state get_default_parameters();

         //defined in producer_pay.cpp
         void reportblocknumber( account_name producer, uint64_t number);

         //defined in scheduler.cpp
         void add_to_pending_queue(account_name producer, const std::string& public_key);

         void remove_from_pending_queue(account_name producer);

         void add_to_subchain(uint64_t chain_name, account_name producer, const std::string& public_key);

         void remove_from_subchain(uint64_t chain_name, account_name producer);

         void activate_committee_update(); //called in onblock, loop for all subchains and activate their committee update

         void getKeydata(const std::string& pubkey,std::array<char,33> & data);

         void checkresexpire();

         void cleanvotetable();

         chaintype get_subchain_basic_info(uint16_t chain_type) const;

         void syncresource(account_name receiver, int64_t combosize, time endtime);

         void distributreward();
   };

} /// ultrainiosystem
