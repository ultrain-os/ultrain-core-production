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
#include "BlockHeaderExtKey.h"
#include "CommitteeSet.h"
#define UNUSED(a) (void*)(a);
namespace ultrainiosystem {
   using namespace ultrainio;
   const name master_chain_name{N(ultrainio)};
   const name default_chain_name{N(default)};  //default chain, will be assigned by system.

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
      int64_t              perblock_bucket = 0;
      uint64_t             total_unpaid_balance = 0;
      int64_t              total_activated_stake = 0;
      block_timestamp      last_name_close;
      uint64_t             max_resources_number = 10000;
      uint64_t             total_resources_used_number = 0;
      uint32_t             newaccount_fee = 2000;
      name                 chain_name = name{N(ultrainio)};
      uint32_t             cur_committee_number = 0;
      uint64_t             worldstate_interval  = 1000;
      uint32_t             resource_fee = 100000;
      exten_types          table_extension;

      enum global_state_exten_type_key {
          global_state_key_start = 0,
          update_auth,
          confirm_point_interval
      };

      ULTRAINLIB_SERIALIZE_DERIVED(ultrainio_global_state, ultrainio::blockchain_parameters,
                                   (max_ram_size)(min_activated_stake)(min_committee_member_number)
                                   (total_ram_bytes_used)(start_block)(block_reward_vec)
                                   (total_cur_chain_block)(perblock_bucket)(total_unpaid_balance)
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
       uint64_t             max_ram_size = 12ll*1024 * 1024 * 1024;
       uint64_t             total_ram_bytes_used = 0;

       ULTRAINLIB_SERIALIZE(chain_resource, (max_resources_number)(total_resources_used_number)
                            (max_ram_size)(total_ram_bytes_used) )
   };

   struct producer_brief {
      account_name          owner;
      name                  location{N(ultrainio)};
      bool                  in_disable = true;
      uint64_t primary_key()const { return owner; }

      bool     is_on_master_chain() const  {return location == master_chain_name;}
      bool     is_on_subchain() const      {
          return (location != master_chain_name) && (location != default_chain_name);
      }

      ULTRAINLIB_SERIALIZE(producer_brief, (owner)(location)(in_disable) )
   };

   using role_base = CommitteeInfo;

   struct disabled_producer : public CommitteeInfo {
      int64_t               total_cons_staked = 0;
      std::string           url;
      uint64_t              total_produce_block = 0;
      uint64_t              last_operate_blocknum = 0;
      uint64_t              delegated_cons_blocknum = 0;
      account_name          claim_rewards_account;

      uint64_t primary_key()const { return owner; }

      ULTRAINLIB_SERIALIZE_DERIVED( disabled_producer, CommitteeInfo, (total_cons_staked)
                                    (url)(total_produce_block)(last_operate_blocknum)
                                    (delegated_cons_blocknum)(claim_rewards_account) )
   };

   struct producer_info : public disabled_producer {
      uint64_t              unpaid_balance = 0;
      uint64_t              vote_number = 0;
      uint64_t              last_vote_blocknum = 0;
      exten_types           table_extension;

      uint64_t primary_key()const { return owner; }

      ULTRAINLIB_SERIALIZE_DERIVED( producer_info, disabled_producer,
                                    (unpaid_balance)(vote_number)(last_vote_blocknum)(table_extension) )
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

   static constexpr uint32_t MAX_WS_COUNT                = 5;

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
       std::vector<role_base>               master_prods;
       uint64_t                             block_height = 0;
       block_id_type                        block_id;
       exten_types                          master_chain_ext;
       uint64_t  primary_key()const { return owner; }
       ULTRAINLIB_SERIALIZE(master_chain_info, (owner)(master_prods)(block_height)(block_id)(master_chain_ext) )
   };

   typedef ultrainio::multi_index<N(briefprod),producer_brief> producer_brief_table;
   typedef ultrainio::multi_index<N(pendingminer),pending_miner> pendingminers;
   typedef ultrainio::multi_index<N(pendingacc),pending_acc> pendingaccounts;
   typedef ultrainio::multi_index<N(pendingres),pending_res> pendingresource;
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
   };

   struct changing_producer : public CommitteeInfo {
      uint32_t          block_num;  //master block number when it happens

      ULTRAINLIB_SERIALIZE_DERIVED( changing_producer, CommitteeInfo, (block_num))
   };

   struct changing_committee {
       std::vector<changing_producer> removed_members;
       std::vector<role_base> new_added_members;

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

   struct unconfirmed_block_header : public ultrainio::block_header {
       block_id_type              block_id;
       uint32_t                   block_number = 0;
       bool                       to_be_paid;    //should block proposer be paid when this block was confirmed
       bool                       is_leaf = true;       //leaf in the fork tree
       bool                       is_synced;
       std::string                signature;
       std::vector<role_base>     committee_set;
       std::vector<checksum256>   trx_hashs;
       std::string                next_committee_mroot;
       ultrainio::extensions_type           table_extension;

       unconfirmed_block_header() {}
       unconfirmed_block_header(const ultrainio::block_header& header, const block_id_type& b_id, uint32_t b_n,
                                bool need_pay, bool is_sync, const std::string& sign) : ultrainio::block_header(header),
                                block_id(b_id), block_number(b_n), to_be_paid(need_pay), is_leaf(true), is_synced(is_sync) {
           if (header.proposer == N(genesis)) {
               signature = sign;
           }
           for (const auto& e : header_extensions) {
               BlockHeaderExtKey key = static_cast<BlockHeaderExtKey>(std::get<0>(e));
               if (key == kCommitteeSet) {
                   const std::vector<char>& vc = std::get<1>(e);
                   CommitteeSet cmt_set(vc);
                   cmt_set.swap(this->committee_set);
                   break;
               }
           }
       }

       ULTRAINLIB_SERIALIZE_DERIVED(unconfirmed_block_header, ultrainio::block_header,(block_id)(block_number)
                                    (to_be_paid)(is_leaf)(is_synced)(signature)(committee_set)(trx_hashs)(next_committee_mroot)(table_extension))
   };

   struct chain_info {
       name                      chain_name;
       uint64_t                  chain_type;
       block_timestamp           genesis_time;
       chain_resource            global_resource;
       bool                      is_synced;
       bool                      is_schedulable;
       bool                      schedule_on = true;
       uint16_t                  committee_num;
       std::vector<role_base>    deprecated_committee;//keep history producers for un-synced chain, clear it once synced
       changing_committee        changing_info; //has changed but not be confirmed by subchain's block header.
       std::vector<user_info>    recent_users;
       uint32_t                  total_user_num;
       checksum256               chain_id;
       checksum256               committee_mroot;
       uint32_t                  confirmed_block_number;
       std::vector<role_base>    committee_set;//current committee set reported by chain
       std::vector<unconfirmed_block_header>  unconfirmed_blocks;
       exten_types                table_extension;

       auto primary_key()const { return chain_name; }

       ULTRAINLIB_SERIALIZE(chain_info, (chain_name)(chain_type)(genesis_time)(global_resource)(is_synced)
                            (is_schedulable)(schedule_on)(committee_num)(deprecated_committee)(changing_info)
                            (recent_users)(total_user_num)(chain_id)(committee_mroot)(confirmed_block_number)
                            (committee_set)(unconfirmed_blocks)(table_extension) )

       void handle_committee_update(CommitteeDelta& cmt_delta) {
           for(auto it_rm = changing_info.removed_members.begin(); it_rm != changing_info.removed_members.end();) {
               if(cmt_delta.checkRemoved(*it_rm)) {
                   it_rm = changing_info.removed_members.erase(it_rm);
               } else {
                    ++it_rm;
               }
           }
           for(auto it_add = changing_info.new_added_members.begin(); it_add != changing_info.new_added_members.end();) {
               if(cmt_delta.checkAdded(*it_add)) {
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

   struct schedule_setting {
       bool          is_schedule_enabled;
       uint16_t      schedule_period;
       uint16_t      expire_minutes;
   };
   typedef ultrainio::singleton<N(schedset), schedule_setting> sched_set_singleton;
   //   static constexpr uint32_t     max_inflation_rate = 5;  // 5% annual inflation

   static constexpr uint32_t seconds_per_day       = 24 * 3600;
   static constexpr uint32_t seconds_per_year      = 52*7*24*3600;
   static constexpr uint64_t useconds_per_day      = 24 * 3600 * uint64_t(1000000);
   static constexpr uint64_t seconds_per_halfhour  = 30 * 60;
   static constexpr uint64_t useconds_per_year     = seconds_per_year*1000000ll;

   static constexpr uint64_t     system_token_symbol = CORE_SYMBOL;

   class system_contract : public native {
      private:
         global_state_singleton   _global;
         ultrainio_global_state   _gstate;

         lwc_singleton            _lwcsingleton;
         lwc_parameters           _lwc;

         chains_table             _chains;
         pendingminers            _pendingminer;
         pendingaccounts          _pendingaccount;
         pendingresource          _pendingres;
         producer_brief_table     _briefproducers;
         sched_set_singleton      _schedsetting;

         bool accept_block_header(name chain_name, const ultrainio::block_header& header, char* confirmed_bh_hash, size_t hash_size);

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
                             uint64_t combosize, uint64_t days, name location = master_chain_name);
         void recycleresource(const account_name owner);


         // functions defined in producer.cpp
         void regproducer( const account_name producer,
                           const std::string& producer_key,
                           const std::string& bls_key,
                           account_name rewards_account,
                           const std::string& url,
                           name location );
         void unregprod( const account_name producer );


         // functions defined in reward.cpp
         void onblock( block_timestamp timestamp, account_name producer );
         void claimrewards();


         // functions defined in scheduler.cpp
         void regchaintype(uint64_t type_id,
                           uint16_t min_producer_num,
                           uint16_t max_producer_num,
                           uint16_t sched_step,
                           uint16_t consensus_period);
         void regsubchain(name chain_name, uint64_t chain_type);
         void acceptmaster(const std::vector<ultrainio::block_header>& headers);
         void acceptheader(name chain_name,
                           const std::vector<ultrainio::block_header>& headers,
                           const std::string& signature);
         void clearchain(name chain_name, bool users_only);
         void empoweruser(account_name user,
                          name chain_name,
                          const std::string& owner_pk,
                          const std::string& active_pk);
         void reportsubchainhash(name subchain,
                                 uint64_t blocknum,
                                 checksum256 hash,
                                 uint64_t file_size);
         void setsched(bool is_enabled, uint16_t sched_period, uint16_t expire_time);
         void forcesetblock(name chain_name,
                            const block_header& header,
                            const std::vector<role_base>& cmt_set);
         void schedule(const std::string& trigger);
         void setlwcparams(uint32_t keep_blocks_num);
         void setchainparam(name chain_name, bool is_sched_on);

         // functions defined in ultrainio.system.cpp
         void setsysparams( const ultrainio_system_params& params );
         void setmasterchaininfo( const master_chain_info& chaininfo );
         void setparams( const ultrainio::blockchain_parameters& params );
         void setpriv( account_name account, uint8_t is_priv );
         void setupdateabled( account_name account, uint8_t is_update );
         void rmvproducer( account_name producer );
         void votecommittee();
         void voteaccount();
         void voteresourcelease();


         // functions defined in synctransaction.cpp
         void synclwctx( name chain_name,
                            uint32_t block_number,
                            std::vector<std::string> merkle_proofs,
                            std::vector<char> tx_bytes );

      private:
         // functions defined in ultrainio.system.cpp
         void updateactiveminers(const ultrainio::proposeminer_info& miners );
         void add_subchain_account(const ultrainio::proposeaccount_info& newacc );
         static ultrainio_global_state get_default_parameters();

         //defind in delegate.cpp
         void change_cons( account_name from, account_name receiver, asset stake_cons_quantity);
         void process_undelegate_request(account_name from, asset unstake_quantity);
         void checkresexpire();


         //defined in reward.cpp
         void reportblocknumber( name chain_name,
                                 uint64_t chain_type,
                                 account_name producer,
                                 uint64_t number);

         void distributreward();


         //defined in scheduler.cpp
         void add_to_chain(name chain_name, const producer_info& producer, uint64_t current_block_number);
         void remove_from_chain(name chain_name, account_name producer_name, uint64_t current_block_number);
         void pre_schedule(); //called in onblock every 24h defaultly.
         void checkbulletin();
         bool move_producer(checksum256 head_id,
                            chains_table::const_iterator from_iter,
                            chains_table::const_iterator to_iter,
                            uint64_t current_block_number, uint32_t num);
         name getdefaultchain();
         bool checkblockproposer(account_name block_proposer, chains_table::const_iterator chain_iter);


         //defined in ultrainio.system.cpp
         void getKeydata(const std::string& pubkey,std::array<char,33> & data);
         void cleanvotetable();
         void syncresource(account_name receiver, uint64_t combosize, uint32_t block_height);
         void checkvotefrequency(ultrainiosystem::producers_table& prod_tbl,
                                 ultrainiosystem::producers_table::const_iterator propos);


         //defined in producer.cpp
         std::vector<name> get_all_chainname();

         // functions defined in synctransaction.cpp
         void execactions( const vector<action> & actios);
   };

} /// ultrainiosystem

   #ifdef __cplusplus
   extern "C" {
   #endif

    void head_block_id(char* buffer, uint32_t buffer_size);
    void empower_to_chain(account_name user, ultrainio::name chain_name);
    bool is_empowered(account_name user, ultrainio::name chain_name);
    bool lightclient_accept_block_header(ultrainio::name chain_name, const char* bh, size_t bh_size, char* confirmed_bh_buffer, size_t buffer_size);

   #ifdef __cplusplus
   }
   #endif
