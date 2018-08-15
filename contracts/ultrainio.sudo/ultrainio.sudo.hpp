#pragma once

#include <ultrainiolib/ultrainio.hpp>

namespace ultrainio {

   class sudo : public contract {
      public:
         sudo( account_name self ):contract(self){}

         void exec();

   };

} /// namespace ultrainio
