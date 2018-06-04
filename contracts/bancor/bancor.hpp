/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/token.hpp>
#include <ultrainiolib/reflect.hpp>
#include <ultrainiolib/generic_currency.hpp>

#include <bancor/converter.hpp>
#include <currency/currency.hpp>

namespace bancor {
   typedef ultrainio::generic_currency< ultrainio::token<N(other),S(4,OTHER)> >  other_currency;
   typedef ultrainio::generic_currency< ultrainio::token<N(bancor),S(4,RELAY)> > relay_currency;
   typedef ultrainio::generic_currency< ultrainio::token<N(currency),S(4,CUR)> > cur_currency;

   typedef converter<relay_currency, other_currency, cur_currency > example_converter;
} /// bancor

