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
    struct TransferActionParam {
        public:
            account_name from;
            account_name to;
            asset        val;
            std::string  memo;

            ULTRAINLIB_SERIALIZE(TransferActionParam, (from)(to)(val)(memo))
    };

   void system_contract::synctransfer( std::string transaction_mroot, uint32_t block_number, std::string tx_id ) {
          merkle_proof mklp = merkle_proof::get_merkle_proof(block_number, tx_id);
          bool r = mklp.verify(transaction_mroot);
          ultrainio_assert(r, "syncTransfer failed: verify merkle proof failed.");

          transaction tx = mklp.recover_transaction();
          ultrainio_assert(tx.actions.size() > 0, "no context related actions contains in this transaction.");

          for (const auto& act : tx.actions) {
              if (act.account == N(utrio.token) && act.name == NEX(transfer)) {
                 TransferActionParam tap = unpack<TransferActionParam>(act.data);
                 ultrainio_assert(tap.to == N(utrio.bank), " account to is not utrio.bank");
                 INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {_self, N(active)},
                    { N(utrio.bank), tap.from, tap.val, std::string("sync transfer") } );   
                 print("from : ", name{tap.from});
                 print(", to: ", name{tap.to});
                 print(", asset: "); tap.val.print();
                 print(", memo: "); print(tap.memo);
                 return;
              }
          }

          print("No transfer action found in transaction.");
      }
} //namespace ultrainiosystem
