/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include <ultrainiolib/ultrainio.hpp>

namespace asserter {
   struct PACKED(assertdef) {
      int8_t   condition;
      int8_t   message_length;
      char     message[];
   };
}
