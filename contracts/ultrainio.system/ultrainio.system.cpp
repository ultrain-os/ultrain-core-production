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
    _reslease_tbl( _self,_self ) {
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
   void system_contract::updateactiveminers(const std::vector<ultrainio::proposeminer_info>&  miners ) {
      for(auto const& miner:miners){
         print("updateactiveminers   proposerminer:",name{miner.account}," adddel:",miner.adddel_miner);
         if(miner.adddel_miner){
            auto prod = _producers.find( miner.account );
            if(prod != _producers.end() && prod->is_enabled){
               print("\nupdateactiveminers curproducer existed  proposerminer:",name{(*prod).owner});
               continue;
            }
            INLINE_ACTION_SENDER(ultrainiosystem::system_contract, regproducer)( N(ultrainio), {miner.account, N(active)},
               { miner.account, miner.public_key, miner.url, miner.location} );
               INLINE_ACTION_SENDER(ultrainiosystem::system_contract, delegatecons)( N(ultrainio), {N(utrio.stake), N(active)},
               { N(utrio.stake),miner.account,asset(consweight_per_subaccount)} ); 
         }else{
            auto prod = _producers.find( miner.account );
            if(prod == _producers.end())// || !prod->is_enabled
            {
               print("updateactiveminers curproducer not found  proposerminer:",ultrainio::name{(*prod).owner}," (*produceriter).total_cons_staked:",(*prod).total_cons_staked);
               continue;
            }
            _producers.modify( prod, 0, [&](auto& p) {
                  p.deactivate();
                  p.is_enabled =false;
               });
            print("updateactiveminers   del proposerminer:",ultrainio::name{(*prod).owner}," (*produceriter).total_cons_staked:",(*prod).total_cons_staked);
            if((*prod).total_cons_staked > 0){
               INLINE_ACTION_SENDER(ultrainiosystem::system_contract, undelegatecons)( N(ultrainio), {N(utrio.stake), N(active)},
                  { N(utrio.stake),(*prod).owner,asset((*prod).total_cons_staked)} );
            }
         }
      }
   }
   void system_contract::votecommittee() {
      constexpr size_t max_stack_buffer_size = 4096;
      size_t size = action_data_size();
      char* buffer = (char*)( max_stack_buffer_size < size ? malloc(size) : alloca(size) );
      read_action_data( buffer, size );
      account_name proposer;
      vector<proposeminer_info> proposeminer;
      datastream<const char*> ds( buffer, size );
      ds >> proposer >> proposeminer;
      uint32_t proposeminersize = proposeminer.size();
      ultrainio_assert( proposeminer.size() == 1, "The number of proposeminer changes cannot exceed one" );
      require_auth( proposer );
      auto propos = _producers.find( proposer );
      ultrainio_assert( propos != _producers.end() && propos->is_enabled, "enabled producer not found this proposer" );
      auto const comp = [this](const proposeminer_info &a, const proposeminer_info &b){
         if (a.account < b.account)
               return true;
         return false;
      };
      std::sort(proposeminer.begin(), proposeminer.end(),comp);

      bool nofindminerflg = false;
      uint32_t  curactiveminer = 0;
      for(auto itr = _producers.begin(); itr != _producers.end(); ++itr){
         if(itr->is_enabled)
            ++curactiveminer;
      }
      print("votecommittee curactiveminer size:", curactiveminer," proposer:",ultrainio::name{proposer}," minersize:",proposeminer.size()," changeproducerflg:",curactiveminer*2/3>proposeminer.size(),"\n");

      for(auto miner:proposeminer){
         print("get votecommittee proposeminer vector:", ultrainio::name{miner.account}, miner.public_key, miner.adddel_miner);
      }
      pendingminers pendingminer( _self, _self );
      provided_proposer  provideapprve(proposer,current_time());
      for(auto pendingiter = pendingminer.begin(); pendingiter != pendingminer.end(); pendingiter++)
      {
         nofindminerflg = false;
         if((*pendingiter).proposal_miner.size() == proposeminersize){
            for(uint32_t i = 0;i < proposeminersize;i++)
            {
               if((proposeminer[i].account != (*pendingiter).proposal_miner[i].account) ||
                 (proposeminer[i].public_key != (*pendingiter).proposal_miner[i].public_key)  ||
                 (proposeminer[i].location != (*pendingiter).proposal_miner[i].location)  ||
                 (proposeminer[i].adddel_miner != (*pendingiter).proposal_miner[i].adddel_miner)){
                  nofindminerflg = true;
                  break;
               }
            }
            if(!nofindminerflg){
               auto itr = std::find( (*pendingiter).provided_approvals.begin(), (*pendingiter).provided_approvals.end(), provideapprve );
               if(itr != (*pendingiter).provided_approvals.end())
               {
                  if((provideapprve.last_vote_time - itr->last_vote_time) < useconds_per_halfhour){
                     print("\nvotecommittee nofindminerflg proposer already voted proposer :",ultrainio::name{proposer}," provideapprve.last_vote_time:",provideapprve.last_vote_time," itr->last_vote_time:",itr->last_vote_time);
                     ultrainio_assert( false, "proposer already voted" );
                  }
                  pendingminer.modify( *pendingiter, 0, [&]( auto& p ) {
                     p.provided_approvals.erase(itr);
                  });
               }
               pendingminer.modify( *pendingiter, 0, [&]( auto& p ) {
                  p.provided_approvals.push_back(provideapprve);
               });
               print("\nvotecommittee nofindminerflg proposer:",ultrainio::name{proposer}," proposernumberflg:",(*pendingiter).provided_approvals.size() >= curactiveminer*2/3," provideapprve.last_vote_time:",provideapprve.last_vote_time," curproposernum:",(*pendingiter).provided_approvals.size());
               if((*pendingiter).provided_approvals.size() >= curactiveminer*2/3){
                  updateactiveminers(proposeminer);
                  pendingminer.modify( *pendingiter, 0, [&]( auto& p ) {
                     p.provided_approvals.clear();
                     p.proposal_miner.clear();
                  });
                  const auto& pending = pendingminer.get( (*pendingiter).proposer, "the current proposal cannot be found" );
                  pendingminer.erase( pending );
               }
               return;
            }
         }
      }
      auto prod = pendingminer.find( proposer );
      if ( prod != pendingminer.end() ) {
         pendingminer.modify( prod, _self, [&]( auto& p ) {
            p.provided_approvals.clear();
            p.provided_approvals.push_back(provideapprve);
            p.proposal_miner.clear();
            for(auto miner:proposeminer)
               p.proposal_miner.push_back(miner);
         });
      } else {
         pendingminer.emplace( _self, [&]( auto& p ) {
            p.proposer = proposer;
            p.provided_approvals.push_back(provideapprve);
            for(auto miner:proposeminer)
               p.proposal_miner.push_back(miner);
         });
      }
      print("votecommittee pushback  pendingminer");
   }
   void system_contract::getKeydata(const std::string& pubkey,std::array<char,33> & data){
      auto const getHexvalue = [](const char ch)->int{
         if(ch >= '0' && ch <= '9')
            return (ch-'0');
         if(ch >= 'a' && ch <= 'f')
            return ((ch-'a')+10);
      };
      char  keydata[512];
      memset(keydata,0,sizeof(keydata));
      frombase58_recover_key(pubkey.c_str(), keydata,0);
      unsigned int j = 0;
      for ( uint32_t i=0; i < strlen(keydata); i++ ){
         if(i%2 == 1)
         {
            data[j] = (getHexvalue(keydata[i-1])*16 + getHexvalue(keydata[i])) & 0xff;
            j++;
         }
         //print("updateactiveaccounts publickey i:" , i," data:",keydata[i]," value:",getHexvalue(keydata[i])," j:",j," jdata:",data[j]);
      }
   }
   void system_contract::add_subchain_account(const std::vector<ultrainio::proposeaccount_info>&  accounts ) {
      for(auto const& newacc:accounts){
         ultrainiosystem::key_weight ownerkeyweight;
         std::array<char,33> ownerdata;
         getKeydata(newacc.owner_key,ownerdata);
         ownerkeyweight.key.data = ownerdata;
         ownerkeyweight.key.type = 0;
         ultrainiosystem::authority    ownerkey  = { .threshold = 1, .keys = { ownerkeyweight }, .accounts = {}, .waits = {} };

         ultrainiosystem::key_weight activekeyweight;
         std::array<char,33> activedata;
         getKeydata(newacc.active_key,activedata);
         activekeyweight.key.data = activedata;
         activekeyweight.key.type = 0;
         ultrainiosystem::authority     activekey = { .threshold = 1, .keys = { activekeyweight }, .accounts = {}, .waits = {} };
         print("updateactiveaccounts proposerminer:",name{newacc.account}," ownerkey:",newacc.owner_key," ownerkey:",newacc.active_key);
         action(
            permission_level{ N(ultrainio), N(active) },
            N(ultrainio), NEX(newaccount),
            std::make_tuple(N(ultrainio),newacc.account, ownerkey, activekey, true)
         ).send();
      }
   }
void system_contract::voteaccount() {
      constexpr size_t max_stack_buffer_size = 4096;
      size_t size = action_data_size();
      char* buffer = (char*)( max_stack_buffer_size < size ? malloc(size) : alloca(size) );
      read_action_data( buffer, size );
      account_name proposer;
      vector<proposeaccount_info> proposeaccount;
      datastream<const char*> ds( buffer, size );
      ds >> proposer >> proposeaccount;
      uint32_t proposeaccountsize = proposeaccount.size();
      ultrainio_assert( proposeaccount.size() > 0, "The number of proposeaccount changes greater than zero" );
      require_auth( proposer );
      auto propos = _producers.find( proposer );
      ultrainio_assert( propos != _producers.end() && propos->is_enabled, "enabled producer not found this proposer" );
      auto const comp = [this](const proposeaccount_info &a, const proposeaccount_info &b){
         if (a.account < b.account)
               return true;
         return false;
      };
      std::sort(proposeaccount.begin(), proposeaccount.end(),comp);

      bool nofindaccountflg = false;
      uint32_t  curactiveaccount = 0;
      for(auto itr = _producers.begin(); itr != _producers.end(); ++itr){
         if(itr->is_enabled)
            ++curactiveaccount;
      }
      print("voteaccount curactiveproducers size:", curactiveaccount," proposer:",ultrainio::name{proposer}," accountsize:",proposeaccount.size()," changeproducerflg:",curactiveaccount*2/3>proposeaccount.size(),"\n");

      for(auto account:proposeaccount){
         print("get voteaccount proposeaccount vector:", ultrainio::name{account.account}, account.owner_key);
      }
      pendingaccounts pendingaccount( _self, _self );
      provided_proposer  provideapprve(proposer,current_time());
      for(auto pendingiter = pendingaccount.begin(); pendingiter != pendingaccount.end(); pendingiter++)
      {
         nofindaccountflg = false;
         if((*pendingiter).proposal_account.size() == proposeaccountsize){
            for(uint32_t i = 0;i < proposeaccountsize;i++)
            {
               if((proposeaccount[i].account != (*pendingiter).proposal_account[i].account) ||
                 (proposeaccount[i].owner_key != (*pendingiter).proposal_account[i].owner_key)  ||
                 (proposeaccount[i].active_key != (*pendingiter).proposal_account[i].active_key) ){
                  nofindaccountflg = true;
                  break;
               }
            }
            if(!nofindaccountflg){
               auto itr = std::find( (*pendingiter).provided_approvals.begin(), (*pendingiter).provided_approvals.end(), provideapprve );
               if(itr != (*pendingiter).provided_approvals.end())
               {
                  if((provideapprve.last_vote_time - itr->last_vote_time) < useconds_per_halfhour){
                     print("\nvoteaccount nofindaccountflg proposer already voted proposer :",ultrainio::name{proposer}," provideapprve.last_vote_time:",provideapprve.last_vote_time," itr->last_vote_time:",itr->last_vote_time);
                     ultrainio_assert( false, "proposer already voted" );
                  }
                  pendingaccount.modify( *pendingiter, 0, [&]( auto& p ) {
                     p.provided_approvals.erase(itr);
                  });
               }
               pendingaccount.modify( *pendingiter, 0, [&]( auto& p ) {
                  p.provided_approvals.push_back(provideapprve);
               });
               print("\nvoteaccount nofindaccountflg proposer:",ultrainio::name{proposer}," proposernumberflg:",(*pendingiter).provided_approvals.size() >= curactiveaccount*2/3," provideapprve.last_vote_time:",provideapprve.last_vote_time," curproposernum:",(*pendingiter).provided_approvals.size());
               if((*pendingiter).provided_approvals.size() >= curactiveaccount*2/3){
                  add_subchain_account(proposeaccount);
                  pendingaccount.modify( *pendingiter, 0, [&]( auto& p ) {
                     p.provided_approvals.clear();
                     p.proposal_account.clear();
                  });
                  const auto& pending = pendingaccount.get( (*pendingiter).proposer, "the current proposal cannot be found" );
                  pendingaccount.erase( pending );
               }
               return;
            }
         }
      }
      auto prod = pendingaccount.find( proposer );
      if ( prod != pendingaccount.end() ) {
         pendingaccount.modify( prod, _self, [&]( auto& p ) {
            p.provided_approvals.clear();
            p.provided_approvals.push_back(provideapprve);
            p.proposal_account.clear();
            for(auto account:proposeaccount)
               p.proposal_account.push_back(account);
         });
      } else {
         pendingaccount.emplace( _self, [&]( auto& p ) {
            p.proposer = proposer;
            p.provided_approvals.push_back(provideapprve);
            for(auto account:proposeaccount)
               p.proposal_account.push_back(account);
         });
      }
      print("voteaccount pushback  pendingaccount");
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
      }

      //user_resources_table  userres( _self, newact);

      //userres.emplace( newact, [&]( auto& res ) {
      //  res.owner = newact;
      //});

      set_resource_limits( newact, 0, 0, 0 );
   }

} /// ultrainio.system


ULTRAINIO_ABI( ultrainiosystem::system_contract,
     // native.hpp (newaccount definition is actually in ultrainio.system.cpp)
     (newaccount)(updateauth)(deleteauth)(linkauth)(unlinkauth)(canceldelay)(onerror)
     // ultrainio.system.cpp
     (setram)(setparams)(setpriv)(rmvproducer)(bidname)(votecommittee)(voteaccount)
     // delegate_bandwidth.cpp
     (delegatecons)(undelegatecons)(refundcons)(resourcelease)
     // voting.cpp
     (regproducer)(unregprod)
     // producer_pay.cpp
     (onblock)(claimrewards)
     // scheduler.cpp
     (regsubchain)(acceptheader)(clearchain)(empoweruser)
)
