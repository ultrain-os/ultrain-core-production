/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <fc/exception/exception.hpp>
#include <ultrainio/chain/exceptions.hpp>

#define ULTRAIN_DECLARE_INTERNAL_EXCEPTION( exc_name, seqnum, msg )  \
   FC_DECLARE_DERIVED_EXCEPTION(                                      \
      internal_ ## exc_name,                                          \
      ultrainio::chain::internal_exception,                            \
      3990000 + seqnum,                                               \
      msg                                                             \
      )

namespace ultrainio { namespace chain {

FC_DECLARE_DERIVED_EXCEPTION( internal_exception, ultrainio::chain::chain_exception, 3990000, "internal exception" )

ULTRAIN_DECLARE_INTERNAL_EXCEPTION( verify_auth_max_auth_exceeded, 1, "Exceeds max authority fan-out" )
ULTRAIN_DECLARE_INTERNAL_EXCEPTION( verify_auth_account_not_found, 2, "Auth account not found" )

} } // ultrainio::chain
