#include "ultrainio.bank.hpp"
#include <ultrainio.system/ultrainio.system.hpp>
#include <ultrainiolib/system.h>
#include <ultrainiolib/types.hpp>
namespace ultrainiobank {
    void bank::checktranstobank( account_name from,
                              const asset&  quantity,
                              const name&  to_chain_name,
                              const name& cur_chain_name ) {
        if( cur_chain_name == N(ultrainio) ){
            ultrainiosystem::chains_table   _chains(N(ultrainio),N(ultrainio));
            auto const iter = std::find_if( _chains.begin(), _chains.end(), [&](auto& c){
                return c.chain_name == to_chain_name;
            });
            ultrainio_assert( iter != _chains.end(), " The chain to synchronize transfer transactions does not exist");
            ultrainio_assert( cur_chain_name != iter->chain_name, "Cannot transfer to the same chain");
            recordchainbalance( to_chain_name, quantity );//if master chain,utrio.bank record the money transferred in
        }else{
            ultrainio_assert( N(ultrainio) == to_chain_name, "Subchain funds transfer must be transferred to the main chain");
            checkchainbalance( to_chain_name, quantity ); //if side chain,utrio.bank check the money transferred in
        }

        ultrainio_assert( quantity >= asset(10), "The amount of funds transferred into the account is too small");

        uint64_t block_height = (uint64_t)head_block_number() + 1;
        bulletin_info  bullinfo(from, quantity);
        bulletinbank bullbank( _self, to_chain_name );
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
    }

    void bank::checktransfrombank( asset quantity, name chain_name ){
        if( chain_name == N(master) || chain_name == N(ultrainio) ) { //如果从主链同步到侧链，在侧链记录下utrio.bank将要发行转出去的金额
            recordchainbalance( name{N(ultrainio)}, quantity );
        } else {  //如果从侧链同步到主链，在主链上检查下先前转入该侧链的金额必须足够本次在主链转出的金额
            checkchainbalance( chain_name, quantity );
        }
    }
    void bank::transfer( account_name from,
                    account_name to,
                    asset        quantity,
                    string       memo ){
        ultrainiosystem::global_state_singleton   _global(N(ultrainio),N(ultrainio));
        ultrainio_assert( _global.exists(), "global table not found");
        ultrainiosystem::ultrainio_global_state _gstate = _global.get();
        if( from == N(ultrainio) ){
            ultrainio_assert( !_gstate.is_master_chain(), " ultrainio is not allowed to transfer to utrio.bank in master chain" );
            if( memo != std::string( ultrainiosystem::bank_issue_memo ) ) {
                ultrainio_assert( false, " ultrainio is not allowed to transfer to utrio.bank in side chain" );
            } else  //sidechain utrio.bank issue
                return;
        }
        name  chain_name = name{string_to_name(memo.c_str())};
        if( to == N(utrio.bank) ){
            checktranstobank( from, quantity, chain_name, _gstate.chain_name );
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

    void bank::recordchainbalance( const name& chain_name, const asset& quantity ) {
        chainbalance  chainbalan(_self, _self);
        auto it_chain = chainbalan.find( chain_name );
        if(it_chain == chainbalan.end()){
            chainbalan.emplace( [&]( auto& b ){
                b.chain_name = chain_name;
                b.balance = quantity;
            });
        }else{
            chainbalan.modify(it_chain, [&]( auto& b ) {
                b.balance += quantity;
            });
        }
    }

    void bank::checkchainbalance( const name& chain_name, const asset& quantity ) {
        chainbalance  chainbalan(_self, _self);
        auto it_chain = chainbalan.find( chain_name );
        ultrainio_assert( it_chain != chainbalan.end(), " chainbalance chain_name no found" );
        ultrainio_assert( it_chain->balance >= quantity, " Insufficient funds transferred in" );
        chainbalan.modify(it_chain, [&]( auto& b ) {
            b.balance -= quantity;
        });
    }

}/// namespace ultrainiobank
