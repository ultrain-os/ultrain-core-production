/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include <ultrainio.system/ultrainio.system.hpp> 

using namespace ultrainiosystem;

extern "C" {

    /// The apply method implements the dispatch of events to this contract
    void apply( uint64_t receiver, uint64_t code, uint64_t act ) {
       //print( ultrainio::name(code), "::", ultrainio::name(act) );
       ultrainiosystem::contract<N(ultrainio)>::apply( receiver, code, act );
    }
}
