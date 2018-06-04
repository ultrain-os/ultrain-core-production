#include <ultrainiolib/ultrainio.hpp>
using namespace ultrainio;

class hello : public ultrainio::contract {
  public:
      using contract::contract;

      /// @abi action 
      void hi( account_name user ) {
         print( "Hello, ", name{user} );
      }
};

ULTRAINIO_ABI( hello, (hi) )
