#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/print.hpp>
using namespace ultrainio;

class payloadless : public ultrainio::contract {
  public:
      using contract::contract;

      void doit() {
         print( "Im a payloadless action" );
      }
};

ULTRAINIO_ABI( payloadless, (doit) )
