#pragma once
#include <ultrainiolib/privileged.hpp>

#include <vector>

namespace ultrainio {
   /**
    *  Defines both the order, account name, and signing keys of the active set of producers.
    */
   struct producer_schedule {
      uint32_t                     version;   ///< sequentially incrementing version number
      std::vector<producer_key>    producers;
   };
} /// namespace ultrainio
