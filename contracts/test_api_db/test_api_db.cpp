/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainiolib/ultrainio.hpp>
#include "../test_api/test_api.hpp"

#include "test_db.cpp"

extern "C" {
   void apply( uint64_t receiver, uint64_t code, uint64_t actH, uint64_t actL ) {
      action_name action(actH, actL);

      require_auth(code);
      WASM_TEST_HANDLER_EX(test_db, primary_i64_general);
      WASM_TEST_HANDLER_EX(test_db, primary_i64_lowerbound);
      WASM_TEST_HANDLER_EX(test_db, primary_i64_upperbound);
      WASM_TEST_HANDLER_EX(test_db, idx64_general);
      WASM_TEST_HANDLER_EX(test_db, idx64_lowerbound);
      WASM_TEST_HANDLER_EX(test_db, idx64_upperbound);
      WASM_TEST_HANDLER_EX(test_db, test_invalid_access);
      WASM_TEST_HANDLER_EX(test_db, idx_double_nan_create_fail);
      WASM_TEST_HANDLER_EX(test_db, idx_double_nan_modify_fail);
      WASM_TEST_HANDLER_EX(test_db, idx_double_nan_lookup_fail);
      WASM_TEST_HANDLER_EX(test_db, misaligned_secondary_key256_tests);

      //unhandled test call
      ultrainio_assert(false, "Unknown Test");
   }

}
