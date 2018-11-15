#include <ultrainio.msig/ultrainio.msig.hpp>
#include <ultrainiolib/action.hpp>
#include <ultrainiolib/permission.hpp>
#include <ultrainiolib/print.hpp>
#include <ultrainiolib/types.hpp>
using ultrainio::action;
namespace ultrainio {

/*
propose function manually parses input data (instead of taking parsed arguments from dispatcher)
because parsing data in the dispatcher uses too much CPU in case if proposed transaction is big

If we use dispatcher the function signature should be:

void multisig::propose( account_name proposer,
                        name proposal_name,
                        vector<permission_level> requested,
                        transaction  trx)
*/

void multisig::propose() {
   constexpr size_t max_stack_buffer_size = 512;
   size_t size = action_data_size();
   char* buffer = (char*)( max_stack_buffer_size < size ? malloc(size) : alloca(size) );
   read_action_data( buffer, size );

   account_name proposer;
   name proposal_name;
   vector<permission_level> requested;
   transaction_header trx_header;

   datastream<const char*> ds( buffer, size );
   ds >> proposer >> proposal_name >> requested;

   size_t trx_pos = ds.tellp();
   ds >> trx_header;

   require_auth( proposer );
   ultrainio_assert( trx_header.expiration >= ultrainio::time_point_sec(now()), "transaction expired" );
   //ultrainio_assert( trx_header.actions.size() > 0, "transaction must have at least one action" );

   proposals proptable( _self, proposer );
   ultrainio_assert( proptable.find( proposal_name ) == proptable.end(), "proposal with the same name exists" );

   bytes packed_requested = pack(requested);
   auto res = ::check_transaction_authorization( buffer+trx_pos, size-trx_pos,
                                                 (const char*)0, 0,
                                                 packed_requested.data(), packed_requested.size()
                                               );
   ultrainio_assert( res > 0, "transaction authorization failed" );

   proptable.emplace( proposer, [&]( auto& prop ) {
      prop.proposal_name       = proposal_name;
      prop.packed_transaction  = bytes( buffer+trx_pos, buffer+size );
   });

   approvals apptable(  _self, proposer );
   apptable.emplace( proposer, [&]( auto& a ) {
      a.proposal_name       = proposal_name;
      a.requested_approvals = std::move(requested);
   });
}

void multisig::approve( account_name proposer, name proposal_name, permission_level level ) {
   require_auth( level );

   approvals apptable(  _self, proposer );
   auto& apps = apptable.get( proposal_name, "proposal not found" );

   auto itr = std::find( apps.requested_approvals.begin(), apps.requested_approvals.end(), level );
   ultrainio_assert( itr != apps.requested_approvals.end(), "approval is not on the list of requested approvals" );

   apptable.modify( apps, proposer, [&]( auto& a ) {
      a.provided_approvals.push_back( level );
      a.requested_approvals.erase( itr );
   });
}

void multisig::unapprove( account_name proposer, name proposal_name, permission_level level ) {
   require_auth( level );

   approvals apptable(  _self, proposer );
   auto& apps = apptable.get( proposal_name, "proposal not found" );
   auto itr = std::find( apps.provided_approvals.begin(), apps.provided_approvals.end(), level );
   ultrainio_assert( itr != apps.provided_approvals.end(), "no approval previously granted" );

   apptable.modify( apps, proposer, [&]( auto& a ) {
      a.requested_approvals.push_back(level);
      a.provided_approvals.erase(itr);
   });
}

void multisig::cancel( account_name proposer, name proposal_name, account_name canceler ) {
   require_auth( canceler );

   proposals proptable( _self, proposer );
   auto& prop = proptable.get( proposal_name, "proposal not found" );

   if( canceler != proposer ) {
      ultrainio_assert( unpack<transaction_header>( prop.packed_transaction ).expiration < ultrainio::time_point_sec(now()), "cannot cancel until expiration" );
   }

   approvals apptable(  _self, proposer );
   auto& apps = apptable.get( proposal_name, "proposal not found" );

   proptable.erase(prop);
   apptable.erase(apps);
}

void multisig::exec( account_name proposer, name proposal_name, account_name executer ) {
   require_auth( executer );

   proposals proptable( _self, proposer );
   auto& prop = proptable.get( proposal_name, "proposal not found" );

   approvals apptable(  _self, proposer );
   auto& apps = apptable.get( proposal_name, "proposal not found" );

   transaction_header trx_header;
   datastream<const char*> ds( prop.packed_transaction.data(), prop.packed_transaction.size() );
   ds >> trx_header;
   ultrainio_assert( trx_header.expiration >= ultrainio::time_point_sec(now()), "transaction expired" );

   bytes packed_provided_approvals = pack(apps.provided_approvals);
   auto res = ::check_transaction_authorization( prop.packed_transaction.data(), prop.packed_transaction.size(),
                                                 (const char*)0, 0,
                                                 packed_provided_approvals.data(), packed_provided_approvals.size()
                                               );
   ultrainio_assert( res > 0, "transaction authorization failed" );

   send_deferred( (uint128_t(proposer) << 64) | proposal_name, executer, prop.packed_transaction.data(), prop.packed_transaction.size() );

   proptable.erase(prop);
   apptable.erase(apps);
}

void multisig::votecommittee() {
   constexpr size_t max_stack_buffer_size = 4096;
   size_t size = action_data_size();
   char* buffer = (char*)( max_stack_buffer_size < size ? malloc(size) : alloca(size) );
   read_action_data( buffer, size );

   account_name proposer;
   vector<proposeminer_info> proposeminer;
   datastream<const char*> ds( buffer, size );
   ds >> proposer >> proposeminer;
   int proposeminersize = proposeminer.size();
   ultrainio_assert( proposeminersize > 0, "propose miner must greater than 0" );
   require_auth( proposer );

   auto const comp = [this](const proposeminer_info &a, const proposeminer_info &b){
      if (a.account < b.account)
            return true;        
      return false;
   };
   std::sort(proposeminer.begin(), proposeminer.end(),comp);

	// auto vector_iterator = std::unique(proposeminer.begin(),proposeminer.end());
   // ultrainio_assert( vector_iterator == proposeminer.end(), " receiver proposeminer  duplication is not allowed" );

   bool nofindminerflg = false;
   int  curactiveminer = 3;
   print("votecommittee proposer:",proposer," minersize:",proposeminer.size());
   for(auto miner:proposeminer){
      print("get votecommittee proposeminer vector:", ultrainio::name{miner.account}, miner.public_key, miner.url,"\n");
   }
   pendingminers pendingminer( _self, _self );
   for(auto pendingiter = pendingminer.begin(); pendingiter != pendingminer.end(); pendingiter++)
   {
      nofindminerflg = false;
      if((*pendingiter).proposal_miner.size() == proposeminersize){
         for(int i = 0;i < proposeminersize;i++)
         {
            if(proposeminer[i].account != (*pendingiter).proposal_miner[i]){
               nofindminerflg = true;
               break;
            }
         }
         if(!nofindminerflg){
            auto itr = std::find( (*pendingiter).provided_approvals.begin(), (*pendingiter).provided_approvals.end(), proposer );
            ultrainio_assert( itr == (*pendingiter).provided_approvals.end(), "proposer already voted" );
            pendingminer.modify( *pendingiter, 0, [&]( auto& p ) {
               p.provided_approvals.push_back(proposer);
            });
            print("votecommittee nofindminerflg proposer:",proposer," nofindminerflg:",nofindminerflg);
            if((*pendingiter).provided_approvals.size() >= curactiveminer*2/3){
               //to do  sendaction   //
               print("votecommittee sendaction proposer:",proposer," minersize:",proposeminer.size());
               action(
                  permission_level{ _self, N(active) },
                  N(ultrainio), NEX(updateactiveminers),
                  std::make_tuple(proposeminer)
               ).send();

            }
            print("votecommittee proposer:",proposer," minersize:",proposeminer.size());
            return;
         }
      }
   }
   pendingminer.emplace( _self, [&]( auto& p ) {
      p.index++;
      p.provided_approvals.push_back(proposer);
      p.proposal_miner.clear();
      for(auto miner:proposeminer)
         p.proposal_miner.push_back(miner.account);
   });
   print("votecommittee pushback  pendingminer");
}

} /// namespace ultrainio

ULTRAINIO_ABI( ultrainio::multisig, (propose)(approve)(unapprove)(cancel)(exec)(votecommittee) )
