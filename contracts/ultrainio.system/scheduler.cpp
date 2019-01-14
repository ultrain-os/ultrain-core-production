#include "ultrainio.system.hpp"

namespace ultrainiosystem {


    //const uint32_t relayer_deposit_threshold = 100000;

    void system_contract::regchaintype(uint16_t type_id, uint32_t min_producer_num, uint32_t max_producer_num, uint16_t consensus_period) {
        require_auth(N(ultrainio));
        chaintypes_table type_tbl(_self, _self);
        auto typeiter = type_tbl.find((uint64_t)type_id);
        if (typeiter == type_tbl.end()) {
            type_tbl.emplace(N(ultrainio), [&]( auto& new_subchain_type ) {
                new_subchain_type.id = type_id;
                new_subchain_type.min_producers = min_producer_num;
                new_subchain_type.max_producers = max_producer_num;
                new_subchain_type.consensus_period = consensus_period;
            });
        } else {
            type_tbl.modify(typeiter, N(ultrainio), [&]( auto& _subchain_type ) {
                _subchain_type.id = type_id;
                _subchain_type.min_producers = min_producer_num;
                _subchain_type.max_producers = max_producer_num;
                _subchain_type.consensus_period = consensus_period;
            });
        }
    }
    ///@abi action
    void system_contract::reportsubchainhash(uint64_t subchain, uint64_t blocknum, checksum256 hash) {
        require_auth(current_sender());
        auto propos = _producers.find(current_sender());
        /*
         *
         *TODO
          1. uncomment the 3 assert when MT done, and it's ready to test with subchain header report
          2. denial of the report when the hash for the blocknum has been voted by more than 2/3 committee member
          3. according the world state design, change the magic num (10000, 1000) to macro or const.
          4. delete the outdated hash when new hash reported.
         */
        //ultrainio_assert( propos != _producers.end() && propos->is_enabled && propos->location == subchain, "enabled producer not found this proposer" );
        auto checkBlockNum = [&](uint64_t  bn) -> bool {
            return (bn%10000) == 0;
        };
        ultrainio_assert(checkBlockNum(blocknum), "report an invalid blocknum ws");
        auto ite_chain = _subchains.find(subchain);
        //ultrainio_assert(blocknum <= ite_chain->head_block_num, "report a blocknum larger than current block");
        //ultrainio_assert(ite_chain->head_block_num - blocknum <= 1000, "report a too old blocknum");

        subchain_hash_table     hashTable(_self, subchain);

        auto wshash = hashTable.find(blocknum);
        if(wshash != hashTable.end()) {
            auto & hashv = wshash->hash_v;
            auto & acc   = wshash->accounts;
            auto ret = std::find(acc.begin(), acc.end(), current_sender());
            ultrainio_assert(ret == acc.end(), "the committee_members already report such ws hash");
            auto it = hashv.begin();
            for(; it != hashv.end(); it++) {
                if(hash == it->hash) {
                    hashTable.modify(wshash, 0, [&](auto &p) {
                            p.hash_v[static_cast<unsigned int>(it-hashv.begin())].votes++;
                            p.accounts.emplace_back(current_sender());
                            });
                    break;
                }
            }
            if(it == hashv.end()) {
                hashTable.modify(wshash, 0, [&](auto& p) {
                        p.hash_v.emplace_back(hash, 1);
                        p.accounts.emplace_back(current_sender());
                        });
            }
        }
        else {
            hashTable.emplace(_self, [&](auto &p) {
                    p.block_num = blocknum;
                    p.hash_v.emplace_back(hash, 1);
                    p.accounts.emplace_back(current_sender());
                    });

        }
    }
    /// @abi action
    void system_contract::regsubchain(uint64_t chain_name, uint16_t chain_type, time genesis_time) {
        require_auth(N(ultrainio));
        ultrainio_assert(chain_name != default_chain_name, "subchian cannot named as default.");
        ultrainio_assert(chain_name != master_chain_name, "subchian cannot named as 0");
        ultrainio_assert(chain_name != pending_queue, "subchian cannot named as uint64::max");
        //auto itor = _subchains.find(chain_name);
        //ultrainio_assert(itor == _subchains.end(), "There has been a subchian with this name.");

        _subchains.emplace(N(ultrainio), [&]( auto& new_subchain ) {
            new_subchain.chain_name        = chain_name;
            new_subchain.chain_type        = chain_type;
            new_subchain.genesis_time      = genesis_time;
            new_subchain.is_active         = false;
            new_subchain.is_synced         = false;
            new_subchain.head_block_id     = block_id_type();
            new_subchain.head_block_num    = 0;
            new_subchain.updated_info.take_effect_at_block = 0;
        });
    }

    /// @abi action
    void system_contract::acceptheader (uint64_t chain_name,
                                  const std::vector<ultrainio::block_header>& headers) {
        require_auth(current_sender());
        ultrainio_assert(headers.size() <= 10, "Too many blocks are reported.");
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "This subchian is not existed.");
        bool synced = headers.size() == 10 ? false : true;  //10 blocks is the max sum when reporting
        //todo, check if the subchain is avtive?
        for(uint32_t idx = 0; idx < headers.size(); ++idx) {
            account_name block_proposer = headers[idx].proposer;
            bool found_prod = false;
            bool need_report = true;

            if(block_proposer != N() && block_proposer != N(genesis)) {
                auto prod = _producers.find(block_proposer);
                ultrainio_assert(prod != _producers.end(), "The prososer is not a valid producer");
                auto itor = ite_chain->committee_members.begin();
                for(; itor != ite_chain->committee_members.end(); ++itor) {
                    if(itor->owner == block_proposer) {
                        found_prod = true;
                        break;
                    }
                }
                //found producer and it's a new added member of last update, means the update works in subchain
                if(found_prod && !ite_chain->updated_info.unactivated_committee.empty()) {
                    auto new_prod = ite_chain->updated_info.unactivated_committee.begin();
                    for(; new_prod != ite_chain->updated_info.unactivated_committee.end(); ++new_prod) {
                        if(*new_prod == block_proposer) {
                            //new committee works, clear unactivated list
                            _subchains.modify(ite_chain, N(ultrainio), [&]( auto& _subchain ) {
                                _subchain.updated_info.unactivated_committee.clear();
                                _subchain.updated_info.unactivated_committee.shrink_to_fit();
                                _subchain.updated_info.take_effect_at_block = (uint32_t)tapos_block_num() + 30;
                            });
                            break;
                        }
                    }
                }
                else if(!found_prod && ite_chain->updated_info.deprecated_committee.empty() ) {
                    uint32_t cur_block_num = (uint32_t)tapos_block_num();
                    for(auto ite_prod = ite_chain->updated_info.deprecated_committee.begin();
                        ite_prod != ite_chain->updated_info.deprecated_committee.end(); ++ite_prod) {
                        if(ite_prod->owner == block_proposer) {
                            //todo, check quit num in produer table for the case about moving producer
                            if(ite_chain->updated_info.take_effect_at_block != 0 &&
                               cur_block_num > ite_chain->updated_info.take_effect_at_block) {
                                need_report = false; //no reward
                            }
                            found_prod = true;
                            break;
                        }
                    }
                }

                if(!found_prod) {
                    //no reward for the producer. Here is not suggested to use assert
                    //todo, add log
                    need_report = false;
                }
            }
            else {
                need_report = false; //Don't pay for null block and all blocks produced by genesis
            }
            //todo, check signatures.

            //compare previous with pre block's id.
            auto block_number = headers[idx].block_num();
            if(ite_chain->head_block_num == 0 ||
               (ite_chain->head_block_id == headers[idx].previous && block_number == ite_chain->head_block_num + 1)) {
                  _subchains.modify(ite_chain, N(ultrainio), [&]( auto& _subchain ) {
                      _subchain.is_synced         = synced;
                      _subchain.head_block_id     = headers[idx].id();
                      _subchain.head_block_num    = block_number;
                  });
                  //record proposer for rewards
                  if(need_report) {
                      reportblocknumber(block_proposer, 1);
                  }
            }
            else if (ite_chain->head_block_id == headers[idx].id() && block_number == ite_chain->head_block_num) {
                //this block has been submited by other relayer
                return;
            }
            else if (block_number == ite_chain->head_block_num) {
                //todo, fork chain block, how to handle?
                ultrainio_assert(false, "fork chian block.");
            }
            else if (block_number > ite_chain->head_block_num + 1) {
                ultrainio_assert(false, "block number is greater than current block.");
            }
            else if (block_number < ite_chain->head_block_num) {
                ultrainio_assert(false, "block number is less than current block.");
            }
            else {
                ultrainio_assert(false, "block header verification failed.");
            }
        }
    }

    void system_contract::clearchain(uint64_t chain_name, bool users_only) {
        require_auth(N(ultrainio));
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "This subchian is not existed.");
        //todo, check if the subchain is avtive?
        if(users_only) {
            _subchains.modify(ite_chain, 0, [&]( auto& _subchain ) {
                _subchain.users.clear();
                _subchain.users.shrink_to_fit();
            });
            return;
        }
        _subchains.modify(ite_chain, N(ultrainio), [&]( auto& _subchain ) {
            _subchain.users.clear();
            _subchain.users.shrink_to_fit();
            _subchain.head_block_id     = block_id_type();
            _subchain.head_block_num    = 0;
        });
        auto ite = _producers.begin();
        for(; ite != _producers.end(); ++ite) {
            if(ite->location == chain_name) {
                _producers.modify(ite, N(ultrainio), [&](auto & p) {
                     p.unpaid_blocks = 0;
                     p.total_produce_block = 0;
                });
            }
        }
    }

    void system_contract::empoweruser(account_name user, const std::string& owner_pk, const std::string& active_pk, uint64_t chain_name) {
        require_auth(user);
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "This subchian is not existed.");
        ultrainio_assert(owner_pk.size() == 53, "owner public key should be of size 53");
        ultrainio_assert(active_pk.size() == 53, "avtive public key should be of size 53");
        //todo, check whether this subchain is active.

        user_info tempuser;
        tempuser.user_name = user;
        tempuser.owner_key = owner_pk;
        tempuser.active_key = active_pk;
        tempuser.emp_time = current_time();
        tempuser.block_num = (uint32_t)tapos_block_num();

        _subchains.modify(ite_chain, N(ultrainio), [&]( auto& _chain ) {
            for(const auto& _user : _chain.users) {
                ultrainio_assert(_user.user_name != user, "User has existed in this subchain.");
            }
            _chain.users.push_back(tempuser);
        });
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

        std::vector<role_base>* p_que;
        if(!_pending_que.exists()) {
            std::vector<role_base> temp_que;
            p_que = &temp_que;
        }
        else {
            auto _pending = _pending_que.get();
            auto ite = _pending.begin();
            for(; ite != _pending.end(); ++ite) {
                if(ite->owner == producer) {
                    break;
                }
            }
            if(ite != _pending.end() ) {
                return; //don't use assert
            }
            p_que = &_pending;
        }
        role_base temp_node;
        temp_node.owner = producer;
        temp_node.producer_key   = public_key;
        p_que->push_back(temp_node);
        //loop all inactive subchain, start it if miners sum meet its requirement.
        auto ite_subchain = _subchains.begin();
        for(; ite_subchain != _subchains.end(); ++ite_subchain) {
            if(!ite_subchain->is_active && ite_subchain->get_subchain_min_miner_num() <= p_que->size()) {
                break;
            }
        }
        if(ite_subchain != _subchains.end()) {
            _subchains.modify(ite_subchain, N(ultrainio), [&](subchain& info) {
                //move miners from pending que to this sub chian
                uint32_t i = 0;
                auto ite_miner = p_que->begin();
                for(; ite_miner != p_que->end() && i <= info.get_subchain_min_miner_num(); ++i) {
                    //update location of producer
                    auto ite_producer = _producers.find(ite_miner->owner);
                    ultrainio_assert( ite_producer != _producers.end(), "cannot find producer in database" );
                    _producers.modify(ite_producer, 0 , [&](auto & v) {
                        v.location = info.chain_name;
                    });
                    //move to subchain
                    info.committee_members.push_back(*ite_miner);
                    ite_miner = p_que->erase(ite_miner);
                }
                info.is_active = true;
            });
        }

        _pending_que.set(*p_que, producer);
    }

    void system_contract::remove_from_pending_queue(account_name producer) {
        ultrainio_assert(_pending_que.exists(), "no pending queue exsited.");

        auto _pending = _pending_que.get();
        auto ite = _pending.begin();
        for(; ite != _pending.end(); ++ite) {
            if(ite->owner == producer) {
                break;
            }
        }
        if(ite == _pending.end()) {
            return; //don't use assert
        }
        _pending.erase(ite);
        _pending_que.set(_pending, producer);
    }

    void system_contract::add_to_subchain(uint64_t chain_name, account_name producer, const std::string& public_key) {
        if(chain_name == default_chain_name) {
            //todo, loop all stable subchain, add to the one with least miners
            //todo, update location of producer
            return;
        }
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "The specific chain is not existed." );

        for(const auto& miner : ite_chain->committee_members) {
            if(miner.owner == producer) {
                _subchains.modify(ite_chain, producer, [&](subchain& info) {
                    auto ite_miner = info.changing_info.removed_members.begin();
                    for(; ite_miner != info.changing_info.removed_members.end(); ++ite_miner) {
                        if(miner.owner == producer) {
                            info.changing_info.removed_members.erase(ite_miner);
                            break;
                        }
                        info.changing_info.removed_members.shrink_to_fit();
                    }
                } );
                return; //don't use assert.
            }
        }
        //check changing info
        for(const auto& miner : ite_chain->changing_info.new_added_members) {
            if(miner.owner == producer) {
                return;
            }
        }
        _subchains.modify(ite_chain, producer, [&](subchain& info) {
            role_base temp_node;
            temp_node.owner = producer;
            temp_node.producer_key   = public_key;
            info.changing_info.new_added_members.push_back(temp_node);
        } );
    }

    void system_contract::remove_from_subchain(uint64_t chain_name, account_name producer) {
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "The subchain is not existed.");

        ultrainio_assert(ite_chain->committee_members.size() - 1 >= ite_chain->get_subchain_min_miner_num(),
                         "this subschain cannot remove producers");
        _subchains.modify(ite_chain, producer, [&](subchain& info){
            auto ite_add = info.changing_info.new_added_members.begin();
            for(; ite_add != info.changing_info.new_added_members.end() ; ++ite_add) {
                if(ite_add->owner == producer) {
                    info.changing_info.new_added_members.erase(ite_add);
                    return;
                }
            }

            auto ite_producer = info.committee_members.begin();
            bool found = false;
            for(; ite_producer != info.committee_members.end(); ++ite_producer) {
                if(ite_producer->owner == producer) {
                    found = true;
                    break;
                }
            }
            ultrainio_assert(found, "The producer is not existed on this subchain.");
            info.changing_info.removed_members.push_back(*ite_producer);
        } );
    }

    void system_contract::activate_committee_update() {
        auto block_height = tapos_block_num();
        if(block_height > 120 && block_height%360 != 0) {
            return;  //do this operation every 1 hours == 360 block.
        }
        auto ite_chain = _subchains.begin();
        for(; ite_chain != _subchains.end(); ++ite_chain) {
            if(!ite_chain->updated_info.deprecated_committee.empty() ||
               !ite_chain->updated_info.unactivated_committee.empty() ||
               !ite_chain->changing_info.removed_members.empty() ||
               !ite_chain->changing_info.new_added_members.empty() ||
               ite_chain->updated_info.take_effect_at_block != 0 ) {
                _subchains.modify(ite_chain, N(ultrainio), [&]( auto& _subchain ) {
                    //remove removed producers from committee
                    auto removing_comm = _subchain.changing_info.removed_members.begin();
                    for(; removing_comm != _subchain.changing_info.removed_members.end(); ++removing_comm) {
                        auto current_comm = _subchain.committee_members.begin();
                        for(; current_comm != _subchain.committee_members.end();) {
                            if(current_comm->owner == removing_comm->owner) {
                                current_comm = _subchain.committee_members.erase(current_comm);
                            }
                            else {
                                ++current_comm;
                            }
                        }
                    }
                    //add new added to committee
                    if(!_subchain.changing_info.new_added_members.empty()) {
                        _subchain.committee_members.reserve(_subchain.committee_members.size() + _subchain.changing_info.new_added_members.size());
                        _subchain.committee_members.insert(_subchain.committee_members.end(),
                                                           _subchain.changing_info.new_added_members.begin(),
                                                           _subchain.changing_info.new_added_members.end());
                    }
                    _subchain.committee_members.shrink_to_fit();
                    //clear updated_info and then move changing_info to updated_info
                    if(_subchain.is_synced) {
                        _subchain.updated_info.deprecated_committee.swap(_subchain.changing_info.removed_members);
                    }
                    else {
                        _subchain.updated_info.deprecated_committee.reserve(_subchain.updated_info.deprecated_committee.size() + _subchain.changing_info.removed_members.size());
                        _subchain.updated_info.deprecated_committee.insert(_subchain.updated_info.deprecated_committee.end(),
                                                std::make_move_iterator(_subchain.changing_info.removed_members.begin()),
                                              std::make_move_iterator(_subchain.changing_info.removed_members.end()));
                    }
                    _subchain.updated_info.take_effect_at_block = 0;
                    _subchain.changing_info.removed_members.clear();

                    _subchain.updated_info.unactivated_committee.clear();
                    auto new_member = _subchain.changing_info.new_added_members.begin();
                    for(; new_member != _subchain.changing_info.new_added_members.end(); ++new_member) {
                        _subchain.updated_info.unactivated_committee.emplace_back(new_member->owner);
                    }
                    _subchain.changing_info.new_added_members.clear();

                    _subchain.updated_info.deprecated_committee.shrink_to_fit();
                    _subchain.updated_info.unactivated_committee.shrink_to_fit();
                    _subchain.changing_info.removed_members.shrink_to_fit();
                    _subchain.changing_info.new_added_members.shrink_to_fit();
                });
            }
        }
    }

    chaintype system_contract::get_subchain_basic_info(uint16_t chain_type) const {
        chaintypes_table type_tbl(_self, _self);
        auto typeiter = type_tbl.find(chain_type);
        ultrainio_assert(typeiter != type_tbl.end(), "chain type is not existed");
        return *typeiter;
    }
} //namespace ultrainiosystem
