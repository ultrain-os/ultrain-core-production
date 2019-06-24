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
            ultrainio_assert( _gstate.cur_committee_number > params.min_committee_member_number,
                              "the current number of committees is greater than the minimum number of committees set up" );
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
          uncfm_block.is_leaf = true;
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
      if(has_auth(_self)){
         require_auth( _self );
      }else{
         require_auth( account );
         ultrainio_assert( !is_update, "the contract account can only be set to unupdatable" );
         INLINE_ACTION_SENDER(ultrainio::token, safe_transfer)( N(utrio.token), {account,N(active)},
            { account, N(utrio.fee), asset(1000), std::string("setupdateabled") } );  //0.1 UGAS service charge
      }
      set_updateabled( account, is_update );
   }

   void system_contract::getKeydata(const std::string& pubkey,std::array<char,33> & data){
      auto const getHexvalue = [](const char ch)->int{
         if(ch >= '0' && ch <= '9')
            return (ch-'0');
         if(ch >= 'a' && ch <= 'f')
            return ((ch-'a')+10);
         return 0;
      };
      char  keydata[67];
      memset(keydata,0,sizeof(keydata));
      frombase58_recover_key(pubkey.c_str(), keydata, 66);
      unsigned int j = 0;
      for ( uint32_t i=0; i < strlen(keydata); i++ ){
         if(i%2 == 1)
         {
            data[j] = static_cast<char>((getHexvalue(keydata[i-1])*16 + getHexvalue(keydata[i])) & 0xff);
            j++;
         }
      }
   }
   void system_contract::add_subchain_account(const ultrainio::proposeaccount_info&  newacc ) {
      ultrainiosystem::key_weight ownerkeyweight;
      std::array<char,33> ownerdata;
      getKeydata(newacc.owner_key,ownerdata);
      ownerkeyweight.key.data = ownerdata;
      ownerkeyweight.key.type = 0;
      ownerkeyweight.weight = 1;
      ultrainiosystem::authority    ownerkey  = { .threshold = 1, .keys = { ownerkeyweight }, .accounts = {}, .waits = {} };

      ultrainiosystem::key_weight activekeyweight;
      std::array<char,33> activedata;
      getKeydata(newacc.active_key,activedata);
      activekeyweight.key.data = activedata;
      activekeyweight.key.type = 0;
      activekeyweight.weight = 1;
      ultrainiosystem::authority     activekey = { .threshold = 1, .keys = { activekeyweight }, .accounts = {}, .waits = {} };
      print("updateactiveaccounts proposerminer:",name{newacc.account}," ownerkey:",newacc.owner_key," active_key:",newacc.active_key);
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
      if(!is_free_create){
         freeaccount free_acc(_self,_self);
         auto itr_acc = free_acc.find(creator);
         if (itr_acc != free_acc.end() && itr_acc->acc_num > 0){
            free_acc.modify( itr_acc, [&]( auto& f  ) {
               f.acc_num--;
            });
            is_free_create = true;
         }
      }
      ultrainio_assert( is_free_create || creator == _self, "The current free account is insufficient, please purchase resouce to get the free account" );
      // if (!is_free_create && creator != _self && newaccount_fee > 0) {
      //    INLINE_ACTION_SENDER(ultrainio::token, safe_transfer)( N(utrio.token), {creator,N(active)},
      //                         { creator, N(utrio.fee), asset(newaccount_fee), std::string("create account") } );
      // }

      set_resource_limits( newact, 0, 0, 0 );
   }
   void native::updateauth( account_name     account,
                  permission_name  permission,
                  permission_name  parent,
                  const authority& data ){
         UNUSED(permission);
         UNUSED(parent);
         global_state_singleton globalparams( _self,_self);
         int32_t updateauth_fee = 0;
         if(globalparams.exists()){
            ultrainio_global_state  _gstate = globalparams.get();
            for(auto extension : _gstate.table_extension){
               if(extension.key == ultrainio_global_state::global_state_exten_type_key::update_auth) {
                  std::string str = extension.value;
                  updateauth_fee = std::stoi(str);
                  break;
               }
            }
         }
         int64_t authsize = data.keys.size() + data.accounts.size() + data.waits.size();
         ultrainio_assert(authsize > 0, "update authority amount has to be greater than zero");
         if(updateauth_fee > 0 && (name{account}.to_string().find( "utrio." ) != 0 ) && (account != _self)){
            INLINE_ACTION_SENDER(ultrainio::token, safe_transfer)( N(utrio.token), {account,N(active)},
               { account, N(utrio.fee), asset(updateauth_fee * authsize), std::string("update auth") } );
         }
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
      INLINE_ACTION_SENDER(ultrainio::token, safe_transfer)( N(utrio.token), { account,N(active) },
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
      chains_table _chains(_self,_self);
      for(auto ite_chain = _chains.begin(); ite_chain != _chains.end(); ++ite_chain) {
         if( ite_chain->chain_name == N(master) )
            continue;
         resources_lease_table _reslease_tbl( _self, ite_chain->chain_name );
         auto reslease_itr = _reslease_tbl.find( account );
         ultrainio_assert( reslease_itr ==  _reslease_tbl.end(), "this account has purchased resource and cannot be deleted");
      }
      print(" native::delaccount contract account:" ,name{account} );
   }

   void native::addblacklist( account_name account ) {
      require_auth( _self );
      print(" native::addblacklist contract account:" ,name{account} );
   }

   void native::rmblacklist( account_name account ) {
      require_auth( _self );
      print("11 native::rmblacklist contract account:" ,name{account} );
   }

} /// ultrainio.system


ULTRAINIO_ABI( ultrainiosystem::system_contract,
     // native.hpp (newaccount definition is actually in ultrainio.system.cpp)
     (newaccount)(updateauth)(deleteauth)(linkauth)(unlinkauth)(canceldelay)(onerror)(deletetable)(delaccount)(addblacklist)(rmblacklist)
     // ultrainio.system.cpp
     (setsysparams)(setglobalextendata)(setmasterchaininfo)(setparams)(setpriv)(setupdateabled)
     // delegate.cpp
     (delegatecons)(undelegatecons)(refundcons)(resourcelease)(recycleresource)(setfreeacc)
     // producer.cpp
     (regproducer)(moveprod)
     // reward.cpp
     (onblock)(onfinish)(calcmasterrewards)(claimrewards)
     // scheduler.cpp
     (regchaintype)(regsubchain)(acceptmaster)(acceptheader)(clearchain)(empoweruser)(reportsubchainhash)
     (setsched)(forcesetblock)(setlwcparams)(setchainparam)
     // synctransaction.cpp
     (synclwctx)
)
