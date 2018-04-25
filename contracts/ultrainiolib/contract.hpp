#pragma once

namespace ultrainio {

struct contract {
   contract( account_name n ):_self(n){}
   account_name _self;
};

} /// namespace ultrainio
