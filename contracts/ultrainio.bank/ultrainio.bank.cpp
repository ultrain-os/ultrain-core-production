#include "ultrainio.bank.hpp"
#include <ultrainio.system/ultrainio.system.hpp>
#include <ultrainiolib/system.h>
#include <ultrainiolib/types.hpp>
namespace ultrainiobank {
    void bank::checktranstobank( account_name from,
                            asset        quantity,
                            name chain_name ){
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
    }

    void bank::checktransfrombank( asset quantity, name chain_name ){
        if(chain_name == N(master)) //if master chain no check
            return;
        chainbalance  chainbalan(_self, _self);
        auto it_chain = chainbalan.find( chain_name );
        ultrainio_assert( it_chain != chainbalan.end(), " chainbalance chain_name no found" );
        ultrainio_assert( it_chain->balance >= quantity, " Insufficient funds transferred in" );
        chainbalan.modify(it_chain, [&]( auto& b ) {
            b.balance -= quantity;
        });
    }
    void bank::transfer( account_name from,
                    account_name to,
                    asset        quantity,
                    string       memo ){
        if(from == N(ultrainio))
            return;
        name  chain_name = name{string_to_name(memo.c_str())};
        if( to == N(utrio.bank) ){
            checktranstobank( from, quantity, chain_name );
        }else if( from == N(utrio.bank) ){
            checktransfrombank( quantity, chain_name );
        } else{
            ultrainio_assert( false, " Incorrect transfer action" );
        }

        print(name{from}," transfer to ", name{to}, "  ", quantity," to_chain:",memo);
        uint64_t block_height = (uint64_t)head_block_number() + 1;
        ultrainiosystem::lwc_singleton   _lwcsingleton(N(ultrainio),N(ultrainio));
        ultrainio_assert( _lwcsingleton.exists(), "lwc_singleton lwc table not found");
        ultrainiosystem::lwc_parameters _lwc = _lwcsingleton.get();
        if(block_height > _lwc.save_blocks_num){
            uint32_t loopsize = 0;
            bulletinbank bullbank( _self, chain_name );
            for(auto bulliter = bullbank.begin(); bulliter != bullbank.end(); ){
                if(bulliter->block_height < (block_height - _lwc.save_blocks_num))
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
