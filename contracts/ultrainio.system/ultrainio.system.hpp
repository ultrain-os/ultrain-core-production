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

namespace ultrainiosystem {
   using namespace ultrainio;

   const uint64_t master_chain_name = 0;
//   const uint64_t pending_queue = std::numeric_limits<uint64_t>::max();
   const uint64_t default_chain_name = N(default);  //default chain, will be assigned by system.

   bool operator!=(const checksum256& sha256_1, const checksum256& sha256_2) {
      for(auto i = 0; i < 32; ++i) {
         if(sha256_1.hash[i] != sha256_2.hash[i]) {
               return true;
         }
      }
      return false;
   }
   struct ultrainio_global_state : ultrainio::blockchain_parameters {
      uint64_t free_ram()const { return max_ram_size - total_ram_bytes_used; }
      bool is_master_chain()const { return chain_name == 0; }
      uint64_t             max_ram_size = 12ll*1024 * 1024 * 1024;
      int64_t              min_activated_stake   = 42'000'0000;
      uint32_t             min_committee_member_number = 1000;
      uint64_t             total_ram_bytes_used = 0;

      uint64_t             start_block = 0;
      std::vector<ultrainio::block_reward> block_reward_vec;
      int64_t              pervote_bucket = 0;
      int64_t              perblock_bucket = 0;
      uint64_t             total_unpaid_balance = 0;
      int64_t              total_activated_stake = 0;
      block_timestamp      last_name_close;
      uint16_t             max_resources_number = 10000;
      uint16_t             total_resources_used_number = 0;
      uint32_t             newaccount_fee = 2000;
      uint64_t             chain_name = 0;
      uint32_t             cur_committee_number = 0;
      ultrainio::extensions_type           table_extension;

      ULTRAINLIB_SERIALIZE_DERIVED(ultrainio_global_state, ultrainio::blockchain_parameters,
                                   (max_ram_size)(min_activated_stake)(min_committee_member_number)
                                   (total_ram_bytes_used)(start_block)(block_reward_vec)
                                   (pervote_bucket)(perblock_bucket)(total_unpaid_balance)
                                   (total_activated_stake)(last_name_close)(max_resources_number)
                                   (total_resources_used_number)(newaccount_fee)(chain_name)(cur_committee_number)(table_extension) )
   };

   struct chain_resource {
       uint16_t             max_resources_number = 10000;
       uint16_t             total_resources_used_number = 0;
       uint64_t             max_ram_size = 12ll*1024 * 1024 * 1024;
       uint64_t             total_ram_bytes_used = 0;

       ULTRAINLIB_SERIALIZE(chain_resource, (max_resources_number)(total_resources_used_number)
                            (max_ram_size)(total_ram_bytes_used) )
   };

   struct producer_brief {
      account_name          owner;
      uint64_t              location;
      bool                  in_disable = true;
      uint64_t primary_key()const { return owner; }

      ULTRAINLIB_SERIALIZE(producer_brief, (owner)(location)(in_disable) )
   };

   struct role_base {
      account_name          owner;
      std::string           producer_key; /// a packed public key objec
      std::string           bls_key;

      ULTRAINLIB_SERIALIZE(role_base, (owner)(producer_key)(bls_key) )
   };

   struct disabled_producer : public role_base{
      int64_t               total_cons_staked = 0;
      std::string           url;
      uint64_t              total_produce_block = 0;
      uint64_t              location = 0;
      uint64_t              last_operate_blocknum = 0;
      uint64_t              delegated_cons_blocknum = 0;
      account_name          claim_rewards_account;

      uint64_t primary_key()const { return owner; }
      bool     is_on_master_chain() const  {return location == master_chain_name;}
      bool     is_on_subchain() const      {
          return (location != master_chain_name) && (location != default_chain_name);
      }

      ULTRAINLIB_SERIALIZE_DERIVED( disabled_producer, role_base, (total_cons_staked)
                                    (url)(total_produce_block)(location)(last_operate_blocknum)
                                    (delegated_cons_blocknum)(claim_rewards_account) )
   };

   struct producer_info : public disabled_producer {
      uint64_t              unpaid_balance = 0;
      uint64_t              vote_number = 0;
      uint64_t              last_vote_blocknum = 0;
      ultrainio::extensions_type           table_extension;

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
       hash_vote(checksum256 hash_p, uint64_t size, uint64_t vote, bool val)
           :hash(hash_p),file_size(size),votes(vote),valid(val){}
       hash_vote(){}
       checksum256      hash;
       uint64_t         file_size;
       uint64_t         votes;
       bool             valid;
       ULTRAINLIB_SERIALIZE(hash_vote , (hash)(file_size)(votes)(valid) )
   };

   static constexpr uint32_t default_worldstate_interval = 60;
   static constexpr uint32_t MAX_WS_COUNT                = 5;

   struct subchain_ws_hash {
       uint64_t             block_num;
       std::vector<hash_vote>    hash_v;
       std::vector<account_name> accounts;
       ultrainio::extensions_type           table_extension;
       uint64_t  primary_key()const { return block_num; }
       ULTRAINLIB_SERIALIZE( subchain_ws_hash , (block_num)(hash_v)(accounts)(table_extension) )
   };
   typedef ultrainio::multi_index<N(briefprod),producer_brief> producer_brief_table;
   typedef ultrainio::multi_index<N(pendingminer),pending_miner> pendingminers;
   typedef ultrainio::multi_index<N(pendingacc),pending_acc> pendingaccounts;
   typedef ultrainio::multi_index<N(pendingres),pending_res> pendingresource;
   typedef ultrainio::multi_index<N(disableprods), disabled_producer> disabled_producers_table;
   typedef ultrainio::multi_index<N(producers), producer_info> producers_table;

   typedef ultrainio::singleton<N(global), ultrainio_global_state> global_state_singleton;
   typedef ultrainio::multi_index< N(wshash), subchain_ws_hash>      subchain_hash_table;

   struct chaintype {
       uint64_t type_id;
       uint16_t stable_min_producers;
       uint16_t stable_max_producers;
       uint16_t sched_inc_step;
       uint16_t consensus_period;
       ultrainio::extensions_type           table_extension;

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
       uint32_t                  block_number = 0;
       checksum256               transaction_mroot;
       std::vector<checksum256>  trx_hashs;
       ultrainio::extensions_type           table_extension;

       auto primary_key() const { return uint64_t(block_number); }

       ULTRAINLIB_SERIALIZE(block_header_digest, (proposer)(block_id)(block_number)
                            (transaction_mroot)(trx_hashs)(table_extension))
   };

   typedef ultrainio::multi_index<N(blockheaders), block_header_digest> block_table;

   struct unconfirmed_block_header : public block_header_digest {
       uint16_t                  fork_id;
       bool                      to_be_paid;    //should block proposer be paid when this block was confirmed
       bool                      is_leaf;       //leaf in the fork tree
       checksum256               committee_mroot;

       ULTRAINLIB_SERIALIZE_DERIVED(unconfirmed_block_header, block_header_digest,
                                    (fork_id)(to_be_paid)(is_leaf)(committee_mroot))
   };

   struct subchain {
       uint64_t                  chain_name;
       uint64_t                  chain_type;
       block_timestamp           genesis_time;
       chain_resource            global_resource;
       bool                      is_active;
       bool                      is_synced;
       bool                      is_schedulable;
       uint16_t                  committee_sum;
       std::vector<role_base>    committee_members;
       updated_committee         updated_info;
       changing_committee        changing_info;
       std::vector<user_info>    recent_users;
       uint32_t                  total_user_num;
       checksum256               chain_id;
       checksum256               committee_mroot;
       uint32_t                  confirmed_block_number;
       uint32_t                  highest_block_number;
       //actually the last confirmed block is also stored in this vector
       std::vector<unconfirmed_block_header>  unconfirmed_blocks;
       ultrainio::extensions_type           table_extension;

       auto primary_key()const { return chain_name; }

       ULTRAINLIB_SERIALIZE(subchain, (chain_name)(chain_type)(genesis_time)(global_resource)
                            (is_active)(is_synced)(is_schedulable)(committee_sum)(committee_members)
                            (updated_info)(changing_info)(recent_users)(total_user_num)(chain_id)
                            (committee_mroot)(confirmed_block_number)(highest_block_number)
                            (unconfirmed_blocks)(table_extension) )
   };
   typedef ultrainio::multi_index<N(subchains), subchain> subchains_table;

   struct ultrainio_system_params {
      ultrainio_system_params(){}
      uint64_t             chain_type;
      uint64_t             max_ram_size;
      int64_t              min_activated_stake;
      uint32_t             min_committee_member_number;
      std::vector<block_reward> block_reward_vec;
      uint16_t             max_resources_number;
      uint32_t             newaccount_fee;
      uint64_t             chain_name;
      std::vector<role_base> master_producers;
      uint64_t  primary_key()const { return chain_type; }
      ULTRAINLIB_SERIALIZE( ultrainio_system_params,(chain_type)(max_ram_size)(min_activated_stake)
                            (min_committee_member_number)(block_reward_vec)(max_resources_number)(newaccount_fee)(chain_name)(master_producers) )
   };

   struct resources_lease {
      account_name   owner;
      uint64_t       lease_num = 0;
      uint32_t       start_block_height = 0;
      uint32_t       end_block_height = 0;
      uint32_t       modify_block_height = 0;

      uint64_t  primary_key()const { return owner; }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE( resources_lease, (owner)(lease_num)(start_block_height)
                            (end_block_height)(modify_block_height) )
   };
   typedef ultrainio::multi_index< N(reslease), resources_lease>      resources_lease_table;

   struct schedule_setting {
       bool          is_schedule_enabled;
       uint16_t      schedule_period;
       uint16_t      committee_confirm_period;
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
         global_state_singleton _global;

         ultrainio_global_state   _gstate;
         subchains_table          _subchains;
         pendingminers            _pendingminer;
         pendingaccounts          _pendingaccount;
         pendingresource          _pendingres;
         producer_brief_table     _briefproducers;
         sched_set_singleton      _schedsetting;

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
                             uint64_t combosize, uint64_t days, uint64_t location = master_chain_name);
         void recycleresource(const account_name owner ,uint64_t lease_num);


         // functions defined in producer.cpp
         void regproducer( const account_name producer,
                           const std::string& producer_key,
                           const std::string& bls_key,
                           account_name rewards_account,
                           const std::string& url,
                           uint64_t location );
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
         void regsubchain(uint64_t chain_name, uint64_t chain_type);
         void acceptheader(uint64_t chain_name,
                           const std::vector<ultrainio::block_header>& headers);
//                          const std::string& aggregatedEcho,
//                          const std::vector<uint32_t>& echo_weight_vector,
//                          const std::vector<std::string>& echo_account_vector);
         void clearchain(uint64_t chain_name, bool users_only);
         void empoweruser(account_name user,
                          uint64_t chain_name,
                          const std::string& owner_pk,
                          const std::string& active_pk);
         void reportsubchainhash(uint64_t subchain,
                                 uint64_t blocknum,
                                 checksum256 hash,
                                 uint64_t file_size);

         //Register to ba a relayer candidate of a subchain,
         //only for those accounts which are not in the committee list of this subchain.
         //All committee members are also be relayer candidates automatically
/*         void register_relayer(const std::string& miner_pk,
                               account_name relayer_account_name,
                               uint32_t relayer_deposit,
                               const std::string& ip,
                               uint64_t chain_name); */

         void setsched(bool is_enabled, uint16_t sched_period, uint16_t confirm_period);
         void forcesetblock(uint64_t chain_name,
                            block_header_digest header_dig,
                            checksum256 committee_mrt);

         // functions defined in ultrainio.system.cpp
         void setsysparams( const ultrainio_system_params& params );
         void setparams( const ultrainio::blockchain_parameters& params );
         void setpriv( account_name account, uint8_t ispriv );
         void rmvproducer( account_name producer );
         void votecommittee();
         void voteaccount();
         void voteresourcelease();


         // functions defined in synctransaction.cpp
         void synctransfer( uint64_t chain_name,
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
         void reportblocknumber( uint64_t chain_name,
                                 uint64_t chain_type,
                                 account_name producer,
                                 uint64_t number);
         void claim_reward_to_account(account_name rewardaccount, asset balance);
         void distributreward();


         //defined in scheduler.cpp
         void add_to_chain(uint64_t chain_name, const producer_info& producer);
         void remove_from_chain(uint64_t chain_name, account_name producer_name);
         //called in onblock, loop for all subchains and activate their committee update
         void activate_committee_update();
         void schedule(); //called in onblock every 24h defaultly.
         void checkbulletin();
         bool move_producer(checksum256 head_id,
                            subchains_table::const_iterator from_iter,
                            subchains_table::const_iterator to_iter,
                            uint16_t index);
         uint64_t getdefaultchain();


         //defined in ultrainio.system.cpp
         void getKeydata(const std::string& pubkey,std::array<char,33> & data);
         void cleanvotetable();
         void syncresource(account_name receiver, uint64_t combosize, uint32_t block_height);
         void checkvotefrequency(ultrainiosystem::producers_table& prod_tbl,
                                 ultrainiosystem::producers_table::const_iterator propos);


         //defined in producer.cpp
         std::vector<uint64_t> get_all_chainname();
   };

} /// ultrainiosystem

   #ifdef __cplusplus
   extern "C" {
   #endif

    void head_block_id(char* buffer, uint32_t buffer_size);
    void empower_to_chain(account_name user, uint64_t chain_name);
    bool is_empowered(account_name user, uint64_t chain_name);
    bool lightclient_accept_block_header(uint64_t chain_name, const char* bh, int bh_len, char* confirmed_bh_buffer, int buffer_len);

   #ifdef __cplusplus
   }
   #endif
