#include "ultrainio.bank.hpp"
#include <ultrainio.system/ultrainio.system.hpp>
#include <ultrainiolib/system.h>
#include <ultrainiolib/types.hpp>
namespace ultrainiobank {
    void bank::transfer( account_name from,
                    account_name to,
                    asset        quantity,
                    string       memo ){
        if(from == N(utrio.bank) || from == N(ultrainio))
            return;
        name  chain_name = name{string_to_name(memo.c_str())};
        ultrainiosystem::global_state_singleton   _global(N(ultrainio),N(ultrainio));
        ultrainio_assert( _global.exists(), "global table not found");
        ultrainiosystem::ultrainio_global_state _gstate = _global.get();
        if(_gstate.is_master_chain()){
            ultrainiosystem::chains_table   _chains(N(ultrainio),N(ultrainio));
            auto const iter = std::find_if( _chains.begin(), _chains.end(), [&](auto& c){
                return c.chain_name == chain_name;
            });
            ultrainio_assert( iter != _chains.end(), " The chain to synchronize transfer transactions does not exist");
            ultrainio_assert( _gstate.chain_name != iter->chain_name, "Cannot transfer to the same chain");
        }else{
            ultrainio_assert( N(ultrainio) == chain_name, "Subchain funds transfer must be transferred to the main chain");
        }

        ultrainio_assert( to == N(utrio.bank), " Wrong account transferred to");
        ultrainio_assert( quantity >= asset(10), "The amount of funds transferred into the account is too small");

        uint64_t block_height = (uint64_t)head_block_number() + 1;
        bulletin_info  bullinfo(from, quantity);
        bulletinbank bullbank( _self, chain_name);
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
        auto it_chain = chainbalan.find( chain_name );
        if(it_chain == chainbalan.end()){
            chainbalan.emplace( [&]( auto& b ){
                b.chain_name = chain_name;
                b.balance += quantity;
            });
        }else{
            chainbalan.modify(it_chain, [&]( auto& b ) {
                b.balance += quantity;
            });
        }
        print(name{from}," transfer to ", name{to}, "  ", quantity," to_chain:",memo);
        if(block_height > 50000){
            uint32_t loopsize = 0;
            for(auto bulliter = bullbank.begin(); bulliter != bullbank.end(); ){
                if(bulliter->block_height < (block_height - 50000))
                    bulliter = bullbank.erase(bulliter);
                else
                    break;
                loopsize++;
                if(loopsize > 1000)
                    break;
            }
        }

    }

}/// namespace ultrainiobank
