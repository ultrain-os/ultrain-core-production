/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include "ultrainio.system.hpp"

#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/print.hpp>
#include <ultrainiolib/multi_index.hpp>
#include <ultrainiolib/transaction.hpp>
#include <ultrainio.token/ultrainio.token.hpp>
#include <ultrainiolib/merkle_proof.hpp>
#include <ultrainiolib/asset.hpp>
#include <ultrainiolib/action.hpp>

namespace ultrainiosystem {
    using namespace ultrainio;
    using ultrainio::asset;
    using ultrainio::bytes;
    using ultrainio::print;
    struct chain_balance {
            name     chain_name;
            asset    balance;
            uint64_t primary_key()const { return chain_name; }

            ULTRAINLIB_SERIALIZE(chain_balance, (chain_name)(balance))
    };
    typedef ultrainio::multi_index<N(chainbalance), chain_balance> chainbalance;
    struct TransferActionParam {
        public:
            account_name from;
            account_name to;
            asset        val;
            std::string  memo;

            ULTRAINLIB_SERIALIZE(TransferActionParam, (from)(to)(val)(memo))
    };

    struct EmpowerUserParam {
        account_name    user;
        name            chain_name;
        std::string     owner_pk;
        std::string     active_pk;
        bool            updateable;
        ULTRAINLIB_SERIALIZE(EmpowerUserParam, (user)(chain_name)(owner_pk)(active_pk)(updateable))
    };

    struct ResleaseActionParam {
        public:
            account_name from;
            account_name receiver;
            uint64_t combosize;
            uint64_t days;
            name location;
            ULTRAINLIB_SERIALIZE(ResleaseActionParam, (from)(receiver)(combosize)(days)(location))
    };
   static std::string checksum256_to_string( const uint8_t* d, uint32_t s )
   {
      std::string r;
      const char* to_hex="0123456789abcdef";
      uint8_t* c = (uint8_t*)d;
      for( uint32_t i = 0; i < s; ++i )
            (r += to_hex[(c[i]>>4)]) += to_hex[(c[i] &0x0f)];
      return r;
   }
   void system_contract::synclwctx( name chain_name, uint32_t block_number, std::vector<std::string> merkle_proofs, std::vector<char> tx_bytes ) {
      require_auth(current_sender());
      block_table subchain_block_tbl(_self, chain_name);
      auto block_ite = subchain_block_tbl.find(block_number);
      ultrainio_assert(block_ite != subchain_block_tbl.end(), "can not find this subchain block_number");
      merkle_proof mklp;
      mklp.proofs = merkle_proofs;
      mklp.tx_bytes = tx_bytes;
      std::string merkle_mroot = checksum256_to_string( block_ite->transaction_mroot.hash, sizeof(block_ite->transaction_mroot.hash));
      bool r = mklp.verify(merkle_mroot);
      ultrainio_assert(r, "syncTransfer failed: verify merkle proof failed.");

      stateful_transaction tx = mklp.recover_transaction();
      ultrainio_assert(tx.status == stateful_transaction::executed, "this transaction is not executed.");
      ultrainio_assert(!tx.tx_id.empty(), "this transaction tx_id is empty.");
      ultrainio_assert(block_ite->trx_ids.count(tx.tx_id) != 1, "syncTransfer failed: current transfer transaction already synchronized");
      subchain_block_tbl.modify( block_ite, [&]( block_header_digest& header ) {
         header.trx_ids.insert(tx.tx_id);
      });
      ultrainio_assert(tx.actions.size() > 0, "no context related actions contains in this transaction.");
      execactions( tx.actions, chain_name );
   }

   void system_contract::execactions( const vector<action> & actios, name chain_name){
      uint32_t  exec_succ = 0;
      for (const auto& act : actios) {
         if (act.account == N(utrio.token) && act.name == NEX(transfer)) {
            TransferActionParam tap = unpack<TransferActionParam>(act.data);
            if (tap.val.symbol != symbol_type(CORE_SYMBOL))
                continue;
            asset cur_tokens = ultrainio::token(N(utrio.token)).get_balance( N(utrio.bank),symbol_type(CORE_SYMBOL).name());
            if(tap.to != N(utrio.bank) ||
              string_to_name(tap.memo.c_str()) != _gstate.chain_name
              )
               continue;

            if( cur_tokens < tap.val && !_gstate.is_master_chain()){ //if master chain no issue tokens
               INLINE_ACTION_SENDER(ultrainio::token, issue)( N(utrio.token), {{N(ultrainio),N(active)}},
               {N(utrio.bank),(tap.val - cur_tokens), std::string("issue tokens for subchain utrio.bank")} );
            } else if(_gstate.is_master_chain()){
               chainbalance  chainbalan(N(utrio.bank), N(utrio.bank));
               auto it_chain = chainbalan.find( chain_name );
               if(cur_tokens < tap.val || it_chain == chainbalan.end() || it_chain->balance < tap.val){
                  print(" ERROR The utrio.bank of the masterchain should never be smaller than the amount of money to be transferred");
                  continue;
               }
            }

            exec_succ++;
            INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {N(utrio.bank), N(active)},
               { N(utrio.bank), tap.from, tap.val, name{chain_name}.to_string() } );
            print("synctransfer  from : ", name{tap.from}, ", to: ", name{tap.to});
            print(", asset: "); tap.val.print();
            print(", memo: ", tap.memo, "\n");
         } else if(act.account == N(ultrainio) && act.name == NEX(empoweruser)) {
             EmpowerUserParam eup = unpack<EmpowerUserParam>(act.data);
             if(eup.chain_name != _gstate.chain_name)
                continue;
             exec_succ++;
             proposeaccount_info new_acc;
             new_acc.account = eup.user;
             new_acc.owner_key = eup.owner_pk;
             new_acc.active_key = eup.active_pk;
             new_acc.updateable = eup.updateable;
             new_acc.location = name{N(ultrainio)};
             add_subchain_account(new_acc);
             print("sync user", name{eup.user}, "\n");
         }else if (act.account == N(ultrainio) && act.name == NEX(resourcelease)) {
            ResleaseActionParam rap = unpack<ResleaseActionParam>(act.data);
            if(rap.location != _gstate.chain_name)
                continue;
            exec_succ++;
            INLINE_ACTION_SENDER(ultrainiosystem::system_contract, resourcelease)( N(ultrainio), {N(ultrainio), N(active)},
                  { N(ultrainio), rap.receiver, rap.combosize, rap.days, master_chain_name} );
         }
      }
      ultrainio_assert(exec_succ > 0, "The current transaction has no execution");
   }
} //namespace ultrainiosystem
