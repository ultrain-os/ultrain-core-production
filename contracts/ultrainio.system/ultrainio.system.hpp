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
#include <ultrainiolib/block_header.hpp>
#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/types.hpp>
#include <string>
#include <vector>
#include <set>
#include "block_header_ext_key.h"
#include "committee_set.h"

#define UNUSED(a) (void)(a);
namespace ultrainiosystem {
   using namespace ultrainio;
   const name self_chain_name{N(ultrainio)};
   const name default_chain_name{N(default)};  //default chain, will be assigned by system.
   const account_name ultrainio_community_name{N(utrio.cmnity)};
   const account_name ultrainio_technical_team_name{N(utrio.thteam)};
   const account_name ultrainio_dapp_name{N(utrio.dapp)};
   const char* bank_issue_memo = "issue tokens for subchain utrio.bank";
   bool operator!=(const checksum256& sha256_1, const checksum256& sha256_2) {
      for(auto i = 0; i < 32; ++i) {
         if(sha256_1.hash[i] != sha256_2.hash[i]) {
               return true;
         }
      }
      return false;
   }

   struct exten_type {
       exten_type(){}
       exten_type( uint16_t k, const std::string& v ) : key(k), value(v){}
       uint16_t         key;
       std::string      value;
       ULTRAINLIB_SERIALIZE(exten_type , (key)(value) )
   };

   typedef std::vector<exten_type>  exten_types;

   struct ultrainio_global_state : ultrainio::blockchain_parameters {
      uint64_t free_ram()const { return max_ram_size - total_ram_bytes_used; }
      bool is_master_chain()const { return chain_name == name{N(ultrainio)}; }
      uint64_t             max_ram_size = 12ll*1024 * 1024 * 1024;
      int64_t              min_activated_stake   = 42'000'0000;
      uint32_t             min_committee_member_number = 1000;
      uint64_t             total_ram_bytes_used = 0;

      uint64_t             start_block =0;
      std::vector<ultrainio::block_reward> block_reward_vec;
      int64_t              total_cur_chain_block = 0;
      int64_t              master_chain_pay_fee = 0;
      uint64_t             total_unpaid_balance = 0;
      int64_t              total_activated_stake = 0;
      block_timestamp      last_name_close;
      uint64_t             max_resources_number = 10000; //number of total resource packages,same as the total_resource_number field of config.hpp
      uint64_t             total_resources_used_number = 0;
      uint32_t             newaccount_fee = 2000;
      name                 chain_name = name{N(ultrainio)};
      uint32_t             cur_committee_number = 0;
      uint64_t             worldstate_interval  = 1000;
      uint32_t             resource_fee = 100000;
      exten_types          table_extension;

      enum global_state_exten_type_key {
         global_state_key_start = 0,
         update_auth = 1,
         confirm_point_interval = 2,
         sidechain_charge_ratio = 3,
         is_claim_reward = 4,
         free_account_per_res = 5,
         version_number = 6,
         is_allow_buy_res = 7, //Allows a general account to buy resources
         check_user_bulletin = 8,
         allow_undelegate_block_interval = 9,
         refund_delegate_consensus_seconds = 10,
         link_auth_fee = 11,
         res_transfer_fee = 12,
         global_state_key_end
      };

      ULTRAINLIB_SERIALIZE_DERIVED(ultrainio_global_state, ultrainio::blockchain_parameters,
                                   (max_ram_size)(min_activated_stake)(min_committee_member_number)
                                   (total_ram_bytes_used)(start_block)(block_reward_vec)
                                   (total_cur_chain_block)(master_chain_pay_fee)(total_unpaid_balance)
                                   (total_activated_stake)(last_name_close)(max_resources_number)
                                   (total_resources_used_number)(newaccount_fee)(chain_name)
                                   (cur_committee_number)(worldstate_interval)(resource_fee)(table_extension) )
   };

   struct lwc_parameters {
       uint32_t save_blocks_num = 30000;

       ULTRAINLIB_SERIALIZE(lwc_parameters, (save_blocks_num))
   };

   struct chain_resource {
       uint64_t             max_resources_number = 10000;
       uint64_t             total_resources_used_number = 0;
       uint64_t             max_ram_size = 32ll*1024 * 1024 * 1024;
       uint64_t             total_ram_bytes_used = 0;

       ULTRAINLIB_SERIALIZE(chain_resource, (max_resources_number)(total_resources_used_number)
                            (max_ram_size)(total_ram_bytes_used) )
   };

   struct producer_brief {
      account_name          owner;
      name                  location{N(ultrainio)};
      bool                  in_disable = true;
      uint64_t primary_key()const { return owner; }

      bool     is_on_master_chain() const  {return location == self_chain_name;}
      bool     is_on_subchain() const      {
          return (location != self_chain_name) && (location != default_chain_name);
      }

      ULTRAINLIB_SERIALIZE(producer_brief, (owner)(location)(in_disable) )
   };

   struct disabled_producer : public committee_info {
      int64_t               total_cons_staked = 0;
      std::string           url;
      uint64_t              total_produce_block = 0;
      uint64_t              last_operate_blocknum = 0;
      uint64_t              delegated_cons_blocknum = 0;
      account_name          claim_rewards_account;

      uint64_t primary_key()const { return owner; }

      ULTRAINLIB_SERIALIZE_DERIVED( disabled_producer, committee_info, (total_cons_staked)
                                    (url)(total_produce_block)(last_operate_blocknum)
                                    (delegated_cons_blocknum)(claim_rewards_account) )
   };

   struct producer_info : public disabled_producer {
      uint64_t              unpaid_balance = 0;
      uint64_t              vote_number = 0;
      //Record the latest block height of a producer or the block height of the chain when a producer became a producer,
      // so as to judge whether a producer has made blocks and remove the committee
      uint64_t              last_record_blockheight = 0;
      exten_types           table_extension;
      enum producers_state_exten_type_key {
         producers_state_key_start = 0,
         claim_rewards_block_height = 1,
         producers_state_key_end,
      };
      uint64_t primary_key()const { return owner; }
      producer_info() {}
      producer_info(const disabled_producer& dp, uint64_t unpay, uint64_t votes, uint64_t last_record_block)
          :disabled_producer(dp), unpaid_balance(unpay), vote_number(votes), last_record_blockheight(last_record_block) {}

      ULTRAINLIB_SERIALIZE_DERIVED( producer_info, disabled_producer,
                                    (unpaid_balance)(vote_number)(last_record_blockheight)(table_extension) )
   };

   struct unpaid_disproducer {
      account_name   owner;
      uint64_t       unpaid_balance = 0;
      account_name   reward_account;
      exten_types    table_extension;
      uint64_t  primary_key()const { return owner; }
      ULTRAINLIB_SERIALIZE( unpaid_disproducer, (owner)(unpaid_balance)(reward_account)(table_extension) )
   };
   typedef ultrainio::multi_index< N(upaiddisprod), unpaid_disproducer>      unpaid_disprod;

   struct hash_vote {
       hash_vote(checksum256 hash_p, uint64_t size, uint64_t vote, bool val, account_name acc)
           :hash(hash_p),file_size(size),votes(vote),valid(val){accounts.emplace(acc);}
       hash_vote(){}
       checksum256            hash;
       uint64_t               file_size;
       uint64_t               votes;
       bool                   valid;
       std::set<account_name> accounts;
       ULTRAINLIB_SERIALIZE(hash_vote , (hash)(file_size)(votes)(valid)(accounts) )
   };

   static constexpr uint32_t NEAR_WS_CNT  = 3;
   static constexpr uint32_t FAR_WS_CNT   = 5;
   static constexpr uint32_t FAR_WS_MUL   = 10;

   struct subchain_ws_hash {
       uint64_t             block_num;
       std::vector<hash_vote>    hash_v;
       std::set<account_name> accounts;
       exten_types           table_extension;
       uint64_t  primary_key()const { return block_num; }
       ULTRAINLIB_SERIALIZE( subchain_ws_hash , (block_num)(hash_v)(accounts)(table_extension) )
   };

   struct master_chain_info {
       account_name                         owner;
       std::vector<committee_info>               master_prods;
       uint64_t                             block_height = 0;
       block_id_type                        block_id;
       checksum256                          committee_mroot;
       exten_types                          master_chain_ext;
       uint64_t  primary_key()const { return owner; }
       ULTRAINLIB_SERIALIZE(master_chain_info, (owner)(master_prods)(block_height)(block_id)(committee_mroot)(master_chain_ext) )
   };

   typedef ultrainio::multi_index<N(briefprod),producer_brief> producer_brief_table;
   typedef ultrainio::multi_index<N(disableprods), disabled_producer> disabled_producers_table;
   typedef ultrainio::multi_index<N(producers), producer_info> producers_table;
   typedef ultrainio::singleton<N(global), ultrainio_global_state> global_state_singleton;
   typedef ultrainio::multi_index< N(wshash), subchain_ws_hash>      subchain_hash_table;
   typedef ultrainio::multi_index<N(masterinfos),master_chain_info> master_chain_infos;
   typedef ultrainio::singleton<N(lwc), lwc_parameters>  lwc_singleton;

   struct chaintype {
       uint64_t type_id;
       uint16_t stable_min_producers;
       uint16_t stable_max_producers;
       uint16_t sched_inc_step;
       uint16_t consensus_period;
       exten_types  table_extension;

       auto primary_key() const { return type_id; }

       ULTRAINLIB_SERIALIZE(chaintype, (type_id)(stable_min_producers)(stable_max_producers)
                            (sched_inc_step)(consensus_period)(table_extension) )
   };
   typedef ultrainio::multi_index< N(chaintypes), chaintype > chaintypes_table;

   struct user_info {
      account_name      user_name;
      std::string       owner_key;
      std::string       active_key;
      time              emp_time;
      bool              is_producer; //producer will also use pk same with master chain
      uint64_t          block_height;
      bool              updateable = true;
   };

   struct changing_producer : public committee_info {
      uint32_t          block_num;  //master block number when it happens

      ULTRAINLIB_SERIALIZE_DERIVED( changing_producer, committee_info, (block_num))
   };

   struct changing_committee {
       std::vector<changing_producer> removed_members;
       std::vector<committee_info> new_added_members;

       bool empty() const {
           return removed_members.empty() && new_added_members.empty();
       }

       void clear() {
           removed_members.clear();
           new_added_members.clear();
       }
   };

   struct block_header_digest {
       uint32_t                  block_number = 0;
       checksum256               transaction_mroot;
       std::set<std::string>     trx_ids;
       ultrainio::extensions_type           table_extension;

       auto primary_key() const { return uint64_t(block_number); }

       block_header_digest() {}
       block_header_digest(uint32_t b_n, const checksum256& tx_mroot): block_number(b_n), transaction_mroot(tx_mroot) {}

       ULTRAINLIB_SERIALIZE(block_header_digest, (block_number)(transaction_mroot)(trx_ids)(table_extension))
   };

   typedef ultrainio::multi_index<N(blockheaders), block_header_digest> block_table;

   struct unconfirmed_block_header : public ultrainio::signed_block_header {
       block_id_type              block_id;
       uint32_t                   block_number = 0;
       bool                       to_be_paid;    //should block proposer be paid when this block was confirmed
       bool                       is_leaf = true;       //leaf in the fork tree
       bool                       is_synced;
       std::string                next_committee_mroot;
       ultrainio::extensions_type   table_extension;

       unconfirmed_block_header() {}
       unconfirmed_block_header(const ultrainio::signed_block_header& signed_header, const block_id_type& b_id, uint32_t b_n,
                                bool need_pay, bool is_sync) : ultrainio::signed_block_header(signed_header),
                                block_id(b_id), block_number(b_n), to_be_paid(need_pay), is_leaf(true), is_synced(is_sync) {
           for (const auto& e : signed_header.header_extensions) {
                block_header_ext_key key = static_cast<block_header_ext_key>(std::get<0>(e));
                if (key == k_next_committee_mroot) {
                    next_committee_mroot = std::string(std::get<1>(e).begin(), std::get<1>(e).end());
                    break;
                }
           }
       }
       committee_set get_committee_set() {
           for (const auto& e : header_extensions) {
               block_header_ext_key key = static_cast<block_header_ext_key>(std::get<0>(e));
               if (key == k_committee_set) {
                   const std::vector<char>& vc = std::get<1>(e);
                   return committee_set(vc);
               }
           }
           return committee_set();
       }

       ULTRAINLIB_SERIALIZE_DERIVED(unconfirmed_block_header, ultrainio::signed_block_header,(block_id)(block_number)
                                    (to_be_paid)(is_leaf)(is_synced)(next_committee_mroot)(table_extension))
   };

   struct chain_info {
       name                                  chain_name;
       uint64_t                              chain_type;
       block_timestamp                       genesis_time;
       chain_resource                        global_resource;
       bool                                  is_synced;
       bool                                  is_schedulable;
       bool                                  schedule_on = true;
       uint16_t                              committee_num;
       std::vector<committee_info>           deprecated_committee;//keep history producers for un-synced chain, clear it once synced
       changing_committee                    changing_info; //has changed but not be confirmed by subchain's block header.
       std::vector<user_info>                recent_users;
       uint32_t                              total_user_num;
       checksum256                           chain_id;
       checksum256                           committee_mroot;
       uint32_t                              confirmed_block_number;
       block_id_type                         confirmed_block_id;
       std::vector<committee_info>           committee_set;//current committee set reported by chain
       std::vector<unconfirmed_block_header> unconfirmed_blocks;
       exten_types                           table_extension;
       enum chains_state_exten_type_key {
         chains_state_key_start = 0,
         genesis_producer_public_key = 1,
         chains_state_key_end,
       };
       auto primary_key()const { return chain_name; }

       ULTRAINLIB_SERIALIZE(chain_info, (chain_name)(chain_type)(genesis_time)(global_resource)(is_synced)
                            (is_schedulable)(schedule_on)(committee_num)(deprecated_committee)(changing_info)
                            (recent_users)(total_user_num)(chain_id)(committee_mroot)(confirmed_block_number)
                            (confirmed_block_id)(committee_set)(unconfirmed_blocks)(table_extension) )

       void handle_committee_update(committee_delta& cmt_delta) {
           for(auto it_rm = changing_info.removed_members.begin(); it_rm != changing_info.removed_members.end();) {
               if(cmt_delta.check_removed(*it_rm)) {
                   it_rm = changing_info.removed_members.erase(it_rm);
               } else {
                    ++it_rm;
               }
           }
           for(auto it_add = changing_info.new_added_members.begin(); it_add != changing_info.new_added_members.end();) {
               if(cmt_delta.check_added(*it_add)) {
                   it_add = changing_info.new_added_members.erase(it_add);
               } else {
                   ++it_add;
               }
           }
           if(changing_info.empty()) {
               is_schedulable = true;
           }
           if(!cmt_delta.empty()) {
               print("error: un-expected committee update happened \n");
           }
       }
   };
   typedef ultrainio::multi_index<N(chains), chain_info> chains_table;

   struct ultrainio_system_params {
      ultrainio_system_params(){}
      uint64_t             chain_type;
      uint64_t             max_ram_size;
      int64_t              min_activated_stake;
      uint32_t             min_committee_member_number;
      std::vector<block_reward> block_reward_vec;
      uint64_t             max_resources_number;
      uint32_t             newaccount_fee;
      name                 chain_name;
      uint64_t             worldstate_interval;
      uint32_t             resource_fee;
      exten_types          table_extension;
      uint64_t  primary_key()const { return chain_type; }
      ULTRAINLIB_SERIALIZE( ultrainio_system_params,(chain_type)(max_ram_size)(min_activated_stake)
                            (min_committee_member_number)(block_reward_vec)(max_resources_number)
                            (newaccount_fee)(chain_name)(worldstate_interval)(resource_fee)(table_extension) )
   };

   struct resources_lease {
      account_name   owner;
      uint64_t       lease_num = 0;
      uint32_t       start_block_height = 0;
      uint32_t       end_block_height = 0;
      uint32_t       modify_block_height = 0;
      uint32_t       free_account_number = 0;
      exten_types    table_extension;
      uint64_t  primary_key()const { return owner; }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE( resources_lease, (owner)(lease_num)(start_block_height)
                            (end_block_height)(modify_block_height)(free_account_number)(table_extension) )
   };
   typedef ultrainio::multi_index< N(reslease), resources_lease>      resources_lease_table;

   struct free_account {
      account_name   owner;
      uint64_t       acc_num = 0;
      exten_types    table_extension;
      uint64_t  primary_key()const { return owner; }
      ULTRAINLIB_SERIALIZE( free_account, (owner)(acc_num)(table_extension) )
   };
   typedef ultrainio::multi_index< N(freeacc), free_account>      freeaccount;

   struct schedule_setting {
       bool          is_schedule_enabled;
       uint16_t      schedule_period;
       uint16_t      expire_minutes;
   };
   typedef ultrainio::singleton<N(schedset), schedule_setting> sched_set_singleton;

   struct pending_deltable {
      account_name   owner;
      uint64_t  primary_key()const { return owner; }
      ULTRAINLIB_SERIALIZE( pending_deltable, (owner) )
   };
   typedef ultrainio::multi_index< N(penddeltab), pending_deltable>   penddeltable;
   //   static constexpr uint32_t     max_inflation_rate = 5;  // 5% annual inflation

   const uint8_t  add_producer = 0x01;
   const uint8_t  remove_producer = 0x02;
   const uint8_t  add_and_remove = 0x03;

   struct committee_bulletin {
       uint64_t          block_num;
       uint8_t           change_type;
       uint64_t  primary_key()const { return block_num; }
   };
   typedef ultrainio::multi_index< N(cmtbltn), committee_bulletin>    cmtbulletin;

   // sync with core/include/core/Evidence.h
   enum producer_evil_type {
      not_evil = 0,

      // native
      sign_multi_propose = 1, //If there are no blocks issued during a specified period (default three days), tokens are not frozen
      vote_multi_propose = 2, //Broadcast an error echo message

      // contract
      limit_time_not_produce = 1000,
   };
   struct bulletin {
       uint64_t              block_num;
       std::set<string>      action_name;
       exten_types           table_extension;
       uint64_t  primary_key()const { return block_num; }
       ULTRAINLIB_SERIALIZE( bulletin , (block_num)(action_name)(table_extension) )
   };
   typedef ultrainio::multi_index< N(bltn), bulletin>    bulletintab;

   struct evilprod {
       account_name          owner;
       uint16_t              evil_type;
       bool                  is_freeze = false;
       exten_types           table_extension;
       uint64_t  primary_key()const { return owner; }
       ULTRAINLIB_SERIALIZE( evilprod , (owner)(evil_type)(is_freeze)(table_extension) )
   };
   typedef ultrainio::multi_index< N(evilprod), evilprod>    evilprodtab;

   static constexpr uint32_t seconds_per_day       = 24 * 3600;
   static constexpr uint32_t seconds_per_year      = 52*7*24*3600;
   static constexpr uint64_t useconds_per_day      = 24 * 3600 * uint64_t(1000000);
   static constexpr uint64_t seconds_per_halfhour  = 30 * 60;
   static constexpr uint64_t seconds_per_hour      = 60 * 60;
   static constexpr uint64_t useconds_per_year     = seconds_per_year*1000000ll;

   static constexpr uint64_t     system_token_symbol = CORE_SYMBOL;

   struct moveprod_param {
       account_name   producer;
       std::string    producerkey;
       std::string    blskey;
       bool           from_disable;
       name           from_chain;
       bool           to_disable;
       name           to_chain;

       moveprod_param() {}
       moveprod_param(account_name prod, const std::string& prod_key, const std::string& bls_key,
                      bool from_dis, name from, bool to_dis, name to) : producer(prod), producerkey(prod_key),
                      blskey(bls_key), from_disable(from_dis), from_chain(from), to_disable(to_dis), to_chain(to) {}

       ULTRAINLIB_SERIALIZE(moveprod_param, (producer)(producerkey)(blskey)(from_disable)(from_chain)(to_disable)(to_chain) )
   };

    struct evildoer {
        account_name account;
        std::string commitee_pk;
        ULTRAINLIB_SERIALIZE(evildoer, (account)(commitee_pk))
    };

   class system_contract : public native {
      private:
         global_state_singleton   _global;
         ultrainio_global_state   _gstate;

         lwc_singleton            _lwcsingleton;
         lwc_parameters           _lwc;

         chains_table             _chains;
         producer_brief_table     _briefproducers;
         sched_set_singleton      _schedsetting;

         bool accept_block_header(name chain_name, const ultrainio::signed_block_header& header, char* confirmed_bh_hash, size_t hash_size);

         int verify_evil(const std::string& evidence, const evildoer& evil);

      public:
         system_contract( account_name s );
         ~system_contract();


         // functions defined in delegate.cpp
         void delegatecons( account_name from, account_name receiver,asset stake_net_quantity);
         void undelegatecons( account_name from, account_name receiver);
         void refundcons( account_name owner );
         /**
          *  This action is called after the delegation-period to claim all pending
          *  unstaked tokens belonging to owner
          */
         void resourcelease( account_name from, account_name receiver,
                             uint64_t combosize, uint64_t days, name location = self_chain_name);
         void transferresource(account_name from, account_name to, uint64_t combosize, name location = self_chain_name);
         void recycleresource(const account_name owner);
         void setfreeacc( account_name account, uint64_t number);

         // functions defined in producer.cpp
         void regproducer( const account_name producer,
                           const std::string& producer_key,
                           const std::string& bls_key,
                           account_name rewards_account,
                           const std::string& url,
                           name location );

         void moveprod(account_name producer,
                       const std::string&  producerkey,
                       const std::string&  blskey,
                       bool from_disable,
                       name from_chain,
                       bool to_disable,
                       name to_chain);

         void verifyprodevil();

         void procevilprod( account_name producer, uint16_t evil_type );

         // functions defined in reward.cpp
         void onblock( block_timestamp timestamp, account_name producer );
         void onfinish();
         void calcmasterrewards();
         void claimrewards( account_name producer );
         void rewardproof( const std::string& proof_info );

         // functions defined in scheduler.cpp
         void regchaintype(uint64_t type_id,
                           uint16_t min_producer_num,
                           uint16_t max_producer_num,
                           uint16_t sched_step,
                           uint16_t consensus_period);
         void regsubchain(name chain_name, uint64_t chain_type, const std::string& genesis_producer_pubkey);
         void acceptmaster(const std::vector<ultrainio::signed_block_header>& headers);
         void acceptheader(name chain_name,
                           const std::vector<ultrainio::signed_block_header>& headers);
         void clearchain(name chain_name, bool users_only);
         void empoweruser(account_name user,
                          name chain_name,
                          const std::string& owner_pk,
                          const std::string& active_pk,
                          bool updateable);
         void reportsubchainhash(name subchain,
                                 uint64_t blocknum,
                                 checksum256 hash,
                                 uint64_t file_size);
         void setsched(bool is_enabled, uint16_t sched_period, uint16_t expire_time);
         void forcesetblock(name chain_name,
                            const signed_block_header& signed_header,
                            const std::vector<committee_info>& cmt_set);
         void setlwcparams(uint32_t keep_blocks_num);
         void setchainparam(name chain_name, uint64_t chain_type, bool is_sched_on);

         // functions defined in ultrainio.system.cpp
         void setsysparams( const ultrainio_system_params& params );
         void setglobalextendata( uint16_t key, std::string value );
         inline uint64_t getglobalextenuintdata( uint16_t key, uint64_t default_value ) const;
         void setmasterchaininfo( const master_chain_info& chaininfo );
         void setparams( const ultrainio::blockchain_parameters& params );
         void setpriv( account_name account, uint8_t is_priv );
         void setupdateabled( account_name account, uint8_t is_update );
         void rmvproducer( account_name producer );

         // functions defined in synctransaction.cpp
         void synclwctx( name chain_name,
                            uint32_t block_number,
                            std::vector<std::string> merkle_proofs,
                            std::vector<char> tx_bytes );

      private:
         // functions defined in ultrainio.system.cpp
         void add_subchain_account(const ultrainio::proposeaccount_info& newacc );
         static ultrainio_global_state get_default_parameters();

         //defind in delegate.cpp
         void change_cons( account_name from, account_name receiver, asset stake_cons_quantity);
         void process_undelegate_request(account_name from, asset unstake_quantity);
         void check_res_expire();
         void del_expire_table();
         void clear_expire_contract( account_name owner );
         //defined in reward.cpp
         void report_subchain_block( account_name producer, uint64_t block_height );
         inline void generate_reward_trx( account_name producer, account_name reward_account, uint64_t paid_balance ) const;
         void distribut_reward();
         inline float get_reward_fee_ratio() const;
         inline uint64_t get_reward_per_block() const;
         inline uint32_t check_previous_claimed_reward( const ultrainiosystem::producer_info& prod,uint32_t block_height ) const;
         inline void send_rewards_for_producer( account_name producer, account_name reward_account, const name& chain_name, uint64_t unpaid_balance );
         inline void record_rewards_for_disproducer( account_name producer, account_name reward_account, uint64_t unpaid_balance );
         inline uint64_t remove_rewards_for_enableproducer( account_name producer );
         inline void record_rewards_for_maintainer( account_name maintainer, uint64_t unpaid_balance );
         inline void send_rewards_for_maintainer( account_name maintainer );

         //defined in scheduler.cpp
         void add_to_chain(name chain_name, const producer_info& producer, uint64_t current_block_number);
         void remove_from_chain(name chain_name, account_name producer_name, uint64_t current_block_number);
         void pre_schedule(); //called in onblock every 24h defaultly.
         void check_bulletin();
         bool move_producer(checksum256 head_id,
                            chains_table::const_iterator from_iter,
                            chains_table::const_iterator to_iter,
                            uint64_t current_block_number, uint32_t num);
         name get_default_chain();
         bool check_block_proposer(account_name block_proposer, chains_table::const_iterator chain_iter);
         uint32_t find_previous_block(const std::vector<unconfirmed_block_header>& block_vct, uint32_t block_num,
                                    const block_id_type& block_id, const block_id_type& previous_id);
         void rm_overdue_blocks(name chain_name, uint32_t last_confirm_num, uint32_t confirm_num);
         uint64_t get_initial_block_num(name chain_name);
         void handle_new_confirm_block(chain_info& _chain, const block_id_type& confirm_block_id);
         void clear_committee_bulletin(name chain_name);

         //defined in ultrainio.system.cpp
         void get_key_data(const std::string& pubkey,std::array<char,33> & data);

         //defined in producer.cpp
         std::vector<name> get_all_chainname();
         inline void send_defer_moveprod_action( const moveprod_param& prodparam ) const;
         inline void check_producer_evillist( const account_name& producer ) const;
         inline void remove_producer_from_evillist( const account_name& producer ) const;
         inline void check_producer_lastblock( const name& chain_name, uint64_t block_height ) const;

         // functions defined in synctransaction.cpp
         void exec_actions( const vector<action> & actios, name chain_name);
   };

} /// ultrainiosystem

   #ifdef __cplusplus
   extern "C" {
   #endif

    void head_block_id(char* buffer, uint32_t buffer_size);
    void empower_to_chain(account_name user, ultrainio::name chain_name);
    bool is_empowered(account_name user, ultrainio::name chain_name);
    bool lightclient_accept_block_header(ultrainio::name chain_name, const char* bh, size_t bh_size, char* confirmed_bh_buffer, size_t buffer_size);
    int  native_verify_evil(const char* evidence, size_t evidence_size, const char* evil, size_t evil_size);
    void get_account_pubkey(account_name user, char* owner_pub, size_t owner_publen, char* active_pub, size_t active_publen );
   #ifdef __cplusplus
   }
   #endif
