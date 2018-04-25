#pragma once 
#include <ultrainiolib/varint.hpp>
#include <ultrainiolib/serialize.hpp>

namespace ultrainio {
   struct public_key {
      unsigned_int        type;
      std::array<char,33> data;

      ULTRAINLIB_SERIALIZE( public_key, (type)(data) )
   };
}
