#include "ultrainio.system.hpp"

namespace ultrainiosystem {


    const uint32_t relayer_deposit_threshold = 100000;

    /// @abi action
    void system_contract::regsubchain(uint64_t chain_name, uint16_t chain_type) {
        require_auth(N(ultrainio));
        auto itor = _subchains.find(chain_name);
        ultrainio_assert(itor == _subchains.end(), "There has been a subchian with this name.");

        _subchains.emplace(N(ultrainio), [&]( auto& new_subchain ) {
            new_subchain.chain_name        = chain_name;
            new_subchain.chain_type        = chain_type;
            new_subchain.is_active         = false;
            new_subchain.head_block_id     = block_id_type();
            new_subchain.head_block_num    = 0;
            new_subchain.chain_id          = checksum256();
        });
    }

    /// @abi action
    void system_contract::acceptheader (uint64_t chain_name,
                                  const ultrainio::block_header& header) {
        require_auth(current_sender());
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "This subchian is not existed.");
        //todo, check if the subchain is avtive?
        account_name block_proposer = header.proposer;
        auto itor = ite_chain->committee_members.begin();
        for(; itor != ite_chain->committee_members.end(); ++itor) {
            if(itor->owner == block_proposer && itor->producer_key.size() == 64) {
                break;
            }
        }
        ultrainio_assert(itor != ite_chain->committee_members.end(), "block proposer is not in committee list of this subchian.");
        //todo, check signatures.

        //compare previous with pre block's id.
        auto block_number = header.block_num();
        if(ite_chain->head_block_num == 0 ||
           (ite_chain->head_block_id == header.previous && block_number == ite_chain->head_block_num + 1)) {
              _subchains.modify(ite_chain, N(ultrainio), [&]( auto& _subchain ) {
                  _subchain.head_block_id     = header.id();
                  _subchain.head_block_num    = block_number;
              });
              //record proposer for rewards
              reportblocknumber(block_proposer, 1);
        }
        else if (ite_chain->head_block_id == header.id() && block_number == ite_chain->head_block_num) {
            //this block has been submited by other relayer
            return;
        }
        else if (block_number == ite_chain->head_block_num) {
            //todo, fork chain block, how to handle?
        }
        else if (block_number > ite_chain->head_block_num) {
            ultrainio_assert(false, "block number is greater than current block.");
        }
        else if (block_number < ite_chain->head_block_num) {
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


    void system_contract::add_to_pending_queue(account_name producer, const std::string& public_key) {
        auto itor = _producers.find(producer);
        ultrainio_assert(itor != _producers.end(), "need to register as a producer first");
/*
        if(!_pending_queue.exists()) {
            //todo, create a new pending chain?
        }

        auto _pending = _pending_subchain.get();
        auto ite = _pending.committee_members.begin();
        for(; ite != _pending.committee_members.end(); ++ite) {
            if(ite->owner == producer) {
                break;
            }
        }
        if(ite != _pending.committee_members.end() ) {
            return; //don't use assert
        }
        role_base temp_node;
        temp_node.owner = producer;
        temp_node.producer_key   = public_key;
        _pending.committee_members.push_back(temp_node);
        if(_pending.committee_members.size() >= _pending.min_committee_member_num) {
            start_pending_chain();
        }
        else {
            _pending_subchain.set(_pending, producer);
        }*/
    }

    void system_contract::remove_from_pending_queue(account_name producer) {
/*
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
*/
    }

    void system_contract::add_to_subchain(uint64_t chain_name, account_name producer, const std::string& public_key) {
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "The specific chain is not existed." );

        for(auto& miner : ite_chain->committee_members) {
            if(miner.owner == producer) {
                return; //don't use assert.
            }
        }
        _subchains.modify(ite_chain, producer, [&](subchain& info) {
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
            bool found = false;
            for(; ite_producer != info.committee_members.end(); ++ite_producer) {
                if(ite_producer->owner == producer) {
                    info.committee_members.erase(ite_producer);
                    found = true;
                    break;
                }
            }
            ultrainio_assert(found, "The producer is not existed on this subchain.");
        } );
    }
} //namespace ultrainiosystem
