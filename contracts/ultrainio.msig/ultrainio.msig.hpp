#pragma once
#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/transaction.hpp>

namespace ultrainio {

   class multisig : public contract {
      public:
         multisig( account_name self ):contract(self){}

         void propose();
         void approve( account_name proposer, name proposal_name, permission_level level );
         void unapprove( account_name proposer, name proposal_name, permission_level level );
         void cancel( account_name proposer, name proposal_name, account_name canceler );
         void exec( account_name proposer, name proposal_name, account_name executer );
         void votecommittee();
      private:
         struct proposal {
            name                       proposal_name;
            vector<char>               packed_transaction;

            auto primary_key()const { return proposal_name.value; }
         };
         typedef ultrainio::multi_index<N(proposal),proposal> proposals;

         struct approvals_info {
            name                       proposal_name;
            vector<permission_level>   requested_approvals;
            vector<permission_level>   provided_approvals;

            auto primary_key()const { return proposal_name.value; }
         };
         typedef ultrainio::multi_index<N(approvals),approvals_info> approvals;

         struct pendingminer {
            uint64_t                   index = 0;
            vector<account_name>       proposal_miner;
            vector<account_name>       provided_approvals;
            auto primary_key()const { return index; }
         };
         typedef ultrainio::multi_index<N(pendingminer),pendingminer> pendingminers;
   };

} /// namespace ultrainio
