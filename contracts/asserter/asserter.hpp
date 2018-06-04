/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include <ultrainiolib/ultrainio.hpp>

namespace asserter {
   struct assertdef {
      int8_t      condition;
      std::string message;

      ULTRAINLIB_SERIALIZE( assertdef, (condition)(message) )
   };
}
