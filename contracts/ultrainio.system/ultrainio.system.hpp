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

namespace ultrainiosystem {

   using ultrainio::asset;
   using ultrainio::indexed_by;
   using ultrainio::const_mem_fun;
   using ultrainio::block_timestamp;

   const int num_rate = 7;
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
      uint64_t             total_unpaid_blocks[num_rate] {}; /// all blocks which have been produced but not paid
      int64_t              total_activated_stake = 0;
      uint64_t             thresh_activated_stake_time = 0;
      double               total_producer_vote_weight = 0; /// the sum of all producer votes
      block_timestamp      last_name_close;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE_DERIVED( ultrainio_global_state, ultrainio::blockchain_parameters,
                                (max_ram_size)(min_activated_stake)(min_committee_member)(min_committee_member_number)
                                (total_ram_bytes_reserved)(total_ram_stake)(start_block)(last_pervote_bucket_fill)
                                (pervote_bucket)(perblock_bucket)(total_unpaid_blocks)(total_activated_stake)(thresh_activated_stake_time)
                                (total_producer_vote_weight)(last_name_close) )
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
      uint64_t              unpaid_blocks[num_rate] {};
      uint64_t              total_produce_block;
      uint64_t              last_claim_time = 0;
      uint64_t              location = 0;

      uint64_t primary_key()const { return owner;                                   }
      double   by_votes()const    { return is_active ? -total_cons_staked : total_cons_staked;  }
      bool     active()const      { return is_active;                               }
      void     deactivate()       { producer_key = std::string(); is_active = false; }
      bool     is_on_master_chain() const  {return location == master_chain_name;}
      bool     is_in_pending_queue() const  {return location == pending_queue;}
      bool     is_on_subchain() const      {return location != master_chain_name && location != pending_queue;}

      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE_DERIVED( producer_info, role_base, (total_cons_staked)(is_active)(is_enabled)(hasactived)(url)
                        (unpaid_blocks)(total_produce_block)(last_claim_time)(location) )
   };

   struct pendingminer {
            uint64_t                   index = 0;
            std::vector<account_name>       proposal_miner;
            std::vector<account_name>       provided_approvals;
            auto primary_key()const { return index; }
         };
   typedef ultrainio::multi_index<N(pendingminer),pendingminer> pendingminers;

   typedef ultrainio::multi_index< N(producers), producer_info,
                               indexed_by<N(prototalvote), const_mem_fun<producer_info, double, &producer_info::by_votes>  >
                               >  producers_table;

   typedef ultrainio::singleton<N(global), ultrainio_global_state> global_state_singleton;

   typedef ultrainio::singleton<N(pendingque), std::vector<role_base>> pending_queue_singleton;

   struct subchain {
       uint64_t                chain_name;
       uint16_t                chain_type;
       bool                    is_active;
       std::vector<role_base>  committee_members;  //all producers with enough deposit
       block_id_type           head_block_id;
       uint32_t                head_block_num;
       checksum256             chain_id;
       std::string             genesis_info;
       std::string             network_topology;   //ignore it now, todo, will re-design it after dynamic p2p network feature implemented
       std::vector<role_base>  relayer_candidates; //relayer only with deposit， not in committee list
       std::vector<role_base>  relayer_list;       // choosen from accounts with enough deposit (both producer and non-producer)

       auto primary_key()const { return chain_name; }

       uint32_t get_subchain_min_miner_num() const { return chain_type == 1 ? 10 : 7;}

       uint32_t get_subchain_max_miner_num() const {return chain_type == 2 ? 12 : 10000;};

       ULTRAINLIB_SERIALIZE(subchain, (chain_name)(chain_type)(is_active)(committee_members)(head_block_id)(head_block_num)(chain_id)
                                      (genesis_info)(network_topology)(relayer_candidates)(relayer_list) )
   };
   typedef ultrainio::multi_index<N(subchains), subchain> subchains_table;

   //   static constexpr uint32_t     max_inflation_rate = 5;  // 5% annual inflation
   static constexpr uint32_t seconds_per_day       = 24 * 3600;
   static constexpr uint32_t seconds_per_year      = 52*7*24*3600;
   static constexpr uint64_t useconds_per_day      = 24 * 3600 * uint64_t(1000000);
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

      public:
         system_contract( account_name s );
         ~system_contract();

         // Actions:
         void onblock( block_timestamp timestamp, account_name producer );
                      // const block_header& header ); /// only parse first 3 fields of block header
         // functions defined in delegate_bandwidth.cpp

         /**
          *  Stakes SYS from the balance of 'from' for the benfit of 'receiver'.
          *  If transfer == true, then 'receiver' can unstake to their account
          *  Else 'from' can unstake at any time.
          */
         void delegatebw( account_name from, account_name receiver,
                          asset stake_net_quantity, asset stake_cpu_quantity, bool transfer );


         /**
          *  Decreases the total tokens delegated by from to receiver and/or
          *  frees the memory associated with the delegation if there is nothing
          *  left to delegate.
          *
          *  This will cause an immediate reduction in net/cpu bandwidth of the
          *  receiver.
          *
          *  A transaction is scheduled to send the tokens back to 'from' after
          *  the staking period has passed. If existing transaction is scheduled, it
          *  will be canceled and a new transaction issued that has the combined
          *  undelegated amount.
          *
          *  The 'from' account loses voting power as a result of this call and
          *  all producer tallies are updated.
          */
         void undelegatebw( account_name from, account_name receiver,
                            asset unstake_net_quantity, asset unstake_cpu_quantity );

         void delegatecons( account_name from, account_name receiver,asset stake_net_quantity);
         void undelegatecons( account_name from, account_name receiver,asset unstake_net_quantity);
         /**
          * Increases receiver's ram quota based upon current price and quantity of
          * tokens provided. An inline transfer from receiver to system contract of
          * tokens will be executed.
          */
         void buyram( account_name buyer, account_name receiver, asset tokens );
         void buyrambytes( account_name buyer, account_name receiver, uint32_t bytes );

         /**
          *  Reduces quota my bytes and then performs an inline transfer of tokens
          *  to receiver based upon the average purchase price of the original quota.
          */
         void sellram( account_name receiver, int64_t bytes );

         /**
          *  This action is called after the delegation-period to claim all pending
          *  unstaked tokens belonging to owner
          */
         void refund( account_name owner );

         void refundcons( account_name owner );

         // functions defined in voting.cpp

         void regproducer( const account_name producer, const std::string& producer_key, const std::string& url, uint64_t location );

         void unregprod( const account_name producer );

         void setram( uint64_t max_ram_size );

         void setparams( const ultrainio::blockchain_parameters& params );

         // functions defined in producer_pay.cpp
         void claimrewards( const account_name& owner );

         void setpriv( account_name account, uint8_t ispriv );

         void rmvproducer( account_name producer );

         void bidname( account_name bidder, account_name newname, asset bid );

         void updateactiveminers(const std::vector<ultrainio::proposeminer_info>& miners );

        // functions defined in scheduler.cpp
         void regsubchain(uint64_t chain_name, uint16_t chain_type);

         void acceptheader (uint64_t chain_name,
                            const ultrainio::block_header& header);
//                          const std::string& aggregatedEcho,
//                          const std::vector<uint32_t>& echo_weight_vector,
//                          const std::vector<std::string>& echo_account_vector);

         void clearblock(uint64_t chain_name);
         //Register to ba a relayer candidate of a subchain, only for those accounts which are not in the committee list of this subchain.
         //All committee members are also be relayer candidates automatically
/*         void register_relayer(const std::string& miner_pk,
                               account_name relayer_account_name,
                               uint32_t relayer_deposit,
                               const std::string& ip,
                               uint64_t chain_name); */

         void votecommittee();
      private:
         inline void update_activated_stake(int64_t stake);

         // Implementation details:

         //defind in delegate_bandwidth.cpp
         void changebw( account_name from, account_name receiver,
                        asset stake_net_quantity, asset stake_cpu_quantity, bool transfer );

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
   };

} /// ultrainiosystem
