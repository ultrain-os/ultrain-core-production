#include "ultrainio.system.hpp"
#include <ultrainiolib/dispatcher.hpp>
#include <ultrainiolib/action.hpp>

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
    _pendingminer( _self, _self ),
    _pendingaccount( _self, _self ),
    _pendingres( _self, _self ),
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

   void system_contract::checkvotefrequency(ultrainiosystem::producers_table& prod_tbl, ultrainiosystem::producers_table::const_iterator propos){
      uint64_t cur_block_num = (uint64_t)head_block_number() + 1;
      if((cur_block_num - propos->last_vote_blocknum) < 60){
         ultrainio_assert( propos->vote_number <= 600 , "too high voting frequency" );
      }else{
         prod_tbl.modify( propos, [&](auto& p ) {
            p.last_vote_blocknum = cur_block_num;
            p.vote_number = 0;
         });
      }
      prod_tbl.modify( propos, [&](auto& p ) {
         p.vote_number++;
      });
   }
   void system_contract::updateactiveminers(const ultrainio::proposeminer_info&  miner ) {
      if(miner.adddel_miner){
         producers_table _producers(_self, master_chain_name);
         auto prod = _producers.find( miner.account );
         if(prod != _producers.end()){
            print("\nupdateactiveminers curproducer existed  proposerminer:",name{(*prod).owner});
            return;
         }
         print("updateactiveminers add  proposerminer:",name{miner.account});
         vector<permission_level>   authorization;
         authorization.emplace_back(permission_level{ N(ultrainio), N(active)});
         authorization.emplace_back(permission_level{miner.account, N(active)});
         INLINE_ACTION_SENDER(ultrainiosystem::system_contract, regproducer)( N(ultrainio), authorization,
            { miner.account, miner.public_key, miner.bls_key, N(ultrainio), miner.url, miner.location } );
         INLINE_ACTION_SENDER(ultrainiosystem::system_contract, delegatecons)( N(ultrainio), {N(utrio.stake), N(active)},
            { N(utrio.stake),miner.account,asset(_gstate.min_activated_stake)} );
      }else{
         producers_table _producers(_self, master_chain_name);
         auto prod = _producers.find( miner.account );
         if( prod == _producers.end() ) {
            print("updateactiveminers curproducer not found  proposerminer:",ultrainio::name{(*prod).owner}," total_cons_staked:",(*prod).total_cons_staked);
            return;
         }

         print("updateactiveminers del proposerminer:",ultrainio::name{(*prod).owner}," total_cons_staked:",(*prod).total_cons_staked);
         if((*prod).total_cons_staked > 0){
            INLINE_ACTION_SENDER(ultrainiosystem::system_contract, undelegatecons)( N(ultrainio), {N(utrio.stake), N(active)},
               { N(utrio.stake),(*prod).owner} );
         }
      }
   }
   void system_contract::votecommittee() {
      constexpr size_t max_stack_buffer_size = 512;
      size_t size = action_data_size();
      const bool heap_allocation = max_stack_buffer_size < size;
      char* buffer = (char*)( heap_allocation ? malloc(size) : alloca(size) );
      read_action_data( buffer, size );
      account_name proposer;
      vector<proposeminer_info> proposeminer;
      datastream<const char*> ds( buffer, size );
      ds >> proposer >> proposeminer;
      ultrainio_assert( proposeminer.size() == 1, "The number of proposeminer changes cannot exceed one" );
      ultrainio_assert( proposeminer[0].url.size() < 512, "url too long" );
      require_auth( proposer );
      producers_table _producers(_self, master_chain_name);
      auto propos = _producers.find( proposer );
      ultrainio_assert( propos != _producers.end(), "enabled producer not found this proposer" );
      checkvotefrequency(_producers, propos );
      uint32_t  enableprodnum = _gstate.cur_committee_number;
      ultrainio_assert( enableprodnum > _gstate.min_committee_member_number || proposeminer[0].adddel_miner, " The number of committees is about to be smaller than the minimum number and therefore cannot be voted away" );
      print("votecommittee enableprodnum size:", enableprodnum," proposer:",ultrainio::name{proposer}," accountsize:",proposeminer.size(),"\n");
      for(auto minerinfo : proposeminer){
         ultrainio_assert( is_account( minerinfo.account ), "vote votecommittee account not exist");
         minerinfo.approve_num = 1;
         provided_proposer  provideapprve(proposer,(uint32_t)head_block_number(),0);
         auto pendingiter = _pendingminer.find( minerinfo.account );
         if ( pendingiter != _pendingminer.end() ) {
            int32_t curproposeresnum = -1;
            uint32_t proposeaccountsize = (*pendingiter).proposal_miner.size();
            for(uint32_t i = 0;i < proposeaccountsize;i++)
            {
               if((minerinfo.account == (*pendingiter).proposal_miner[i].account) &&
                  (minerinfo.public_key == (*pendingiter).proposal_miner[i].public_key)  &&
                  (minerinfo.bls_key == (*pendingiter).proposal_miner[i].bls_key)  &&
                  (minerinfo.location == (*pendingiter).proposal_miner[i].location)  &&
                  (minerinfo.adddel_miner == (*pendingiter).proposal_miner[i].adddel_miner)){
                  curproposeresnum = (int32_t)i;
                  break;
               }
            }
            if(curproposeresnum >= 0){
               _pendingminer.modify( pendingiter, [&]( auto& p ) {
                  auto itr = std::find( p.provided_approvals.begin(), p.provided_approvals.end(), provideapprve );
                  if(itr != p.provided_approvals.end())
                  {
                     if((itr->resource_index == (uint64_t)curproposeresnum) && (block_interval_seconds()*(provideapprve.last_vote_blockheight - itr->last_vote_blockheight) < seconds_per_halfhour)){
                           print("\nvoteaccount proposer already voted proposer :",ultrainio::name{proposer}," current_last_vote_blockheight:",provideapprve.last_vote_blockheight," last_vote_blockheight:",itr->last_vote_blockheight);
                           ultrainio_assert( false, "voteaccount proposer already voted" );
                     }
                     p.provided_approvals.erase(itr);
                  }
                  p.proposal_miner[(uint32_t)curproposeresnum].approve_num++;
                  provideapprve.resource_index = (uint64_t)curproposeresnum;
                  p.provided_approvals.push_back(provideapprve);
               });

               if((*pendingiter).proposal_miner[(uint32_t)curproposeresnum].approve_num >= ceil((double)(enableprodnum)*2/3)){
                  updateactiveminers(minerinfo);
                  _pendingminer.modify( pendingiter, [&]( auto& p ) {
                     p.provided_approvals.clear();
                     p.proposal_miner.clear();
                  });
                  _pendingminer.erase( pendingiter );
               }
            }else{
               _pendingminer.modify( pendingiter, [&]( auto& p ) {
                  p.proposal_miner.push_back(minerinfo);
                  provideapprve.resource_index = p.proposal_miner.size() - 1;
                  p.provided_approvals.push_back(provideapprve);
               });
            }
         }else{
            _pendingminer.emplace( [&]( auto& p ) {
               p.owner = minerinfo.account;
               p.provided_approvals.push_back(provideapprve);
               p.proposal_miner.push_back(minerinfo);
            });
         }
      }
      if(heap_allocation){
          free(buffer);
      }
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
void system_contract::voteaccount() {
      constexpr size_t max_stack_buffer_size = 512;
      size_t size = action_data_size();
      const bool heap_allocation = max_stack_buffer_size < size;
      char* buffer = (char*)( heap_allocation ? malloc(size) : alloca(size) );
      read_action_data( buffer, size );
      account_name proposer;
      vector<proposeaccount_info> proposeaccount;
      datastream<const char*> ds( buffer, size );
      ds >> proposer >> proposeaccount;
      ultrainio_assert( proposeaccount.size() > 0 && proposeaccount.size() <= 10, "The number of proposeaccount changes changes not correct" );
      require_auth( proposer );
      producers_table _producers(_self, master_chain_name);
      auto propos = _producers.find( proposer );
      ultrainio_assert( propos != _producers.end(), "enabled producer not found this proposer" );
      checkvotefrequency(_producers, propos );
      uint32_t  enableprodnum = _gstate.cur_committee_number;
      print("voteaccount enableprodnum size:", enableprodnum," proposer:",ultrainio::name{proposer}," accountsize:",proposeaccount.size(),"\n");
      for(auto accinfo : proposeaccount){
         ultrainio_assert( !is_account( accinfo.account ), "vote create account already exist");
         ultrainio_assert( accinfo.owner_key.length() == 53, "vote owner public key should be of size 53" );
         ultrainio_assert( accinfo.active_key.length() == 53, "vote active public key should be of size 53" );
         accinfo.approve_num = 1;
         provided_proposer  provideapprve(proposer,(uint32_t)head_block_number(),0);
         auto pendingiter = _pendingaccount.find( accinfo.account );
         if ( pendingiter != _pendingaccount.end() ) {
            int32_t curproposeresnum = -1;
            uint32_t proposeaccountsize = (*pendingiter).proposal_account.size();
            for(uint32_t i = 0;i < proposeaccountsize;i++)
            {
               if((accinfo.account == (*pendingiter).proposal_account[i].account) &&
                  (accinfo.owner_key == (*pendingiter).proposal_account[i].owner_key)  &&
                  (accinfo.active_key == (*pendingiter).proposal_account[i].active_key)  &&
                  (accinfo.updateable == (*pendingiter).proposal_account[i].updateable)  &&
                  (accinfo.location == (*pendingiter).proposal_account[i].location) ){
                  curproposeresnum = (int32_t)i;
                  break;
               }
            }
            if(curproposeresnum >= 0){
               _pendingaccount.modify( pendingiter, [&]( auto& p ) {
                  auto itr = std::find( p.provided_approvals.begin(), p.provided_approvals.end(), provideapprve );
                  if(itr != p.provided_approvals.end())
                  {
                     if((itr->resource_index == (uint64_t)curproposeresnum) && (block_interval_seconds()*(provideapprve.last_vote_blockheight - itr->last_vote_blockheight) < seconds_per_halfhour)){
                           print("\nvoteaccount proposer already voted proposer :",ultrainio::name{proposer}," current_last_vote_blockheight:",provideapprve.last_vote_blockheight," last_vote_blockheight:",itr->last_vote_blockheight);
                           ultrainio_assert( false, "voteaccount proposer already voted" );
                     }
                     p.provided_approvals.erase(itr);
                  }
                  p.proposal_account[(uint32_t)curproposeresnum].approve_num++;
                  provideapprve.resource_index = (uint64_t)curproposeresnum;
                  p.provided_approvals.push_back(provideapprve);
               });

               if((*pendingiter).proposal_account[(uint32_t)curproposeresnum].approve_num >= ceil((double)enableprodnum*2/3)){
                  add_subchain_account(accinfo);
                  _pendingaccount.modify( pendingiter, [&]( auto& p ) {
                     p.provided_approvals.clear();
                     p.proposal_account.clear();
                  });
                  _pendingaccount.erase( pendingiter );
               }
            }else{
               _pendingaccount.modify( pendingiter, [&]( auto& p ) {
                  p.proposal_account.push_back(accinfo);
                  provideapprve.resource_index = p.proposal_account.size() - 1;
                  p.provided_approvals.push_back(provideapprve);
               });
            }
         }else{
            _pendingaccount.emplace( [&]( auto& p ) {
               p.owner = accinfo.account;
               p.provided_approvals.push_back(provideapprve);
               p.proposal_account.push_back(accinfo);
            });
         }
      }
      if(heap_allocation){
          free(buffer);
      }
   }

void system_contract::voteresourcelease() {
      constexpr size_t max_stack_buffer_size = 512;
      size_t size = action_data_size();
      const bool heap_allocation = max_stack_buffer_size < size;
      char* buffer = (char*)( heap_allocation ? malloc(size) : alloca(size) );
      read_action_data( buffer, size );
      account_name proposer;
      vector<proposeresource_info> proposeresource;
      datastream<const char*> ds( buffer, size );
      ds >> proposer >> proposeresource;
      ultrainio_assert( proposeresource.size() > 0 && proposeresource.size() <= 10, "The number of proposeresource changes not correct" );
      require_auth( proposer );
      producers_table _producers(_self, master_chain_name);
      auto propos = _producers.find( proposer );
      ultrainio_assert( propos != _producers.end(), "enabled producer not found this proposer" );
      checkvotefrequency(_producers, propos );
      uint32_t  enableprodnum = _gstate.cur_committee_number;
      print("voteresourcelease enableprodnum size:", enableprodnum," proposer:",ultrainio::name{proposer}," proposeresource_info size:",proposeresource.size(),"\n");
      for(auto resinfo : proposeresource){
         ultrainio_assert( is_account( resinfo.account ), "vote resoucelease account not exist");
         resinfo.approve_num = 1;
         provided_proposer  provideapprve( proposer, (uint32_t)head_block_number(), 0);
         auto pendingiter = _pendingres.find( resinfo.account );
         if ( pendingiter != _pendingres.end() ) {
            int32_t curproposeresnum = -1;
            uint32_t proposeaccountsize = (*pendingiter).proposal_resource.size();
            for(uint32_t i = 0;i < proposeaccountsize;i++)
            {
               if((resinfo.account == (*pendingiter).proposal_resource[i].account) &&
                  (resinfo.lease_num == (*pendingiter).proposal_resource[i].lease_num)  &&
                  (resinfo.block_height_interval == (*pendingiter).proposal_resource[i].block_height_interval)  &&
                  (resinfo.location == (*pendingiter).proposal_resource[i].location) ){
                  curproposeresnum = (int32_t)i;
                  break;
               }
            }
            if(curproposeresnum >= 0){
               _pendingres.modify( pendingiter, [&]( auto& p ) {
                  auto itr = std::find( p.provided_approvals.begin(), p.provided_approvals.end(), provideapprve );
                  if(itr != p.provided_approvals.end())
                  {
                     if((itr->resource_index == (uint64_t)curproposeresnum) && (block_interval_seconds()*(provideapprve.last_vote_blockheight - itr->last_vote_blockheight) < seconds_per_halfhour)){
                           print("\nvoteresourcelease proposer already voted proposer :",ultrainio::name{proposer}," current_last_vote_blockheight:",provideapprve.last_vote_blockheight," last_vote_blockheight:",itr->last_vote_blockheight);
                           ultrainio_assert( false, "proposer already voted" );
                     }
                     p.provided_approvals.erase(itr);
                  }
                  p.proposal_resource[(uint32_t)curproposeresnum].approve_num++;
                  provideapprve.resource_index = (uint64_t)curproposeresnum;
                  p.provided_approvals.push_back(provideapprve);
               });

               if((*pendingiter).proposal_resource[(uint32_t)curproposeresnum].approve_num >= ceil((double)enableprodnum*2/3)){
                  syncresource(resinfo.account, resinfo.lease_num, resinfo.block_height_interval);
                  _pendingres.modify( pendingiter, [&]( auto& p ) {
                     p.provided_approvals.clear();
                     p.proposal_resource.clear();
                  });
                  _pendingres.erase( pendingiter );
               }
            }else{
               _pendingres.modify( pendingiter, [&]( auto& p ) {
                  p.proposal_resource.push_back(resinfo);
                  provideapprve.resource_index = p.proposal_resource.size() - 1;
                  p.provided_approvals.push_back(provideapprve);
               });
            }
         }else{
            _pendingres.emplace( [&]( auto& p ) {
               p.owner = resinfo.account;
               p.provided_approvals.push_back(provideapprve);
               p.proposal_resource.push_back(resinfo);
            });
         }
      }
      if(heap_allocation){
          free(buffer);
      }
   }

   void system_contract::cleanvotetable(){
      auto block_height = (uint32_t)head_block_number() + 1;
      uint32_t interval_num = seconds_per_halfhour/block_interval_seconds();
      if(block_height < 120 || block_height%interval_num != 0) {
         return;
      }
      uint32_t curblockheight = (uint32_t)head_block_number() + 1;
      uint64_t starttime = current_time();
      //clean vote pendingminer expire
      for(auto mineriter = _pendingminer.begin(); mineriter != _pendingminer.end(); ){
         _pendingminer.modify( mineriter, [&]( auto& p ) {
            for(auto itr = p.provided_approvals.begin(); itr != p.provided_approvals.end();){
               if(block_interval_seconds()*(curblockheight - itr->last_vote_blockheight) > seconds_per_halfhour){
                  itr = p.provided_approvals.erase(itr);
               }else
                  ++itr;
            }
         });
         print("cleanvotetable _pendingminer name:",name{mineriter->owner}, " approversize::",mineriter->provided_approvals.size()," curblockheight:",curblockheight);
         if(mineriter->provided_approvals.size() == 0){
            mineriter = _pendingminer.erase(mineriter);
         }else
            ++mineriter;
      }

      //clean vote _pendingaccount expire
      for(auto acciter = _pendingaccount.begin(); acciter != _pendingaccount.end(); ){
         _pendingaccount.modify( acciter, [&]( auto& p ) {
            for(auto itr = p.provided_approvals.begin(); itr != p.provided_approvals.end();){
               if(block_interval_seconds()*(curblockheight - itr->last_vote_blockheight) > seconds_per_halfhour){
                  itr = p.provided_approvals.erase(itr);
               }else
                  ++itr;
            }
         });
         print("cleanvotetable _pendingaccount name:",name{acciter->owner}, " approversize::",acciter->provided_approvals.size()," curblockheight:",curblockheight);
         if(acciter->provided_approvals.size() == 0){
            acciter = _pendingaccount.erase(acciter);
         }else
            ++acciter;
      }

      //clean vote _pendingres expire
      for(auto resiter = _pendingres.begin(); resiter != _pendingres.end(); ){
         _pendingres.modify( resiter, [&]( auto& p ) {
            for(auto itr = p.provided_approvals.begin(); itr != p.provided_approvals.end();){
               if(block_interval_seconds()*(curblockheight - itr->last_vote_blockheight) > seconds_per_halfhour){
                  itr = p.provided_approvals.erase(itr);
               }else
                  ++itr;
            }
         });
         print("cleanvotetable _pendingres name:",name{resiter->owner}, " approversize::",resiter->provided_approvals.size()," curblockheight:",curblockheight);
         if(resiter->provided_approvals.size() == 0){
            resiter = _pendingres.erase(resiter);
         }else
            ++resiter;
      }
      uint64_t endtime = current_time();
      print("cleanvotetable expend time:",(endtime - starttime), "\n");
   }

    bool system_contract::accept_block_header(name chain_name, const ultrainio::signed_block_header& header, char* confirmed_bh_hash, size_t hash_len) {
      bytes header_bytes = pack(header);
      return lightclient_accept_block_header(chain_name, header_bytes.data(), header_bytes.size(), confirmed_bh_hash, hash_len);
    }

   /**
    *  Called after a new account is created. This code enforces resource-limits rules
    *  for new accounts as well as new account naming conventions.
    *
    *  Account names containing '.' symbols must have a suffix equal to the name of the creator.
    *  This allows users who buy a premium name (shorter than 12 characters with no dots) to be the only ones
    *  who can create accounts with the creator's name as a suffix.
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

   void native::deletetable( account_name code ) {
      require_auth(_self);
      print(" native::deletetable contract account:" ,name{code} );
      db_drop_table(code);
   }
} /// ultrainio.system


ULTRAINIO_ABI( ultrainiosystem::system_contract,
     // native.hpp (newaccount definition is actually in ultrainio.system.cpp)
     (newaccount)(updateauth)(deleteauth)(linkauth)(unlinkauth)(canceldelay)(onerror)(deletetable)
     // ultrainio.system.cpp
     (setsysparams)(setmasterchaininfo)(setparams)(setpriv)(setupdateabled)(votecommittee)(voteaccount)(voteresourcelease)
     // delegate.cpp
     (delegatecons)(undelegatecons)(refundcons)(resourcelease)(recycleresource)
     // producer.cpp
     (regproducer)(unregprod)
     // reward.cpp
     (onblock)(claimrewards)
     // scheduler.cpp
     (regchaintype)(regsubchain)(acceptmaster)(acceptheader)(clearchain)(empoweruser)(reportsubchainhash)
     (setsched)(forcesetblock)(schedule)(setlwcparams)(setchainparam)
     // synctransaction.cpp
     (synclwctx)
)
