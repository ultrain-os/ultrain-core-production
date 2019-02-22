#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/transaction.hpp>
#include <ultrainiolib/merkle_proof.hpp>

using namespace ultrainio;

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
};

ULTRAINIO_ABI( merkle_proof_contract, (verify)(merkleProof) )
