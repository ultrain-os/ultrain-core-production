#include "ultrainio.system.hpp"

namespace ultrainiosystem {


    const uint32_t relayer_deposit_threshold = 100000;

    /// @abi action
    void system_contract::regsubchain(uint64_t chain_name,
                                       account_name root_user_name,
                                       const std::string& root_user_pk,
                                       uint32_t sub_chain_deposit,
                                       uint32_t node_num) {
        require_auth(root_user_name);
        ultrainio_assert(!_pending_subchain.exists(), "There's already a pending subchain, please try later.");
        auto itor = _subchains.find(root_user_name);
        ultrainio_assert(itor == _subchains.end(), "This account has hosted a subchian.");

        //todo, if there are enough nodes in the waiting queue, start the new chain immediately.

        //else, store it in pending chain.
        subchain_base new_subchain;
        new_subchain.root_name                 = root_user_name;
        new_subchain.root_pk                   = root_user_pk;
        new_subchain.min_active_stake          = sub_chain_deposit;
        new_subchain.min_committee_member_num  = node_num;
        new_subchain.total_deposit             = 0;
        new_subchain.head_block_id             = block_id_type();
        new_subchain.head_block_num            = 0;
        _pending_subchain.set(new_subchain, root_user_name);
    }

    /// @abi action
    void system_contract::acceptheader (uint64_t chain_name,
                                  const ultrainio::block_header& header) {
        require_auth(current_sender());
        //todo, check chain_name is a valid name of subchain
        account_name block_proposer = header.proposer;
        //todo, check the pending chain temporarily, will update to check subchain's committee list.
        //use the _pending_subchain temporarily
        ultrainio_assert(_pending_subchain.exists(), "please start a subchain first");
        auto _pending = _pending_subchain.get();
        auto itor = _pending.committee_members.begin();
        for(; itor != _pending.committee_members.end(); ++itor) {
            if(itor->owner == block_proposer && itor->producer_key.size() == 64) {
                break;
            }
        }
        ultrainio_assert(itor != _pending.committee_members.end(), "block proposer is not in committee list of this subchian.");
        //todo, check signatures.

        //compare previous with pre block's id.
        auto block_number = header.block_num();
        if(_pending.head_block_num == 0 ||
           (_pending.head_block_id == header.previous && block_number == _pending.head_block_num + 1)) {
              _pending.head_block_id = header.id(); 
              _pending.head_block_num = block_number;
              _pending_subchain.set(_pending, block_proposer);
              //record proposer for rewards
              reportblocknumber(block_proposer, 1);
        }
        else if (_pending.head_block_id == header.id() && block_number == _pending.head_block_num) {
            //this block has been submited by other relayer
            return;
        }
        else if (block_number == _pending.head_block_num) {
            //todo, fork chain block, how to handle?
        }
        else if (block_number > _pending.head_block_num) {
            ultrainio_assert(false, "block number is greater than current block.");
        }
        else if (block_number < _pending.head_block_num) {
            ultrainio_assert(false, "block number is less than current block.");
        }
        else {
            ultrainio_assert(false, "block header verification failed.");
        }
    }
/*
    void system_contract::register_relayer(const std::string& miner_pk,
                                     account_name relayer_account_name,
                                     uint32_t relayer_deposit,
                                     const std::string& ip,
                                     uint64_t chain_name) {}
*/

    void system_contract::start_pending_chain() {
        ultrainio_assert(_pending_subchain.exists(), "pending chain is not existed.");
        //move pending subchain into subchains table
        auto _pending = _pending_subchain.get();
        _subchains.emplace(_pending.root_name, [&]( auto& new_subchain ) {
            new_subchain.root_name                 = _pending.root_name;
            new_subchain.root_pk                   = _pending.root_pk;
            new_subchain.min_active_stake          = _pending.min_active_stake;
            new_subchain.min_committee_member_num  = _pending.min_committee_member_num;
            new_subchain.total_deposit             = _pending.total_deposit;
            new_subchain.head_block_id             = _pending.head_block_id;
            new_subchain.head_block_num            = _pending.head_block_num;
//1            new_subchain.chain_hash                = ; //new generated or set?
            new_subchain.genesis_info              = "";
            new_subchain.relayer_stake_threshold   = 0;
        });
        //clear the _pending
        _pending_subchain.remove();
        //todo, select relayers.
    }

    void system_contract::add_to_pendingchain(account_name producer, const std::string& public_key) {
        auto itor = _producers.find(producer);
        ultrainio_assert(itor != _producers.end(), "need to register as a producer first");
        if(!_pending_subchain.exists()) {
            //todo, create a new pending chain?
            ultrainio_assert(false, "need to register as a subchian first");
        }

        auto _pending = _pending_subchain.get();
        auto ite = _pending.committee_members.begin();
        for(; ite != _pending.committee_members.end(); ++ite) {
            if(ite->owner == producer) {
                break;
            }
        }
        ultrainio_assert(ite == _pending.committee_members.end(), "this account has registered to this subchian.");
        role_base temp_node;
        temp_node.owner = producer;
        temp_node.producer_key   = public_key;
        _pending.committee_members.push_back(temp_node);
        if(_pending.committee_members.size() >= _pending.min_committee_member_num) {
            start_pending_chain();
        }
        else {
            _pending_subchain.set(_pending, producer);
        }
    }

    void system_contract::remove_from_pendingchain(account_name producer) {
        ultrainio_assert(_pending_subchain.exists(), "no pending subchain exsited.");

        auto _pending = _pending_subchain.get();
        auto ite = _pending.committee_members.begin();
        for(; ite != _pending.committee_members.end(); ++ite) {
            if(ite->owner == producer) {
                break;
            }
        }
        ultrainio_assert(ite != _pending.committee_members.end(), "this producer is not in the pending subchian.");
        _pending.committee_members.erase(ite);
        _pending_subchain.set(_pending, producer);
    }

    void system_contract::add_to_subchain(uint64_t chain_name, account_name producer, const std::string& public_key) {
        auto ite_chain = _subchains.find(chain_name);
        if(ite_chain == _subchains.end()) {
            //change location to pending_chain_name
            auto prod = _producers.find( producer );
            if ( prod != _producers.end() ) {
                _producers.modify( prod, producer, [&]( producer_info& info ){
                    info.location     = pending_chain_name;
                });
            }
            add_to_pendingchain(producer, public_key);
            return;
        }

        _subchains.modify(ite_chain, producer, [&](subchain& info){
            role_base temp_node;
            temp_node.owner = producer;
            temp_node.producer_key   = public_key;
            info.committee_members.push_back(temp_node);
        } );

        //todo, modify dest_location in DB _producers.
    }

    void system_contract::remove_from_subchain(uint64_t chain_name, account_name producer) {
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "The subchain is not existed.");

        _subchains.modify(ite_chain, producer, [&](subchain& info){
            auto ite_producer = info.committee_members.begin();
            for(; ite_producer != info.committee_members.end(); ++ite_producer) {
                if(ite_producer->owner == producer) {
                    info.committee_members.erase(ite_producer);
                }
            }
        } );
    }
} //namespace ultrainiosystem
