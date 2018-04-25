/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include <noop/noop.hpp>

namespace noop {
   extern "C" {
      /// The apply method implements the dispatch of events to this contract
      void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
         ultrainio::dispatch<noop, noop::anyaction>(code, action);
      }
   }
}
