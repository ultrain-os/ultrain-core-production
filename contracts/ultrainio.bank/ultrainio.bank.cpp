#include "ultrainio.bank.hpp"
#include <ultrainio.system/ultrainio.system.hpp>
#include <ultrainiolib/system.h>
#include <ultrainiolib/types.hpp>
namespace ultrainiobank {
    void bank::transfer( account_name from,
                    account_name to,
                    asset        quantity,
                    string       memo ){
        ultrainiosystem::chains_table             _chains(N(ultrainio),N(ultrainio));
        //auto const iter = std::find_if( _chains.begin(), _chains.end(), string_to_name(memo.c_str()) );
        //ultrainio_assert(iter != chain_name_vec.end()," The chain to synchronize transfer transactions does not exist");
        //ultrainio_assert(to == N(utrio.bank)," Wrong account transferred to");
        ultrainio_assert(quantity.amount >= 10,"The amount of funds transferred into the account is too small");
        uint64_t block_height = (uint64_t)head_block_number() + 1;
        bulletin_info  bullinfo(from, quantity);
        bulletinbank bullbank( _self, string_to_name(memo.c_str()));
        auto it_bank = bullbank.find(block_height);
        if(it_bank == bullbank.end()){
            bullbank.emplace( [&]( auto& b ){
                b.block_height = block_height;
                b.bulletin_infos.emplace_back(bullinfo);
            });
        }else{
            bullbank.modify(it_bank, [&]( auto& b ) {
                b.bulletin_infos.emplace_back(bullinfo);
            });            
        }

        print(name{from}," transfer to ", name{to}, "  ", quantity," to_chain:",memo);
        if(block_height > 1000){
            for(auto bulliter = bullbank.begin(); bulliter != bullbank.end(); ){
                if(bulliter->block_height < (block_height - 1000))
                    bulliter = bullbank.erase(bulliter);
                else
                    break;
            }
        }

    }

}/// namespace ultrainiobank
