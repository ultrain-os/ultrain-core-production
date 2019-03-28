#include "ultrainio.bank.hpp"
#include <ultrainio.system/ultrainio.system.hpp>
#include <ultrainiolib/system.h>
#include <ultrainiolib/types.hpp>
namespace ultrainiobank {
    void bank::transfer( account_name from,
                    account_name to,
                    asset        quantity,
                    string       memo ){
        ultrainiosystem::chains_table   _chains(N(ultrainio),N(ultrainio));
        auto const iter = std::find_if( _chains.begin(), _chains.end(), [&](auto& c){
           return c.chain_name == string_to_name(memo.c_str());
         });
        ultrainio_assert( iter != _chains.end(), " The chain to synchronize transfer transactions does not exist");
        ultrainiosystem::global_state_singleton   _global(N(ultrainio),N(ultrainio));
        ultrainio_assert( _global.exists(), "global table not found");
        ultrainiosystem::ultrainio_global_state _gstate = _global.get();
        ultrainio_assert( _gstate.chain_name != iter->chain_name, "Cannot transfer to the same chain");
        if(!_gstate.is_master_chain()){
            ultrainio_assert( N(ultrainio) == iter->chain_name, "Subchain funds transfer must be transferred to the main chain");
        }
        ultrainio_assert( to == N(utrio.bank), " Wrong account transferred to");
        ultrainio_assert( quantity >= asset(10), "The amount of funds transferred into the account is too small");

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
        chainbalance  chainbalan(_self, _self);
        auto it_chain = chainbalan.find( iter->chain_name );
        if(it_chain == chainbalan.end()){
            chainbalan.emplace( [&]( auto& b ){
                b.chain_name = iter->chain_name;
                b.balance += quantity;
            });
        }else{
            chainbalan.modify(it_chain, [&]( auto& b ) {
                b.balance += quantity;
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
