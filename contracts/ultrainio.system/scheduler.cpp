#include "ultrainio.system.hpp"
#include <list>

bool operator!=(const checksum256& sha256_1, const checksum256& sha256_2) {
    for(auto i = 0; i < 32; ++i) {
        if(sha256_1.hash[i] != sha256_2.hash[i]) {
            return true;
        }
    }
    return false;
}

namespace ultrainiosystem {

    //const uint32_t relayer_deposit_threshold = 100000;

    ///@abi action
    void system_contract::regchaintype(uint64_t type_id, uint16_t min_producer_num, uint16_t max_producer_num,
                                       uint16_t sched_step, uint16_t consensus_period) {
        require_auth(N(ultrainio));
        ultrainio_assert(min_producer_num >= 4, "wrong min_producer_num, at least 4 producers is required for a chain");
        ultrainio_assert(min_producer_num < max_producer_num, "max_producer_num must grater than min_producer_num");
        ultrainio_assert(sched_step > 1 && sched_step <= 100, "sched_step should in scope [2, 100]");
        ultrainio_assert(consensus_period > 1 && consensus_period <= 10, "consensus_period should in scope [2, 10]");
        chaintypes_table type_tbl(_self, _self);
        auto typeiter = type_tbl.find(type_id);
        if (typeiter == type_tbl.end()) {
            type_tbl.emplace( [&]( auto& new_subchain_type ) {
                new_subchain_type.type_id = type_id;
                new_subchain_type.stable_min_producers = min_producer_num;
                new_subchain_type.stable_max_producers = max_producer_num;
                new_subchain_type.sched_inc_step = sched_step;
                new_subchain_type.consensus_period = consensus_period;
            });
        } else {
            type_tbl.modify(typeiter, [&]( auto& _subchain_type ) {
                _subchain_type.stable_min_producers = min_producer_num;
                _subchain_type.stable_max_producers = max_producer_num;
                _subchain_type.sched_inc_step = sched_step;
                _subchain_type.consensus_period = consensus_period;
            });
        }
    }
    ///@abi action
    void system_contract::reportsubchainhash(uint64_t subchain, uint64_t blocknum, checksum256 hash, uint64_t file_size) {
        require_auth(current_sender());
        auto propos = _producers.find(current_sender());
        /*
         *
         *TODO
          1. denial of the report when the hash for the blocknum has been voted by more than 2/3 committee member
         */
        ultrainio_assert( propos != _producers.end() && propos->is_enabled && propos->location == subchain, "enabled producer not found this proposer" );
        auto checkBlockNum = [&](uint64_t  bn) -> bool {
            return (bn%default_worldstate_interval) == 0;
        };
        ultrainio_assert(checkBlockNum(blocknum), "report an invalid blocknum ws");
        auto ite_chain = _subchains.find(subchain);
        ultrainio_assert(ite_chain != _subchains.end(), "subchain not found");
        ultrainio_assert(blocknum <= ite_chain->head_block_num, "report a blocknum larger than current block");
        ultrainio_assert(ite_chain->head_block_num - blocknum <= default_worldstate_interval, "report a too old blocknum");

        subchain_hash_table     hashTable(_self, subchain);

        auto wshash = hashTable.find(blocknum);
        if(wshash != hashTable.end()) {
            auto & hashv = wshash->hash_v;
            auto & acc   = wshash->accounts;
            auto ret = std::find(acc.begin(), acc.end(), current_sender());
            ultrainio_assert(ret == acc.end(), "the committee_members already report such ws hash");
            auto it = hashv.begin();
            for(; it != hashv.end(); it++) {
                if(hash == it->hash && file_size == it->file_size) {
                    hashTable.modify(wshash, [&](auto &p) {
                            p.hash_v[static_cast<unsigned int>(it - hashv.begin())].votes++;
                            p.accounts.emplace_back(current_sender());
                            });
                    break;
                }
            }
            if(it == hashv.end()) {
                hashTable.modify(wshash, [&](auto& p) {
                    p.hash_v.emplace_back(hash, file_size, 1);
                    p.accounts.emplace_back(current_sender());
                });
            }
        }
        else {
            hashTable.emplace([&](auto &p) {
                    p.block_num = blocknum;
                    p.hash_v.emplace_back(hash, file_size, 1);
                    p.accounts.emplace_back(current_sender());
                    });
            if (blocknum > uint64_t(MAX_WS_COUNT * default_worldstate_interval)){
                uint64_t expired = blocknum - uint64_t(MAX_WS_COUNT * default_worldstate_interval);
                auto old = hashTable.find(expired);
                if (old != hashTable.end())
                    hashTable.erase(old);
            }
        }
    }
    /// @abi action
    void system_contract::regsubchain(uint64_t chain_name, uint64_t chain_type) {
        require_auth(N(ultrainio));
        ultrainio_assert(chain_name != default_chain_name, "subchian cannot named as default.");
        ultrainio_assert(chain_name != master_chain_name, "subchian cannot named as 0");
        ultrainio_assert(chain_name != pending_queue, "subchian cannot named as uint64::max");
        auto itor = _subchains.find(chain_name);
        ultrainio_assert(itor == _subchains.end(), "there has been a subchian with this name");
        chaintypes_table type_tbl(_self, _self);
        auto type_iter = type_tbl.find(chain_type);
        ultrainio_assert(type_iter != type_tbl.end(), "this chain type is not existed");

        _subchains.emplace( [&]( auto& new_subchain ) {
            new_subchain.chain_name        = chain_name;
            new_subchain.chain_type        = chain_type;
            new_subchain.is_active         = false;
            new_subchain.is_synced         = false;
            new_subchain.head_block_id     = block_id_type();
            new_subchain.head_block_num    = 0;
            new_subchain.updated_info.take_effect_at_block = 0;
            new_subchain.is_schedulable    = false;
            new_subchain.total_user_num    = 0;
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
                            _subchains.modify(ite_chain, [&]( auto& _subchain ) {
                                _subchain.updated_info.unactivated_committee.clear();
                                _subchain.updated_info.unactivated_committee.shrink_to_fit();
                                _subchain.updated_info.take_effect_at_block = (uint32_t)head_block_number() + 30;
                            });
                            break;
                        }
                    }
                }
                else if(!found_prod && ite_chain->updated_info.deprecated_committee.empty() ) {
                    uint32_t cur_block_num = (uint32_t)head_block_number() + 1;
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
            if(0 == ite_chain->head_block_num && 1 == block_number) {
                  _subchains.modify(ite_chain, [&]( auto& _subchain ) {
                      _subchain.head_block_id     = headers[idx].id();
                      _subchain.head_block_num    = 1;
                      _subchain.chain_id          = headers[idx].action_mroot; //save chain id
                      _subchain.genesis_time      = headers[idx].timestamp;
                      _subchain.is_schedulable    = true;
                      _subchain.committee_mroot   = headers[idx].committee_mroot;
                  });
            }
            else if (ite_chain->head_block_id == headers[idx].previous && block_number == ite_chain->head_block_num + 1) {
                  _subchains.modify(ite_chain, [&]( auto& _subchain ) {
                      _subchain.is_synced         = synced;
                      _subchain.head_block_id     = headers[idx].id();
                      _subchain.head_block_num    = block_number;
                      if(ite_chain->committee_mroot != headers[idx].committee_mroot) {
                          _subchain.committee_mroot = headers[idx].committee_mroot;
                          _subchain.is_schedulable  = true;
                      }
                  });
                  // veirfy block header ext
                  ultrainio::extensions_type exts = headers[idx].header_extensions;
                  print("acceptheader verify block header exts.size : ", exts.size(), "\n");
                  for (auto& t : exts) {
                      uint16_t k = std::get<0>(t);
                      if (k == 0) { // bls
                          std::vector<char> vc = std::get<1>(t);
                          std::string blsData;
                          blsData.assign(vc.begin(), vc.end());
                          int verify = verify_header_extensions(chain_name, k, blsData.c_str(), blsData.length());
                          ultrainio_assert(verify == 0, "verify header ext error.");
                      }
                  }
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
            _subchains.modify(ite_chain, [&]( auto& _subchain ) {
                _subchain.recent_users.clear();
                _subchain.recent_users.shrink_to_fit();
            });
            return;
        }
        _subchains.modify(ite_chain, [&]( auto& _subchain ) {
            _subchain.recent_users.clear();
            _subchain.recent_users.shrink_to_fit();
            _subchain.head_block_id     = block_id_type();
            _subchain.head_block_num    = 0;
            _subchain.is_schedulable    = false;
            _subchain.committee_mroot   = checksum256();
            _subchain.chain_id          = checksum256();
        });
        print( "clearchain chain_name:", chain_name, " users_only:", users_only, "\n" );
        auto ite = _producers.begin();
        for(; ite != _producers.end(); ++ite) {
            if(ite->location == chain_name) {
                _producers.modify(ite, [&](auto & p) {
                     p.unpaid_blocks = 0;
                     p.total_produce_block = 0;
                });
            }
        }
    }

    void system_contract::empoweruser(account_name user, const std::string& owner_pk, const std::string& active_pk, uint64_t chain_name) {
        require_auth(user);
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "this subchian is not existed");
        ultrainio_assert(!is_empowered(user, chain_name), "user has been empowered to this chain before");
        bool is_prod = false;
        auto prod = _producers.find(user);
        if(prod == _producers.end()) {
            ultrainio_assert(owner_pk.size() == 53, "owner public key should be of size 53");
            ultrainio_assert(active_pk.size() == 53, "avtive public key should be of size 53");
        }
        else {
            is_prod = true;
        }
        //todo, check whether this subchain is active.

        empower_to_chain(user, chain_name);

        user_info tempuser;
        tempuser.user_name = user;
        tempuser.is_producer = is_prod;
        if(is_prod) {
            tempuser.owner_key = "";  //use pk of master mandatorily
            tempuser.active_key = "";
        }
        else {
            tempuser.owner_key = owner_pk;
            tempuser.active_key = active_pk;
        }
        tempuser.emp_time = now();

        _subchains.modify(ite_chain, [&]( auto& _chain ) {
            for(const auto& _user : _chain.recent_users) {
                ultrainio_assert(_user.user_name != user, "User has published in this subchain recently.");
            }
            _chain.recent_users.push_back(tempuser);
            _chain.total_user_num += 1;
        });
    }

    void system_contract::add_to_pending_queue(account_name producer, const std::string& public_key, const std::string& bls_key) {
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
        temp_node.bls_key   = bls_key;
        p_que->push_back(temp_node);
        uint16_t stable_min = 0;
        //loop all inactive subchain, start it if miners sum meet its requirement.
        auto ite_subchain = _subchains.begin();
        for(; ite_subchain != _subchains.end(); ++ite_subchain) {
            if(!ite_subchain->is_active) {
                chaintypes_table type_tbl(_self, _self);
                auto type_iter = type_tbl.find(ite_subchain->chain_type);
                ultrainio_assert(type_iter != type_tbl.end(), "chian type of subchian is not existed");
                if(type_iter->stable_min_producers <= p_que->size()) {
                    stable_min = type_iter->stable_min_producers;
                    break;
                }
            }
        }
        if(ite_subchain != _subchains.end()) {
            _subchains.modify(ite_subchain, [&](subchain& info) {
                //move miners from pending que to this sub chian
                uint32_t i = 0;
                auto ite_miner = p_que->begin();
                for(; ite_miner != p_que->end() && i <= stable_min; ++i) {
                    //update location of producer
                    auto ite_producer = _producers.find(ite_miner->owner);
                    ultrainio_assert( ite_producer != _producers.end(), "cannot find producer in database" );
                    _producers.modify(ite_producer, [&](auto & v) {
                        v.location = info.chain_name;
                    });
                    //move to subchain
                    info.committee_members.push_back(*ite_miner);
                    ite_miner = p_que->erase(ite_miner);
                }
                info.is_active = true;
            });
        }

        _pending_que.set(*p_que);
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
        _pending_que.set(_pending);
    }

    void system_contract::add_to_subchain(uint64_t chain_name, account_name producer, const std::string& public_key, const std::string& bls_key) {
        if(chain_name == default_chain_name) {
            //todo, loop all stable subchain, add to the one with least miners
            //todo, update location of producer
            return;
        }
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "The specific chain is not existed." );

        for(const auto& miner : ite_chain->committee_members) {
            if(miner.owner == producer) {
                _subchains.modify(ite_chain, [&](subchain& info) {
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
        _subchains.modify(ite_chain, [&](subchain& info) {
            role_base temp_node;
            temp_node.owner = producer;
            temp_node.producer_key   = public_key;
            temp_node.bls_key   = bls_key;
            info.changing_info.new_added_members.push_back(temp_node);
        } );
    }

    void system_contract::remove_from_subchain(uint64_t chain_name, account_name producer) {
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "The subchain is not existed.");

        //ultrainio_assert(ite_chain->committee_members.size() - 1 >= ite_chain->get_subchain_min_miner_num(),
        //                 "this subschain cannot remove producers");
        _subchains.modify(ite_chain, [&](subchain& info){
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
        int32_t period_minutes = 60;
        if(_schedsetting.exists()) {
            auto temp = _schedsetting.get();
            period_minutes = int32_t(temp.committee_confirm_period);
        }
        auto block_height = head_block_number() + 1;
        if(block_height > 180 && block_height%(period_minutes * 6) != 0 ) {
            return;
        }
        auto ite_chain = _subchains.begin();
        for(; ite_chain != _subchains.end(); ++ite_chain) {
            if(!ite_chain->updated_info.deprecated_committee.empty() ||
               !ite_chain->updated_info.unactivated_committee.empty() ||
               !ite_chain->changing_info.removed_members.empty() ||
               !ite_chain->changing_info.new_added_members.empty() ||
               ite_chain->updated_info.take_effect_at_block != 0 ) {
                _subchains.modify(ite_chain, [&]( auto& _subchain ) {
                    //remove removed producers from committee
                    auto removing_comm = _subchain.changing_info.removed_members.begin();
                    for(; removing_comm != _subchain.changing_info.removed_members.end(); ++removing_comm) {
                        auto current_comm = _subchain.committee_members.begin();
                        for(; current_comm != _subchain.committee_members.end();) {
                            if(current_comm->owner == removing_comm->owner) {
                                current_comm = _subchain.committee_members.erase(current_comm);
                                _subchain.is_schedulable = false;
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
                        _subchain.is_schedulable = false;
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

    struct chain_sched_info {
        uint16_t                          sched_out_num = 0;
        uint16_t                          gap_to_next_level = 0;
        uint8_t                           sched_random_factor = 0; //used for random order
        subchains_table::const_iterator   chain_ite;

        chain_sched_info(uint16_t out, uint16_t gap_to_nl, uint8_t random_factor, subchains_table::const_iterator ite):sched_out_num(out),gap_to_next_level(gap_to_nl),sched_random_factor(random_factor),chain_ite(ite) {}
    };

    bool compare_gt(const chain_sched_info& chain1, const chain_sched_info& chain2) {
        if((chain1.sched_out_num > chain2.sched_out_num) ||
            (chain1.sched_out_num == chain2.sched_out_num && chain1.gap_to_next_level < chain2.gap_to_next_level)) {
                return true;
        }
        return false;
    }

    void system_contract::schedule() {
        int32_t sched_period_minute = 60*24;
        if(_schedsetting.exists()) {
            auto temp = _schedsetting.get();
            if(!temp.is_schedule_enabled) {
                return;
            }
            sched_period_minute = int32_t(temp.schedule_period);
        }

        auto block_height = head_block_number() + 1;
        if( block_height%(sched_period_minute * 6) != 0) {
            return;
        }

        print( "[schedule] start, master block num: ", block_height, "\n");

        //get current head block hash
        char blockid[32];
        head_block_id(blockid, sizeof(blockid));
        checksum256 head_block_hash;
        memcpy(head_block_hash.hash, blockid, sizeof(blockid));

        chaintypes_table type_tbl(_self, _self);
        std::list<chain_sched_info> out_list;

        ////parepare all info
        uint32_t index_in_list = 0;
        auto chain_iter = _subchains.begin();
        for(; chain_iter != _subchains.end(); ++chain_iter, ++index_in_list) {
            auto type_iter = type_tbl.find(chain_iter->chain_type);
            if(type_iter == type_tbl.end()) {
                print("[schedule] error: the chain type is not existed\n");
                return;
            }
            //below cases will not take part in scheduling
            //1. producer sum doesn't reach stable_min
            //2. not schedulable: committee mroot did't update since last committee update
            //3. not synced: current block reported to master is far from the head block on subchain
            if((chain_iter->committee_members.size() < uint32_t(type_iter->stable_min_producers)) ||
               !(chain_iter->is_schedulable) || !(chain_iter->is_synced)) {
                continue;
            }
            auto out_num = (chain_iter->committee_members.size() - type_iter->stable_min_producers)/type_iter->sched_inc_step;
            auto gap = type_iter->sched_inc_step - (chain_iter->committee_members.size() - type_iter->stable_min_producers)%type_iter->sched_inc_step;
            uint8_t rf = head_block_hash.hash[31 - index_in_list % 30] + uint8_t(index_in_list / 30);//first 2 bytes are always 0, so here we use 30 =32 -2
            out_list.emplace_back(out_num, gap, rf, chain_iter);
        }

        print( "[schedule] out list size: ", out_list.size(), "\n" );
        if(out_list.size() < 2) {
            return;
        }

        //sort by committee sum
        out_list.sort(compare_gt);

        ////start 1st scheduling, this step is for balance
        auto min_sched_chain  = out_list.end();
        min_sched_chain--;

        print("[schedule] step 1:\n");
        auto out_iter = out_list.begin();
        for(; out_iter != min_sched_chain && compare_gt(*out_iter, *min_sched_chain) ; ++out_iter) {
            print("[schedule] out_chain: ", out_iter->chain_ite->chain_name);
            print(" can move out: ", uint32_t(out_iter->sched_out_num));
            print(" producers to in_chain: ", min_sched_chain->chain_ite->chain_name, "\n");
            for(uint16_t out_idx = 0; out_idx < out_iter->sched_out_num; ++out_idx ) {
                if(!move_producer(head_block_hash, out_iter->chain_ite, min_sched_chain->chain_ite, out_idx) ) {
                    continue;
                }
                --min_sched_chain->gap_to_next_level;
                if(0 == min_sched_chain->gap_to_next_level) {
                    --min_sched_chain; //one chain can only increase one level in a scheduling loop
                    if(min_sched_chain == out_iter) {
                        break;
                    }
                }
            }
        }
        //sort by random factor
        out_list.sort([](const chain_sched_info& chain1, const chain_sched_info& chain2) {
            return chain1.sched_random_factor < chain2.sched_random_factor;
        });
        ////start 2nd scheduling, this step is for security
        auto chain_from = out_list.begin();
        auto chain_to = out_list.begin();
        ++chain_to;
        print("[schedule] step 2:\n");
        for(; chain_to != out_list.end(); ++chain_from, ++chain_to) {
            print("[schedule] from_chain: ", chain_from->chain_ite->chain_name);
            print(", to_chain: ", chain_to->chain_ite->chain_name, "\n");
            if(!move_producer(head_block_hash, chain_from->chain_ite, chain_to->chain_ite, 0) ) {
                continue;
            }
        }
        chain_from = out_list.end();
        --chain_from;
        chain_to = out_list.begin();
        print("[schedule] from chain: ", chain_from->chain_ite->chain_name);
        print(", to chain: ", chain_to->chain_ite->chain_name, "\n");
        move_producer(head_block_hash, chain_from->chain_ite, chain_to->chain_ite, 0);
    }

    bool system_contract::move_producer(checksum256 head_id, subchains_table::const_iterator from_iter,
                                        subchains_table::const_iterator to_iter, uint16_t index) {
        index = index % 25;
        uint32_t x = uint32_t(head_id.hash[31 - index]);
        //filter the the producers which has been scheduled out in this loop.
        std::vector<account_name> schedule_producers;
        std::for_each(from_iter->committee_members.begin(), from_iter->committee_members.end(), [&](const role_base& prod){
            auto removing_iter = from_iter->changing_info.removed_members.begin();
            for(; removing_iter != from_iter->changing_info.removed_members.end(); ++removing_iter) {
                if(removing_iter->owner == prod.owner) {
                    break;
                }
            }
            if(removing_iter == from_iter->changing_info.removed_members.end()) {
                schedule_producers.push_back(prod.owner);
            }
        });
        //get a producder for scheduling out
        uint32_t totalsize = schedule_producers.size();
        auto producer = schedule_producers[x % totalsize];
        auto producer_iter = _producers.find(producer);
        if(producer_iter == _producers.end()) {
            print("[schedule] error: producer to move out is not found in _producers\n");
            return false;
        }
        print("[schedule] move ", name{producer}, " to ", to_iter->chain_name, "\n");
        //check user before move
        if(!is_empowered(producer, to_iter->chain_name)) {
            user_info tempuser;
            tempuser.user_name = producer;
            tempuser.is_producer = true;
            tempuser.emp_time = now();

            _subchains.modify(to_iter, [&]( auto& _chain ) {
                _chain.recent_users.push_back(tempuser);
                _chain.total_user_num += 1;
            });
            empower_to_chain(producer, to_iter->chain_name);
        }
        //move producer
        _producers.modify( producer_iter, [&]( producer_info& info ) {
               info.location     = to_iter->chain_name;
        });
        remove_from_subchain(from_iter->chain_name, producer);
        add_to_subchain(to_iter->chain_name, producer, producer_iter->producer_key, producer_iter->bls_key);

        return true;
    }

    void system_contract::setsched(bool is_enabled, uint16_t sched_period, uint16_t confirm_period) {
        require_auth(N(ultrainio));

        ultrainio_assert(sched_period <= 1440*7, "scheduling period is overlong, it's supposed at least once a week."); //60m*24h = minutes per day, perform scheduling at least once a week;
        ultrainio_assert(confirm_period <= 60, "committee update confirm time is overlong, the maximum time is 60 minutes");
        ultrainio_assert(confirm_period < sched_period, "committee update confirm time must lesser than scheduling period");

        schedule_setting temp;
        temp.is_schedule_enabled = is_enabled;
        temp.schedule_period = sched_period;
        temp.committee_confirm_period = confirm_period;
        _schedsetting.set(temp);
    }

    void system_contract::checkbulletin() {
        auto ct = now();
        auto chain_it = _subchains.begin();
        for(; chain_it != _subchains.end(); ++chain_it) {
            if(!chain_it->recent_users.empty()) {
                if( (ct > chain_it->recent_users[0].emp_time) && (ct - chain_it->recent_users[0].emp_time >= 30*60 ) ) {
                    _subchains.modify(chain_it, [&](auto& _subchain) {
                        auto user_it = _subchain.recent_users.begin();
                        for(; user_it != _subchain.recent_users.end();) {
                            if(ct > user_it->emp_time && (ct - user_it->emp_time >= 30*60)) {
                                user_it = _subchain.recent_users.erase(user_it);
                            }
                            else {
                                ++user_it;
                            }
                        }
                        _subchain.recent_users.shrink_to_fit();
                    });
                }
            }
        }
    }
} //namespace ultrainiosystem
