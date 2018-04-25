/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <skeleton.hpp>

/**
 *  The init() and apply() methods must have C calling convention so that the blockchain can lookup and
 *  call these methods.
 */
extern "C" {

    /// The apply method implements the dispatch of events to this contract
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
       ultrainio::print( "Hello World: ", ultrainio::name(code), "->", ultrainio::name(action), "\n" );
    }

} // extern "C"
