/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainiolib/print.hpp> /// defines transfer struct (abi)

extern "C" {

    /// The apply method just prints forever
    void apply( uint64_t, uint64_t, uint64_t ) {
       int idx = 0;
       while(true) {
          ultrainio::print(idx++);
       }
    }
}
