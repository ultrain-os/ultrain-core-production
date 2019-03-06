#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/privileged.h>

namespace ultrainio {

   class testinline : public contract {
      public:
         testinline( account_name self ):contract(self){}

         void reqauth( account_name from ) {
            require_auth( from );
         }

         void forward( account_name reqauth, account_name forward_code, account_name forward_auth ) {
            require_auth( reqauth );
            INLINE_ACTION_SENDER(testinline, reqauth)( forward_code, {forward_auth,N(active)}, {forward_auth} );
            //SEND_INLINE_ACTION( testinline(forward_code), reqauth, {forward_auth,N(active)}, {forward_auth} );
            //ultrainio::dispatch_inline<account_name>( N(forward_code), N(reqauth), {{forward_auth, N(active)}}, {forward_auth} );
         }
   };

} /// namespace ultrainio
