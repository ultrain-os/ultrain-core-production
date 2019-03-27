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
   static std::string checksum256_to_string( const uint8_t* d, uint32_t s )
   {
      std::string r;
      const char* to_hex="0123456789abcdef";
      uint8_t* c = (uint8_t*)d;
      for( uint32_t i = 0; i < s; ++i )
            (r += to_hex[(c[i]>>4)]) += to_hex[(c[i] &0x0f)];
      return r;
   }
   void system_contract::synctransfer( name chain_name, uint32_t block_number, std::vector<std::string> merkle_proofs, std::vector<char> tx_bytes ) {
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
      checksum256 txn_hash;
      sha256((&tx_bytes[0]), tx_bytes.size(), &txn_hash);
      ultrainio_assert(block_ite->trx_hashs.count(txn_hash) != 1, "syncTransfer failed: current transfer transaction already synchronized");
      subchain_block_tbl.modify( block_ite, [&]( block_header_digest& header ) {
         header.trx_hashs.insert(txn_hash);
      });
      ultrainio_assert(tx.actions.size() > 0, "no context related actions contains in this transaction.");

      for (const auto& act : tx.actions) {
         if (act.account == N(utrio.token) && act.name == NEX(transfer)) {
            TransferActionParam tap = unpack<TransferActionParam>(act.data);
            ultrainio_assert(tap.to == N(utrio.bank), " account to is not utrio.bank");
            ultrainio_assert(string_to_name(tap.memo.c_str()) == _gstate.chain_name, "The synchronized chain is not correct");
            asset cur_tokens = ultrainio::token(N(utrio.token)).get_balance( N(utrio.bank),symbol_type(CORE_SYMBOL).name());
            ultrainio_assert( cur_tokens >= tap.val, " utrio.bank account insufficient funds" );
            INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {N(utrio.bank), N(active)},
               { N(utrio.bank), tap.from, tap.val, std::string("sync transfer") } );
            print("synctransfer  from : ", name{tap.from});
            print(", to: ", name{tap.to});
            print(", asset: "); tap.val.print();
            print(", memo: "); print(tap.memo);
         }
      }
   }
} //namespace ultrainiosystem
