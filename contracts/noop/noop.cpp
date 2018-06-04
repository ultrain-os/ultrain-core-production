/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include <ultrainiolib/ultrainio.hpp>

namespace ultrainio {

   class noop: public contract {
      public:
         noop( account_name self ): contract( self ) { }
         void anyaction( account_name from,
                         const std::string& /*type*/,
                         const std::string& /*data*/ )
         {
            require_auth( from );
         }
   };

   ULTRAINIO_ABI( noop, ( anyaction ) )

} /// ultrainio     
