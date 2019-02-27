#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/transaction.hpp>
#include <ultrainiolib/merkle_proof.hpp>
#include <ultrainiolib/asset.hpp>
#include <ultrainiolib/transaction.hpp>

using namespace ultrainio;

class TransferActionParam {
    public:
        account_name from;
        account_name to;
        asset        val;
        std::string  memo;

        ULTRAINLIB_SERIALIZE(TransferActionParam, (from)(to)(val)(memo))
};

class merkle_proof_contract : public ultrainio::contract {
  public:
      using contract::contract;

      /// @abi action
      void verify( std::string transaction_mroot, std::vector<std::string> merkle_proofs, vector<char> tx_bytes ) {
         merkle_proof mklp;
         mklp.proofs = merkle_proofs;
         mklp.tx_bytes = tx_bytes;

         bool r = mklp.verify(transaction_mroot);
         print_f("CPP::merkle_proof verify = %s", r ? "true" : "false");
      }
      /// @abi action
      void merkleProof( std::string transaction_mroot, uint32_t block_number, std::string tx_id) {
          merkle_proof mklp = merkle_proof::get_merkle_proof(block_number, tx_id);
          print("CPP::merkle_proof.proofs: ");
          for (auto& pf : mklp.proofs) {
              print(pf);print(",");
          }
          print("CPP::merkle_proof.tx_bytes: ");
          for (auto& tb : mklp.tx_bytes) {
              print(0xff & int(tb));print(",");
          }

          bool r = mklp.verify(transaction_mroot);
          print_f("CPP::merkle_proof verify = %", r ? "true" : "false");
      }

      /// @abi action
      void verifyTransfer( std::string transaction_mroot, uint32_t block_number, std::string tx_id ) {
          merkle_proof mklp = merkle_proof::get_merkle_proof(block_number, tx_id);
          bool r = mklp.verify(transaction_mroot);

          ultrainio_assert(r, "verifyTransfer failed: verify merkle proof failed.");

          transaction tx = mklp.recover_transaction();
          ultrainio_assert(tx.actions.size() > 0, "no context related actions contains in this transaction.");

          for (const auto& act : tx.actions) {
              if (act.account == N(utrio.token) && act.name == NEX(transfer)) {
                 TransferActionParam tap = unpack<TransferActionParam>(act.data);
                 // TODO: here you can check the 'transfer' action now.

                 print("from : ", name{tap.from});
                 print(", to: ", name{tap.to});
                 print(", asset: "); tap.val.print();
                 print(", memo: "); print(tap.memo);
                 return;
              }
          }

          print("No transfer action found in transaction.");
      }
};

ULTRAINIO_ABI( merkle_proof_contract, (verify)(merkleProof)(verifyTransfer) )
