#include "ultrainio.system.hpp"
#include <ultrainiolib/dispatcher.hpp>

#include "producer_pay.cpp"
#include "delegate_bandwidth.cpp"
#include "voting.cpp"
#include "exchange_state.cpp"
#include "scheduler.cpp"
#include <ultrainiolib/action.hpp>


namespace ultrainiosystem {

   system_contract::system_contract( account_name s )
   :native(s),
    _producers(_self,_self),
    _global(_self,_self),
    _rammarket(_self,_self),
    _pending_que(_self, _self),
    _subchains(_self,_self),
    _pendingminer( _self, _self ),
    _pendingaccount( _self, _self ),
    _pendingres( _self, _self ),
    _schedsetting(_self, _self) {
      //print( "construct system\n" );
      _gstate = _global.exists() ? _global.get() : get_default_parameters();

      auto itr = _rammarket.find(S(4,RAMCORE));

      if( itr == _rammarket.end() ) {
         auto system_token_supply   = ultrainio::token(N(utrio.token)).get_supply(ultrainio::symbol_type(system_token_symbol).name()).amount;
         if( system_token_supply > 0 ) {
            itr = _rammarket.emplace( _self, [&]( auto& m ) {
               m.supply.amount = 100000000000000ll;
               m.supply.symbol = S(4,RAMCORE);
               m.base.balance.amount = int64_t(_gstate.free_ram());
               m.base.balance.symbol = S(0,RAM);
               m.quote.balance.amount = system_token_supply / 1000;
               m.quote.balance.symbol = CORE_SYMBOL;
            });
         }
      } else {
         //print( "ram market already created" );
      }
   }

   ultrainio_global_state system_contract::get_default_parameters() {
      ultrainio_global_state dp;
      get_blockchain_parameters(dp);
      return dp;
   }


   system_contract::~system_contract() {
      //print( "destruct system\n" );
      _global.set( _gstate, _self );
      //ultrainio_exit(0);
   }

   void system_contract::setram( uint64_t max_ram_size ) {
      require_auth( _self );

      ultrainio_assert( _gstate.max_ram_size < max_ram_size, "ram may only be increased" ); /// decreasing ram might result market maker issues
      ultrainio_assert( max_ram_size < 1024ll*1024*1024*1024*1024, "ram size is unrealistic" );
      ultrainio_assert( max_ram_size > _gstate.total_ram_bytes_reserved, "attempt to set max below reserved" );

      auto delta = int64_t(max_ram_size) - int64_t(_gstate.max_ram_size);
      auto itr = _rammarket.find(S(4,RAMCORE));

      /**
       *  Increase or decrease the amount of ram for sale based upon the change in max
       *  ram size.
       */
      _rammarket.modify( itr, 0, [&]( auto& m ) {
         m.base.balance.amount += delta;
      });

      _gstate.max_ram_size = max_ram_size;
      _global.set( _gstate, _self );
   }

   void system_contract::setblockreward( uint64_t rewardvalue ) {
      require_auth( _self );
      _gstate.reward_preblock = rewardvalue;
      _global.set( _gstate, _self );
   }

   void system_contract::setmincommittee( uint32_t number, uint32_t staked ) {
      require_auth( _self );
      _gstate.min_committee_member_number = number;
      _gstate.min_activated_stake = (int64_t)staked;
      _global.set( _gstate, _self );
   }

   void system_contract::setparams( const ultrainio::blockchain_parameters& params ) {
      require_auth( N(ultrainio) );
      (ultrainio::blockchain_parameters&)(_gstate) = params;
      ultrainio_assert( 3 <= _gstate.max_authority_depth, "max_authority_depth should be at least 3" );
      set_blockchain_parameters( params );
   }

   void system_contract::setpriv( account_name account, uint8_t ispriv ) {
      require_auth( _self );
      set_privileged( account, ispriv );
   }

   void system_contract::rmvproducer( account_name producer ) {
      require_auth( _self );
      auto prod = _producers.find( producer );
      ultrainio_assert( prod != _producers.end(), "producer not found" );
      _producers.modify( prod, 0, [&](auto& p) {
            p.deactivate();
         });
   }

   void system_contract::bidname( account_name bidder, account_name newname, asset bid ) {
      require_auth( bidder );
      ultrainio_assert( ultrainio::name_suffix(newname) == newname, "you can only bid on top-level suffix" );
      ultrainio_assert( newname != 0, "the empty name is not a valid account name to bid on" );
      ultrainio_assert( (newname & 0xFull) == 0, "13 character names are not valid account names to bid on" );
      ultrainio_assert( (newname & 0x1F0ull) == 0, "accounts with 12 character names and no dots can be created without bidding required" );
      ultrainio_assert( !is_account( newname ), "account already exists" );
      ultrainio_assert( bid.symbol == asset().symbol, "asset must be system token" );
      ultrainio_assert( bid.amount > 0, "insufficient bid" );

      INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {bidder,N(active)},
                                                    { bidder, N(utrio.names), bid, std::string("bid name ")+(name{newname}).to_string()  } );

      name_bid_table bids(_self,_self);
      print( name{bidder}, " bid ", bid, " on ", name{newname}, "\n" );
      auto current = bids.find( newname );
      if( current == bids.end() ) {
         bids.emplace( bidder, [&]( auto& b ) {
            b.newname = newname;
            b.high_bidder = bidder;
            b.high_bid = bid.amount;
            b.last_bid_time = current_time();
         });
      } else {
         ultrainio_assert( current->high_bid > 0, "this auction has already closed" );
         ultrainio_assert( bid.amount - current->high_bid > (current->high_bid / 10), "must increase bid by 10%" );
         ultrainio_assert( current->high_bidder != bidder, "account is already highest bidder" );

         INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {N(utrio.names),N(active)},
                                                       { N(utrio.names), current->high_bidder, asset(current->high_bid),
                                                       std::string("refund bid on name ")+(name{newname}).to_string()  } );

         bids.modify( current, bidder, [&]( auto& b ) {
            b.high_bidder = bidder;
            b.high_bid = bid.amount;
            b.last_bid_time = current_time();
         });
      }
   }
   void system_contract::checkvotefrequency(ultrainiosystem::producers_table::const_iterator propos){
      uint64_t cur_block_num = (uint64_t)tapos_block_num();
      if((cur_block_num - propos->last_vote_blocknum) < 60){
         ultrainio_assert( propos->vote_number <= 600 , "too high voting frequency" );
      }else{
         _producers.modify( propos, 0, [&](auto& p ) {
            p.last_vote_blocknum = cur_block_num;
            p.vote_number = 0;
         });
      }
      _producers.modify( propos, 0, [&](auto& p ) {
         p.vote_number++;
      });
   }
   void system_contract::updateactiveminers(const ultrainio::proposeminer_info&  miner ) {
      if(miner.adddel_miner){
         auto prod = _producers.find( miner.account );
         if(prod != _producers.end() && prod->is_enabled){
            print("\nupdateactiveminers curproducer existed  proposerminer:",name{(*prod).owner});
            return;
         }
         print("updateactiveminers add  proposerminer:",name{miner.account});
         vector<permission_level>   authorization;
         authorization.emplace_back(permission_level{ N(ultrainio), N(active)});
         authorization.emplace_back(permission_level{miner.account, N(active)});
         INLINE_ACTION_SENDER(ultrainiosystem::system_contract, regproducer)( N(ultrainio), authorization,
            { miner.account, miner.public_key, miner.bls_key, miner.url, miner.location, N(ultrainio)} );
         INLINE_ACTION_SENDER(ultrainiosystem::system_contract, delegatecons)( N(ultrainio), {N(utrio.stake), N(active)},
            { N(utrio.stake),miner.account,asset(_gstate.min_activated_stake)} );
      }else{
         auto prod = _producers.find( miner.account );
         if(prod == _producers.end() || !(prod->is_active))
         {
            print("updateactiveminers curproducer not found  proposerminer:",ultrainio::name{(*prod).owner}," total_cons_staked:",(*prod).total_cons_staked);
            return;
         }

         print("updateactiveminers del proposerminer:",ultrainio::name{(*prod).owner}," total_cons_staked:",(*prod).total_cons_staked);
         if((*prod).total_cons_staked > 0){
            INLINE_ACTION_SENDER(ultrainiosystem::system_contract, undelegatecons)( N(ultrainio), {N(utrio.stake), N(active)},
               { N(utrio.stake),(*prod).owner} );
         }
         _producers.modify( prod, 0, [&](auto& p) {
               p.deactivate();
            });
      }
   }
   void system_contract::votecommittee() {
      size_t size = action_data_size();
      char* buffer = (char*)alloca(size);
      read_action_data( buffer, size );
      account_name proposer;
      vector<proposeminer_info> proposeminer;
      datastream<const char*> ds( buffer, size );
      ds >> proposer >> proposeminer;
      ultrainio_assert( proposeminer.size() == 1, "The number of proposeminer changes cannot exceed one" );
      ultrainio_assert( proposeminer[0].url.size() < 512, "url too long" );
      require_auth( proposer );
      auto propos = _producers.find( proposer );
      ultrainio_assert( propos != _producers.end() && propos->is_enabled && propos->is_on_master_chain(), "enabled producer not found this proposer" );
      checkvotefrequency( propos );
      uint32_t  enableprodnum = 0;
      for(auto itr = _producers.begin(); itr != _producers.end(); ++itr){
         if(itr->is_enabled && itr->is_on_master_chain())
            ++enableprodnum;
      }
      print("votecommittee enableprodnum size:", enableprodnum," proposer:",ultrainio::name{proposer}," accountsize:",proposeminer.size(),"\n");
      for(auto minerinfo : proposeminer){
         ultrainio_assert( is_account( minerinfo.account ), "vote votecommittee account not exist");
         minerinfo.approve_num = 1;
         provided_proposer  provideapprve(proposer,now(),0);
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
               _pendingminer.modify( pendingiter, 0, [&]( auto& p ) {
                  auto itr = std::find( p.provided_approvals.begin(), p.provided_approvals.end(), provideapprve );
                  if(itr != p.provided_approvals.end())
                  {
                     if((itr->resource_index == (uint64_t)curproposeresnum) && ((provideapprve.last_vote_time - itr->last_vote_time) < seconds_per_halfhour)){
                           print("\nvoteaccount proposer already voted proposer :",ultrainio::name{proposer}," current_time:",provideapprve.last_vote_time," last_vote_time:",itr->last_vote_time);
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
                  _pendingminer.modify( pendingiter, 0, [&]( auto& p ) {
                     p.provided_approvals.clear();
                     p.proposal_miner.clear();
                  });
                  _pendingminer.erase( pendingiter );
               }
            }else{
               _pendingminer.modify( pendingiter, 0, [&]( auto& p ) {
                  p.proposal_miner.push_back(minerinfo);
                  provideapprve.resource_index = p.proposal_miner.size() - 1;
                  p.provided_approvals.push_back(provideapprve);
               });
            }
         }else{
            _pendingminer.emplace( _self, [&]( auto& p ) {
               p.owner = minerinfo.account;
               p.provided_approvals.push_back(provideapprve);
               p.proposal_miner.push_back(minerinfo);
            });
         }
      }
   }
   void system_contract::getKeydata(const std::string& pubkey,std::array<char,33> & data){
      auto const getHexvalue = [](const char ch)->int{
         if(ch >= '0' && ch <= '9')
            return (ch-'0');
         if(ch >= 'a' && ch <= 'f')
            return ((ch-'a')+10);
      };
      char  keydata[67];
      memset(keydata,0,sizeof(keydata));
      frombase58_recover_key(pubkey.c_str(), keydata, 66);
      unsigned int j = 0;
      for ( uint32_t i=0; i < strlen(keydata); i++ ){
         if(i%2 == 1)
         {
            data[j] = (getHexvalue(keydata[i-1])*16 + getHexvalue(keydata[i])) & 0xff;
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
      size_t size = action_data_size();
      char* buffer = (char*)alloca(size);
      read_action_data( buffer, size );
      account_name proposer;
      vector<proposeaccount_info> proposeaccount;
      datastream<const char*> ds( buffer, size );
      ds >> proposer >> proposeaccount;
      ultrainio_assert( proposeaccount.size() > 0 && proposeaccount.size() <= 10, "The number of proposeaccount changes changes not correct" );
      require_auth( proposer );
      auto propos = _producers.find( proposer );
      ultrainio_assert( propos != _producers.end() && propos->is_enabled && propos->is_on_master_chain(), "enabled producer not found this proposer" );
      checkvotefrequency( propos );
      uint32_t  enableprodnum = 0;
      for(auto itr = _producers.begin(); itr != _producers.end(); ++itr){
         if(itr->is_enabled && itr->is_on_master_chain())
            ++enableprodnum;
      }
      print("voteaccount enableprodnum size:", enableprodnum," proposer:",ultrainio::name{proposer}," accountsize:",proposeaccount.size(),"\n");
      for(auto accinfo : proposeaccount){
         ultrainio_assert( !is_account( accinfo.account ), "vote create account already exist");
         ultrainio_assert( accinfo.owner_key.length() == 53, "vote owner public key should be of size 53" );
         ultrainio_assert( accinfo.active_key.length() == 53, "vote active public key should be of size 53" );
         accinfo.approve_num = 1;
         provided_proposer  provideapprve(proposer,now(),0);
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
               _pendingaccount.modify( pendingiter, 0, [&]( auto& p ) {
                  auto itr = std::find( p.provided_approvals.begin(), p.provided_approvals.end(), provideapprve );
                  if(itr != p.provided_approvals.end())
                  {
                     if((itr->resource_index == (uint64_t)curproposeresnum) && ((provideapprve.last_vote_time - itr->last_vote_time) < seconds_per_halfhour)){
                           print("\nvoteaccount proposer already voted proposer :",ultrainio::name{proposer}," current_time:",provideapprve.last_vote_time," last_vote_time:",itr->last_vote_time);
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
                  _pendingaccount.modify( pendingiter, 0, [&]( auto& p ) {
                     p.provided_approvals.clear();
                     p.proposal_account.clear();
                  });
                  _pendingaccount.erase( pendingiter );
               }
            }else{
               _pendingaccount.modify( pendingiter, 0, [&]( auto& p ) {
                  p.proposal_account.push_back(accinfo);
                  provideapprve.resource_index = p.proposal_account.size() - 1;
                  p.provided_approvals.push_back(provideapprve);
               });
            }
         }else{
            _pendingaccount.emplace( _self, [&]( auto& p ) {
               p.owner = accinfo.account;
               p.provided_approvals.push_back(provideapprve);
               p.proposal_account.push_back(accinfo);
            });
         }
      }
   }

void system_contract::voteresourcelease() {
      size_t size = action_data_size();
      char* buffer = (char*)alloca(size);
      read_action_data( buffer, size );
      account_name proposer;

      vector<proposeresource_info> proposeresource;
      datastream<const char*> ds( buffer, size );
      ds >> proposer >> proposeresource;
      ultrainio_assert( proposeresource.size() > 0 && proposeresource.size() <= 10, "The number of proposeresource changes not correct" );
      require_auth( proposer );
      auto propos = _producers.find( proposer );
      ultrainio_assert( propos != _producers.end() && propos->is_enabled && propos->is_on_master_chain(), "enabled producer not found this proposer" );
      checkvotefrequency( propos );
      uint32_t  enableprodnum = 0;
      for(auto itr = _producers.begin(); itr != _producers.end(); ++itr){
         if(itr->is_enabled && itr->is_on_master_chain())
            ++enableprodnum;
      }
      print("voteresourcelease enableprodnum size:", enableprodnum," proposer:",ultrainio::name{proposer}," proposeresource_info size:",proposeresource.size(),"\n");
      for(auto resinfo : proposeresource){
         ultrainio_assert( is_account( resinfo.account ), "vote resoucelease account not exist");
         resinfo.approve_num = 1;
         provided_proposer  provideapprve(proposer,now(),0);
         auto pendingiter = _pendingres.find( resinfo.account );
         if ( pendingiter != _pendingres.end() ) {
            int32_t curproposeresnum = -1;
            uint32_t proposeaccountsize = (*pendingiter).proposal_resource.size();
            for(uint32_t i = 0;i < proposeaccountsize;i++)
            {
               if((resinfo.account == (*pendingiter).proposal_resource[i].account) &&
                  (resinfo.lease_num == (*pendingiter).proposal_resource[i].lease_num)  &&
                  (resinfo.end_time == (*pendingiter).proposal_resource[i].end_time)  &&
                  (resinfo.location == (*pendingiter).proposal_resource[i].location) ){
                  curproposeresnum = (int32_t)i;
                  break;
               }
            }
            if(curproposeresnum >= 0){
               _pendingres.modify( pendingiter, 0, [&]( auto& p ) {
                  auto itr = std::find( p.provided_approvals.begin(), p.provided_approvals.end(), provideapprve );
                  if(itr != p.provided_approvals.end())
                  {
                     if((itr->resource_index == (uint64_t)curproposeresnum) && ((provideapprve.last_vote_time - itr->last_vote_time) < seconds_per_halfhour)){
                           print("\nvoteresourcelease proposer already voted proposer :",ultrainio::name{proposer}," current_time:",provideapprve.last_vote_time," last_vote_time:",itr->last_vote_time);
                           ultrainio_assert( false, "proposer already voted" );
                     }
                     p.provided_approvals.erase(itr);
                  }
                  p.proposal_resource[(uint32_t)curproposeresnum].approve_num++;
                  provideapprve.resource_index = (uint64_t)curproposeresnum;
                  p.provided_approvals.push_back(provideapprve);
               });

               if((*pendingiter).proposal_resource[(uint32_t)curproposeresnum].approve_num >= ceil((double)enableprodnum*2/3)){
                  syncresource(resinfo.account, resinfo.lease_num, resinfo.end_time);
                  _pendingres.modify( pendingiter, 0, [&]( auto& p ) {
                     p.provided_approvals.clear();
                     p.proposal_resource.clear();
                  });
                  _pendingres.erase( pendingiter );
               }
            }else{
               _pendingres.modify( pendingiter, 0, [&]( auto& p ) {
                  p.proposal_resource.push_back(resinfo);
                  provideapprve.resource_index = p.proposal_resource.size() - 1;
                  p.provided_approvals.push_back(provideapprve);
               });
            }
         }else{
            _pendingres.emplace( _self, [&]( auto& p ) {
               p.owner = resinfo.account;
               p.provided_approvals.push_back(provideapprve);
               p.proposal_resource.push_back(resinfo);
            });
         }
      }
   }

   void system_contract::cleanvotetable(){
      time curtime = now();
      if(_gstate.last_vote_expiretime == 0)
         _gstate.last_vote_expiretime = now();
      if(_gstate.last_vote_expiretime < curtime && (curtime - _gstate.last_vote_expiretime) >= seconds_per_halfhour){
         _gstate.last_vote_expiretime = curtime;
         uint64_t starttime = current_time();
         //clean vote pendingminer expire
         for(auto mineriter = _pendingminer.begin(); mineriter != _pendingminer.end(); ){
            _pendingminer.modify( mineriter, 0, [&]( auto& p ) {
               for(auto itr = p.provided_approvals.begin(); itr != p.provided_approvals.end();){
                  if((curtime - itr->last_vote_time) > seconds_per_halfhour){
                     itr = p.provided_approvals.erase(itr);
                  }else
                     ++itr;
               }
            });
            print("cleanvotetable _pendingminer name:",name{mineriter->owner}, " approversize::",mineriter->provided_approvals.size()," curtime:",curtime);
            if(mineriter->provided_approvals.size() == 0){
               mineriter = _pendingminer.erase(mineriter);
            }else
               ++mineriter;
         }

         //clean vote _pendingaccount expire
         for(auto acciter = _pendingaccount.begin(); acciter != _pendingaccount.end(); ){
            _pendingaccount.modify( acciter, 0, [&]( auto& p ) {
               for(auto itr = p.provided_approvals.begin(); itr != p.provided_approvals.end();){
                  if((curtime - itr->last_vote_time) > seconds_per_halfhour){
                     itr = p.provided_approvals.erase(itr);
                  }else
                     ++itr;
               }
            });
            print("cleanvotetable _pendingaccount name:",name{acciter->owner}, " approversize::",acciter->provided_approvals.size()," curtime:",curtime);
            if(acciter->provided_approvals.size() == 0){
               acciter = _pendingaccount.erase(acciter);
            }else
               ++acciter;
         }

         //clean vote _pendingres expire
         for(auto resiter = _pendingres.begin(); resiter != _pendingres.end(); ){
            _pendingres.modify( resiter, 0, [&]( auto& p ) {
               for(auto itr = p.provided_approvals.begin(); itr != p.provided_approvals.end();){
                  if((curtime - itr->last_vote_time) > seconds_per_halfhour){
                     itr = p.provided_approvals.erase(itr);
                  }else
                     ++itr;
               }
            });
            print("cleanvotetable _pendingres name:",name{resiter->owner}, " approversize::",resiter->provided_approvals.size()," curtime:",curtime);
            if(resiter->provided_approvals.size() == 0){
               resiter = _pendingres.erase(resiter);
            }else
               ++resiter;
         }
         uint64_t endtime = current_time();
         print("cleanvotetable expend time:",(endtime - starttime));
      }
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

      if( creator != _self ) {
         auto tmp = newact >> 4;
         bool has_dot = false;

         for( uint32_t i = 0; i < 12; ++i ) {
           has_dot |= !(tmp & 0x1f);
           tmp >>= 5;
         }
         if( has_dot ) { // or is less than 12 characters
            auto suffix = ultrainio::name_suffix(newact);
            if( suffix == newact ) {
               name_bid_table bids(_self,_self);
               auto current = bids.find( newact );
               ultrainio_assert( current != bids.end(), "no active bid for name" );
               ultrainio_assert( current->high_bidder == creator, "only highest bidder can claim" );
               ultrainio_assert( current->high_bid < 0, "auction for name is not closed yet" );
               bids.erase( current );
            } else {
               ultrainio_assert( creator == suffix, "only suffix may create this account" );
            }
         }
         INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {creator,N(active)},
            { creator, N(utrio.fee), asset(2000), std::string("create account") } );
      }

      //user_resources_table  userres( _self, newact);

      //userres.emplace( newact, [&]( auto& res ) {
      //  res.owner = newact;
      //});

      set_resource_limits( newact, 0, 0, 0 );
   }
   void native::updateauth( account_name     account/*,
                  permission_name  permission,
                  permission_name  parent,
                  const authority& data*/ ){
         INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {account,N(active)},
            { account, N(utrio.fee), asset(10000), std::string("update auth") } );
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
     (setram)(setblockreward)(setmincommittee)(setparams)(setpriv)(rmvproducer)(bidname)(votecommittee)(voteaccount)(voteresourcelease)
     // delegate_bandwidth.cpp
     (delegatecons)(undelegatecons)(refundcons)(resourcelease)(recycleresource)
     // voting.cpp
     (regproducer)(unregprod)
     // producer_pay.cpp
     (onblock)(claimrewards)
     // scheduler.cpp
     (regchaintype)(regsubchain)(acceptheader)(clearchain)(empoweruser)(reportsubchainhash)(setsched)
)
