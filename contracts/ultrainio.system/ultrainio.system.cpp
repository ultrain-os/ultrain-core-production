#include "ultrainio.system.hpp"

#include "reward.cpp"
#include "delegate.cpp"
#include "producer.cpp"
#include "scheduler.cpp"
#include "synctransaction.cpp"


namespace ultrainiosystem {

   system_contract::system_contract( account_name s )
   :native(s),
    _global(_self,_self),
    _lwcsingleton(_self, _self),
    _chains(_self,_self),
    _briefproducers(_self,_self),
    _schedsetting(_self, _self) {
      _gstate = _global.exists() ? _global.get() : get_default_parameters();
      if(_lwcsingleton.exists()) {
          _lwc = _lwcsingleton.get();
      } else {
          _lwc.save_blocks_num = 30000;
      }
   }

   ultrainio_global_state system_contract::get_default_parameters() {
      ultrainio_global_state dp;
      get_blockchain_parameters(dp);
      return dp;
   }

   system_contract::~system_contract() {
      _global.set( _gstate );
      _lwcsingleton.set(_lwc);
   }

   void system_contract::setsysparams( const ultrainio_system_params& params) {
        require_auth( _self );
        if (params.chain_type == 0) { //master chain
            _gstate.max_ram_size = params.max_ram_size;
            ultrainio_assert( params.max_ram_size > _gstate.total_ram_bytes_used,
                              "attempt to set max below reserved" );
            _gstate.min_activated_stake = params.min_activated_stake;
            if ( _gstate.cur_committee_number > _gstate.min_committee_member_number ) { //genesis bios already ended
                ultrainio_assert( _gstate.cur_committee_number > params.min_committee_member_number,
                                "min_committee_member_number must be less than cur_committee_number" );
            }
            _gstate.min_committee_member_number = params.min_committee_member_number;
            if(params.block_reward_vec.size() > 0){
                _gstate.block_reward_vec.clear();
                _gstate.block_reward_vec.assign(params.block_reward_vec.begin(), params.block_reward_vec.end());;
            }
            _gstate.newaccount_fee = params.newaccount_fee;
            _gstate.chain_name = params.chain_name;
            _gstate.max_resources_number = params.max_resources_number;
            _gstate.worldstate_interval = params.worldstate_interval;
            _gstate.resource_fee = params.resource_fee;
            _gstate.table_extension.assign(params.table_extension.begin(),params.table_extension.end());
        } else {
            auto ite_chain = _chains.find(params.chain_name);
            ultrainio_assert(ite_chain != _chains.end(), "this chian does not exist");
            _chains.modify(ite_chain, [&](chain_info& info) {
                info.global_resource.max_ram_size = params.max_ram_size;
                info.global_resource.max_resources_number = params.max_resources_number;
            } );
        }
   }
   void system_contract::setglobalextendata( uint16_t key, std::string value ) {
      require_auth( _self );
      ultrainio_assert( !value.empty(), " global value is empty" );
      ultrainio_assert( key < ultrainio_global_state::global_state_exten_type_key::global_state_key_end, " key should not exist" );
      bool is_exist_key = false;
      for( auto& exten : _gstate.table_extension ){
         if(exten.key == key){
            exten.value = value;
            is_exist_key = true;
            break;
         }
      }
      if( !is_exist_key ){
         _gstate.table_extension.push_back( exten_type( key, value) );
      }
   }

   uint64_t system_contract::getglobalextenuintdata( uint16_t key, uint64_t default_value ) const {
      for( auto& exten : _gstate.table_extension ){
         if( exten.key == key && !exten.value.empty() ){
            return std::stoull( exten.value );
         }
      }
      return default_value;
   }

   void system_contract::setchaintypeextendata( uint64_t type_id, uint16_t key, std::string value ) {
      require_auth( _self );
      ultrainio_assert( !value.empty(), "setchaintypeextendata value is empty" );
      ultrainio_assert( key < chaintype::chaintype_exten_key::chaintype_key_end, " key should not exist" );
      bool is_exist_key = false;
      chaintypes_table type_tbl( _self, _self );
      auto type_iter = type_tbl.find( type_id );
      ultrainio_assert( type_iter != type_tbl.end(), "setchaintypeextendata type_id not found" );
      type_tbl.modify( type_iter, [&]( auto& _chain_type ) {
         for( auto& exten : _chain_type.table_extension ) {
            if(exten.key == key) {
               exten.value = value;
               is_exist_key = true;
               break;
            }
         }
         if( !is_exist_key ) {
            _chain_type.table_extension.push_back( exten_type( key, value) );
         }
      });
   }

   uint64_t system_contract::getchaintypeextenuintdata( const ultrainiosystem::chaintypes_table::const_iterator& type_iter, uint16_t key, uint64_t default_value ) const {

      for( auto& exten : type_iter->table_extension ){
         if( exten.key == key && !exten.value.empty() ){
            return std::stoull( exten.value );
         }
      }
      return default_value;
   }

   void system_contract::setmasterchaininfo( const master_chain_info& chaininfo ){
      require_auth( _self );
      master_chain_infos masterinfos(_self, _self);
      auto it_masterchain = masterinfos.find(chaininfo.owner);
      ultrainio_assert(it_masterchain == masterinfos.end(), "masterinfos already exist");
      masterinfos.emplace( [&]( master_chain_info& info ) {
         info = chaininfo;
         info.owner = chaininfo.owner;
      });

      auto it = _chains.find(N(master));
      ultrainio_assert(it == _chains.end(), "master chain already exist");
      _chains.emplace( [&]( auto& masterchain ) {
          masterchain.chain_name         = name{N(master)};
          masterchain.chain_type         = 0; //0 is for master chain, not created in chaintypes table
          masterchain.is_synced          = false;
          masterchain.is_schedulable     = false;
          masterchain.committee_num      = uint16_t(chaininfo.master_prods.size());
          masterchain.total_user_num     = 0;
          masterchain.chain_id           = checksum256();
          masterchain.committee_mroot    = chaininfo.committee_mroot;
          masterchain.confirmed_block_number = uint32_t(chaininfo.block_height);
          masterchain.confirmed_block_id = chaininfo.block_id;
          masterchain.committee_set      = chaininfo.master_prods;
          unconfirmed_block_header uncfm_block;
          uncfm_block.committee_mroot = chaininfo.committee_mroot;
          uncfm_block.block_id = chaininfo.block_id;
          uncfm_block.block_number = uint32_t(chaininfo.block_height);
          uncfm_block.to_be_paid = false;
          uncfm_block.is_leaf = true;
          uncfm_block.is_synced = false;
          masterchain.unconfirmed_blocks.push_back(uncfm_block);
      });
   }
   void system_contract::setparams( const ultrainio::blockchain_parameters& params ) {
      require_auth( N(ultrainio) );
      (ultrainio::blockchain_parameters&)(_gstate) = params;
      ultrainio_assert( 3 <= _gstate.max_authority_depth, "max_authority_depth should be at least 3" );
      set_blockchain_parameters( params );
   }

   void system_contract::setpriv( account_name account, uint8_t is_priv ) {
      require_auth( _self );
      set_privileged( account, is_priv );
   }

   void system_contract::setupdateabled( account_name account, uint8_t is_update ) {
      if( !has_auth(_self) ){
         require_auth( account );
         ultrainio_assert( !is_update, "the contract account can only be set to unupdatable" );
         INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {account,N(active)},
            { account, N(utrio.fee), asset(1000), std::string("setupdateabled") } );  //0.1 UGAS service charge
      }
      set_updateabled( account, is_update );
   }

   void system_contract::setprodontimeblock( account_name account, uint32_t online_second, uint32_t blocknum ) {
      require_auth( _self );
      auto briefprod = _briefproducers.find(account);
      ultrainio_assert(briefprod != _briefproducers.end(), "this account is not a producer");
      print("setprodontimeblock producer:",name{account}," online_second:",std::to_string(online_second).c_str()," blocknum:",std::to_string(blocknum).c_str()," now():",std::to_string(now()).c_str());

      ultrainio_assert(!briefprod->in_disable, "producer is disabled");
      producers_table _producers(_self, briefprod->location);
      const auto& it = _producers.find( account );
      ultrainio_assert(it != _producers.end(), "receiver is not found in its location");
      _producers.modify(it, [&](auto & v) {
         if(blocknum != 0){
            v.total_produce_block += (uint64_t)blocknum;
         }
         if(online_second != 0){
            ultrainio_assert(now()>(uint32_t)online_second, "set online second is wrong");
            uint32_t cur_online_time = now()- (uint32_t)online_second;
            bool is_exist_start_produce_time = false;
            for( auto& exten : v.table_extension ){
               if( exten.key == producer_info::producers_state_exten_type_key::start_produce_time ) {
                  exten.value = std::to_string(cur_online_time);
                  is_exist_start_produce_time = true;
                  break;
               }
            }
            if( !is_exist_start_produce_time )
               v.table_extension.push_back(exten_type(producer_info::producers_state_exten_type_key::start_produce_time ,std::to_string(cur_online_time)));
         }
      });
   }

   void system_contract::updatecommercdel( name chain_name, asset delegated_value ) {
      require_auth( _self );
      update_commercial_delegatesd( chain_name, delegated_value );
   }
   void system_contract::update_commercial_delegatesd( name chain_name, asset delegated_value ) {
      auto ite_chain = _chains.find(chain_name);
      ultrainio_assert(ite_chain != _chains.end(), "this chian does not exist");
      chain_comm_del_table chaincommdeltab( _self, _self );
      auto ite_com_table = chaincommdeltab.find( chain_name );
      if( ite_com_table == chaincommdeltab.end() ) {
        chaincommdeltab.emplace( [&]( chain_commercial_delegate& d ) {
            d.chain_name = chain_name;
            d.delegate_value = delegated_value.amount;
        });
      } else {
        chaincommdeltab.modify( ite_com_table, [&]( chain_commercial_delegate& d ) {
            d.delegate_value += delegated_value.amount;
            //In order to prevent not in time to commercial delegated, resulting in the reward can not be claimed, so cancel assert
            //ultrainio_assert( d.delegate_value >= 0, " total_delegated_value shouldn't be negative" );
        });
      }
   }
   void system_contract::get_key_data(const std::string& pubkey,std::array<char,33> & data){
      auto const get_hex_value = [](const char ch)->int{
         if(ch >= '0' && ch <= '9')
            return (ch-'0');
         if(ch >= 'a' && ch <= 'f')
            return ((ch-'a')+10);
         return 0;
      };
      char  key_data[67];
      memset(key_data,0,sizeof(key_data));
      frombase58_recover_key(pubkey.c_str(), key_data, 66);
      unsigned int j = 0;
      for ( uint32_t i=0; i < strlen(key_data); i++ ){
         if(i%2 == 1)
         {
            data[j] = static_cast<char>((get_hex_value(key_data[i-1])*16 + get_hex_value(key_data[i])) & 0xff);
            j++;
         }
      }
   }
   void system_contract::add_subchain_account(const ultrainio::proposeaccount_info&  newacc ) {
      ultrainiosystem::key_weight ownerkeyweight;
      std::array<char,33> ownerdata;
      get_key_data(newacc.owner_key,ownerdata);
      ownerkeyweight.key.data = ownerdata;
      ownerkeyweight.key.type = 0;
      ownerkeyweight.weight = 1;
      ultrainiosystem::authority    ownerkey  = { .threshold = 1, .keys = { ownerkeyweight }, .accounts = {}, .waits = {} };

      ultrainiosystem::key_weight activekeyweight;
      std::array<char,33> activedata;
      get_key_data(newacc.active_key,activedata);
      activekeyweight.key.data = activedata;
      activekeyweight.key.type = 0;
      activekeyweight.weight = 1;
      ultrainiosystem::authority     activekey = { .threshold = 1, .keys = { activekeyweight }, .accounts = {}, .waits = {} };
      print("updateactiveaccounts proposerminer:",name{newacc.account}," ownerkey:",newacc.owner_key," active_key:",newacc.active_key, "\n");
      action(
         permission_level{ N(ultrainio), N(active) },
         N(ultrainio), NEX(newaccount),
         std::make_tuple(N(ultrainio),newacc.account, ownerkey, activekey, newacc.updateable)
      ).send();
   }

    bool system_contract::accept_block_header(name chain_name, const ultrainio::signed_block_header& header, char* confirmed_bh_hash, size_t hash_len) {
      bytes header_bytes = pack(header);
      return lightclient_accept_block_header(chain_name, header_bytes.data(), header_bytes.size(), confirmed_bh_hash, hash_len);
    }

    int system_contract::verify_evil(const std::string& evidence, const evildoer& evil) {
       bytes evidence_bytes = pack(evidence);
       bytes evil_bytes = pack(evil);
       return native_verify_evil(evidence_bytes.data(), evidence_bytes.size(), evil_bytes.data(), evil_bytes.size());
    }

   /**
    *  Called after a new account is created. This code enforces resource-limits rules
    *  for new accounts as well as new account naming conventions.
    *
    *  More restriction on account creating rule can be found in
    *  apply_ultrainio_newaccount() in ultrainio_contract.cpp.
    *
    */
   void native::newaccount( account_name     creator,
                            account_name     newact
                            /*  no need to parse authorities
                            const authority& owner,
                            const authority& active*/ ) {
      global_state_singleton globalparams( _self,_self);
      std::string name_str = name{newact}.to_string();
      // Check if the creator is privileged
      if( !is_privileged( creator ) ) {
         ultrainio_assert( name_str.size() >= 5 && name_str.size() <= 12 , "account names must be between 5-12 in length" );
         bool is_contain_letter = false;
         bool is_contain_number = false;
         for( uint16_t i = 0; i < name_str.size(); i++ ) {
            char cha = name_str[i];
            if( cha >= 'a' && cha <= 'z' ) {
               is_contain_letter = true;
            }
            if( cha >= '1' && cha <= '5' ) {
               is_contain_number = true;
            }
         }
         ultrainio_assert( is_contain_letter && is_contain_number , "account names must contain lowercase letters and Numbers (1-5)" );
         ultrainio_assert( name_str.find( "utrio." ) != 0, "only privileged accounts can have names that start with 'utrio.'" );
      }
      int32_t newaccount_fee = 0;
      if(globalparams.exists()){
         ultrainio_global_state  _gstate = globalparams.get();
         newaccount_fee = (int32_t)_gstate.newaccount_fee;
         ultrainio_assert( _gstate.is_master_chain() || creator == _self, "only master chain allow create account" );
      }
      bool  is_free_create = false;
      chains_table _chains(_self,_self);
      for(auto ite_chain = _chains.begin(); ite_chain != _chains.end(); ++ite_chain) {
         if(ite_chain->chain_name == N(master))
               continue;
         resources_lease_table _reslease_tbl( _self,ite_chain->chain_name );
         auto reslease_itr = _reslease_tbl.find( creator );
         if( reslease_itr !=  _reslease_tbl.end() && reslease_itr->free_account_number > 0) {
            _reslease_tbl.modify( reslease_itr, [&]( auto& tot ) {
                  tot.free_account_number--;
               });
            is_free_create = true;
            break;
         }
      }
      freeaccount free_acc(_self,_self);
      auto itr_acc = free_acc.find(creator);
      if (itr_acc != free_acc.end() && itr_acc->acc_num > 0){
          free_acc.modify( itr_acc, [&]( auto& f  ) {
              f.acc_num--;
          });
          is_free_create = true;
      }
      if(!is_free_create && creator != _self){
        INLINE_ACTION_SENDER(ultrainiores::resource, modifyfreeaccount)
                            (N(utrio.res), {N(utrio.res), N(active)},{creator, 1});
      }
      // if (!is_free_create && creator != _self && newaccount_fee > 0) {
      //    INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {creator,N(active)},
      //                         { creator, N(utrio.fee), asset(newaccount_fee), std::string("create account") } );
      // }

      if ( name_str.find( "utrio." ) != 0 ) {
          set_resource_limits( newact, 0, 0, 0 );
      }
   }
   void native::updateauth( account_name     account,
                  permission_name  permission,
                  permission_name  parent,
                  const authority& data ){
         UNUSED(permission);
         UNUSED(parent);
         int64_t authsize = data.keys.size() + data.accounts.size() + data.waits.size();
         ultrainio_assert(authsize > 0, "update authority amount has to be greater than zero");

         global_state_singleton globalparams( _self,_self);
         ultrainio_assert(globalparams.exists(), "global state is not existed");
         ultrainio_global_state  _gstate = globalparams.get();
         if(!_gstate.is_master_chain()) {
            require_auth( _self );
            return;
         }
         int32_t updateauth_fee = 0;
         for(auto extension : _gstate.table_extension){
             if(extension.key == ultrainio_global_state::global_state_exten_type_key::update_auth) {
                 std::string str = extension.value;
                 updateauth_fee = std::stoi(str);
                 break;
             }
         }

         if(updateauth_fee > 0 && (name{account}.to_string().find( "utrio." ) != 0 ) && (account != _self)){
            INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {account,N(active)},
               { account, N(utrio.fee), asset(updateauth_fee * authsize), std::string("update auth") } );
         }
      }

    void native::deleteauth( account_name account, permission_name permission ) {
        UNUSED( account );
        UNUSED( permission );
    }
   void native::linkauth( account_name    account,
                          account_name    code,
                          action_name     type,
                          permission_name requirement ) {
      UNUSED( code );
      UNUSED( type );
      UNUSED( requirement );
      global_state_singleton globalparams( _self,_self);
      int32_t linkauth_fee = 10000;
      if(globalparams.exists()){
         ultrainio_global_state  _gstate = globalparams.get();
         for(auto extension : _gstate.table_extension){
            if(extension.key == ultrainio_global_state::global_state_exten_type_key::link_auth_fee
               && !extension.value.empty()) {
               linkauth_fee = std::stol( extension.value );
               break;
            }
         }
      }
      INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), { account,N(active) },
         { account, N(utrio.fee), asset( linkauth_fee ), name{account}.to_string() + std::string(" linkauth fee") } );
   }

   void native::deletetable( account_name code ) {
      require_auth(_self);
      int32_t dropstatus = db_drop_table(code);
      print(" native::deletetable contract account:" ,name{code}, " dropstatus:", dropstatus);
   }

   void native::delaccount( account_name account ) {
      require_auth( _self );
      asset account_tokens = ultrainio::token(N(utrio.token)).get_balance( account ,symbol_type(CORE_SYMBOL).name());
      ultrainio_assert( account_tokens.amount == 0, " The account still has a balance and cannot be deleted" );
      producer_brief_table   _briefproducers( _self, _self );
      auto const briefprod = _briefproducers.find( account );
      ultrainio_assert(briefprod == _briefproducers.end(), "this account is a producer and cannot be deleted");
      /*
      chains_table _chains(_self,_self);
      for(auto ite_chain = _chains.begin(); ite_chain != _chains.end(); ++ite_chain) {
         if( ite_chain->chain_name == N(master) )
            continue;
         resources_lease_table _reslease_tbl( _self, ite_chain->chain_name );
         auto reslease_itr = _reslease_tbl.find( account );
         ultrainio_assert( reslease_itr ==  _reslease_tbl.end(), "this account has purchased resource and cannot be deleted");
      }*/
      print(" native::delaccount contract account:" ,name{account} );
   }
} /// ultrainio.system


ULTRAINIO_ABI( ultrainiosystem::system_contract,
     // native.hpp (newaccount definition is actually in ultrainio.system.cpp)
     (newaccount)(updateauth)(deleteauth)(linkauth)(unlinkauth)(canceldelay)(onerror)(deletetable)(delaccount)(addwhiteblack)(rmwhiteblack)
     // ultrainio.system.cpp
     (setsysparams)(setglobalextendata)(setmasterchaininfo)(setparams)(setpriv)(setupdateabled)(setprodontimeblock)(setchaintypeextendata)(updatecommercdel)
     // delegate.cpp
     (delegatecons)(undelegatecons)(refundcons)(setfreeacc)
     // producer.cpp
     (regproducer)(moveprod)(verifyprodevil)(procevilprod)(prodheartbeat)
     // reward.cpp
     (onblock)(onfinish)(calcmasterrewards)(claimrewards)(rewardproof)
     // scheduler.cpp
     (regchaintype)(regsubchain)(acceptmaster)(acceptheader)(clearchain)(empoweruser)
     (reportblockwshash)(reportsubchainhash)(setgenesisprodpk)(startnewchain)
     (setsched)(forcesetblock)(setlwcparams)(setchainparam)(destorychain)(empower)
     // synctransaction.cpp
     (synclwctx)
)
