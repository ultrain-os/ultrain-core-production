/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/print.hpp>
#include <ultrainiolib/asset.hpp>
#include <ultrainiolib/currency.hpp>
#include<string>
#include<vector>
namespace ultrainiobank {
    using namespace ultrainio;
    using std::string;
    class bank : public contract {

        public:
            using contract::contract;
            std::string str1;

            void transfer( account_name from,
                        account_name to,
                        asset        quantity,
                        string       memo );

        private:
            struct bulletin_info {
                bulletin_info(account_name a, asset b)
                    :receiver(a),quantity(b){}
                bulletin_info(){}                
                account_name receiver;
                asset        quantity;
                ULTRAINLIB_SERIALIZE(bulletin_info, (receiver)(quantity))
            };
            struct bulletin_bank {
                uint64_t     block_height;
                std::vector<bulletin_info> bulletin_infos;
                
                uint64_t primary_key()const { return block_height; }

                ULTRAINLIB_SERIALIZE(bulletin_bank, (block_height)(bulletin_infos))
            };
            typedef ultrainio::multi_index<N(bulletinbank), bulletin_bank> bulletinbank;

    };
    extern "C" 
    void apply( uint64_t receiver, uint64_t code, uint64_t actH, uint64_t actL ) {
        action_name action(actH, actL); 
        if( code == N(utrio.token) && action == NEX(transfer) ){
            bank bk(receiver);
            const currency::transfer& t = unpack_action_data<currency::transfer>();
            bk.transfer(t.from, t.to, t.quantity, t.memo);
        }
    }
}/// namespace ultrainiobank
