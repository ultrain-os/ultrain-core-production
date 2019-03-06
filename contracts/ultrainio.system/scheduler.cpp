#include "ultrainio.system.hpp"
#include <list>

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
        auto ite_chain = _subchains.find(subchain);
        ultrainio_assert(ite_chain != _subchains.end(), "subchain not found");
        producers_table _producers(_self, subchain);
        auto propos = _producers.find(current_sender());

        ultrainio_assert( propos != _producers.end() && propos->location == subchain, "enabled producer not found this proposer" );
        auto checkBlockNum = [&](uint64_t  bn) -> bool {
            return (bn%default_worldstate_interval) == 0;
        };
        ultrainio_assert(checkBlockNum(blocknum), "report an invalid blocknum ws");

        ultrainio_assert(blocknum <= ite_chain->confirmed_block_number, "report a blocknum larger than current block");
        ultrainio_assert(ite_chain->confirmed_block_number - blocknum <= default_worldstate_interval, "report a too old blocknum");

        subchain_hash_table     hashTable(_self, subchain);

        auto wshash = hashTable.find(blocknum);
        if(wshash != hashTable.end()) {
            auto & hashv = wshash->hash_v;
            auto & acc   = wshash->accounts;
            auto ret = std::find(acc.begin(), acc.end(), current_sender());
            ultrainio_assert(ret == acc.end(), "the committee_members already report such ws hash");

            uint32_t  pro_cnt = 0;
            for(auto itr = _producers.begin(); itr != _producers.end(); itr++) {
                if( itr->location == subchain ) {
                    pro_cnt++;
                }
            }

            auto it = hashv.begin();
            for(; it != hashv.end(); it++) {
                if(hash == it->hash && file_size == it->file_size) {
                    if((it->votes+1) >= ceil((double)pro_cnt*2/3)) {
                        hashTable.modify(wshash, [&](auto &p) {
                            p.hash_v[static_cast<unsigned int>(it - hashv.begin())].votes++;
                            p.hash_v[static_cast<unsigned int>(it - hashv.begin())].valid = true;
                            p.accounts.emplace_back(current_sender());
                            });
                    } else {
                        hashTable.modify(wshash, [&](auto &p) {
                            p.hash_v[static_cast<unsigned int>(it - hashv.begin())].votes++;
                            p.accounts.emplace_back(current_sender());
                            });
                    }
                    break;
                }
            }
            if(it == hashv.end()) {
                hashTable.modify(wshash, [&](auto& p) {
                    p.hash_v.emplace_back(hash, file_size, 1, false);
                    p.accounts.emplace_back(current_sender());
                });
            }
        }
        else {
            hashTable.emplace([&](auto &p) {
                    p.block_num = blocknum;
                    p.hash_v.emplace_back(hash, file_size, 1, false);
                    p.accounts.emplace_back(current_sender());
                    });

            // delete item which is old enough but need to keep it if it is the only one with valid flag
            if (blocknum > uint64_t(MAX_WS_COUNT * default_worldstate_interval)){
                uint64_t latest_valid = 0;
                uint64_t expired = blocknum - uint64_t(MAX_WS_COUNT * default_worldstate_interval);
                std::vector<uint64_t> expired_nums;

                for(auto itr = hashTable.begin(); itr != hashTable.end(); itr++) {
                    auto & hashv = itr->hash_v;
                    for(auto it = hashv.begin(); it != hashv.end(); it++) {
                        if(it->valid == true && itr->block_num > latest_valid) {
                            latest_valid = itr->block_num;
                        }
                    }
                    if(itr->block_num <= expired) {
                        expired_nums.push_back(itr->block_num);
                    }
                }
                for(auto exp : expired_nums) {
                    auto old = hashTable.find(exp);
                    if (old != hashTable.end() && (latest_valid == 0 || exp != latest_valid)) {
                        hashTable.erase(old);
                    }
                }
            }
        }
    }
    /// @abi action
    void system_contract::regsubchain(uint64_t chain_name, uint64_t chain_type) {
        require_auth(N(ultrainio));
        ultrainio_assert(chain_name != default_chain_name, "subchian cannot named as default.");
        ultrainio_assert(chain_name != master_chain_name, "subchian cannot named as 0");
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
            new_subchain.updated_info.take_effect_at_block = 0;
            new_subchain.is_schedulable    = false;
            new_subchain.total_user_num    = 0;
            new_subchain.chain_id          = checksum256();
            new_subchain.committee_mroot   = checksum256();
            new_subchain.confirmed_block_number    = 0;
            new_subchain.highest_block_number    = 0;
        });
    }

    /// @abi action
    void system_contract::acceptheader (uint64_t chain_name,
                                  const std::vector<ultrainio::block_header>& headers) {
        require_auth(current_sender());
        ultrainio_assert(headers.size() <= 10, "too many blocks are reported.");
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "subchian is not existed.");
        bool synced = headers.size() == 10 ? false : true;
        //todo, check if the subchain is avtive?
        for(uint32_t idx = 0; idx < headers.size(); ++idx) {
            bool has_accepted = false;
            auto block_number = headers[idx].block_num();
            if(block_number <= ite_chain->confirmed_block_number) {
                //ignore blocks whose number has been confirmed
                continue;
            }
            auto block_id = headers[idx].id();
            uint16_t pre_id = 0;
            auto ite_parent_block = ite_chain->unconfirmed_blocks.end();

            //check if it has been accepted, if not, find its previous block
            auto ite_block = ite_chain->unconfirmed_blocks.begin();
            for(; ite_block != ite_chain->unconfirmed_blocks.end(); ++ite_block) {
                if(ite_block->block_id == block_id && block_number == ite_block->block_number) {
                    has_accepted = true;
                    break;
                }
                else if(ite_block->block_id == headers[idx].previous && block_number == ite_block->block_number + 1) {
                    //found previous block
                    pre_id = ite_block->fork_id;
                    ite_parent_block = ite_block;
                }
            }
            if(has_accepted) {
                continue;
            }

            //find proposer and check if it should be paid for proposing this block.
            account_name block_proposer = headers[idx].proposer;
            bool found_prod = false;
            bool need_report = true;
            if(block_proposer != N() && block_proposer != N(genesis)) {
                producers_table _producers(_self, chain_name);
                auto prod = _producers.find(block_proposer);
                ultrainio_assert(prod != _producers.end(), "the prososer is not a valid producer");
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
                    //no reward for the producer. assert is not suggested to be used here
                    need_report = false;
                }
            }
            else {
                need_report = false; //Don't pay for null block and all blocks produced by genesis
            }

            // veirfy block header ext
            ultrainio::extensions_type exts = headers[idx].header_extensions;
            print("acceptheader verify block header exts.size : ", exts.size(), "\n");
            for (const auto& t : exts) {
                uint16_t k = std::get<0>(t);
                if (k == 0) { // bls
                    std::vector<char> vc = std::get<1>(t);
                    std::string blsData;
                    blsData.assign(vc.begin(), vc.end());
//                    int verify = verify_header_extensions(chain_name, k, blsData.c_str(), blsData.length());
//                    ultrainio_assert(verify == 0, "verify header ext error.");
                }
            }

            block_header_digest confirmed_header;
            //add block head to table
            if(1 == block_number) {
                //same block case has been filtered above, so the genesis block should be the first one in this subchain
                ultrainio_assert(ite_chain->unconfirmed_blocks.empty(), "a sidechain can only accept one genesis block");
                _subchains.modify(ite_chain, [&]( auto& _subchain ) {
                    _subchain.chain_id          = headers[idx].action_mroot; //save chain id
                    _subchain.genesis_time      = headers[idx].timestamp;
                    _subchain.is_schedulable    = true;
                    _subchain.confirmed_block_number = 1;
                    _subchain.highest_block_number = 1;

                    unconfirmed_block_header temp_header;
                    temp_header.fork_id       = 0x0001;
                    temp_header.block_id      = block_id;
                    temp_header.block_number  = 1;
                    temp_header.to_be_paid    = need_report;
                    temp_header.is_leaf       = true;
                    temp_header.proposer      = headers[idx].proposer;
                    temp_header.committee_mroot = headers[idx].committee_mroot;
                    temp_header.transaction_mroot = headers[idx].transaction_mroot;
                    _subchain.unconfirmed_blocks.push_back(temp_header);
                });

                confirmed_header.proposer          = headers[idx].proposer;
                confirmed_header.block_id          = block_id;
                confirmed_header.block_number      = 1;
                confirmed_header.transaction_mroot = headers[idx].transaction_mroot;
            }
            else {
                ultrainio_assert(pre_id != 0 && ite_parent_block != ite_chain->unconfirmed_blocks.end(), "previous block is not found");
                _subchains.modify(ite_chain, [&]( auto& _subchain ) {
                    if(idx == headers.size() - 1) {
                         _subchain.is_synced         = synced;
                    }

                    uint16_t my_sequence = 1;
                    if(_subchain.highest_block_number >= block_number ) {
                        //fork happened, find right id of current block
                        uint16_t temp_id = my_sequence + uint16_t(pre_id << 4);
                        auto ite_block_s = ++ite_parent_block;//loop from it's parent, children are all behind of their previous block
                        for(; ite_block_s != _subchain.unconfirmed_blocks.end(); ++ite_block_s) {
                            if(ite_block_s->fork_id == temp_id) {
                                ++my_sequence;
                                ultrainio_assert(my_sequence < 16, "too many forks"); //hex can only support maximum 15 forks
                                ++temp_id;
                            }
                        }
                    }
                    else{
                        _subchain.highest_block_number = block_number;
                    }

                    //need update latest confirm block
                    if(pre_id > 0x1110) {
                        uint16_t confirm_id = (pre_id & 0x0F00) >> 8;
                        if(block_number > 4) {
                            _subchain.confirmed_block_number = block_number - 3;
                        }
                        auto ite_confirm_block = _subchain.unconfirmed_blocks.end();
                        //update all blocks' id, and erase obsolete blocks, also get the confirm block info
                        auto ite_block_a = _subchain.unconfirmed_blocks.begin();
                        for(; ite_block_a != _subchain.unconfirmed_blocks.end(); ) {
                            uint16_t x_id = 0;
                            if(ite_block_a->fork_id <= 0x000F) {
                                //need removed
                            }
                            else if(ite_block_a->fork_id <= 0x00FF) {
                                ite_block_a->fork_id &= 0x000F;
                                if(ite_block_a->fork_id == confirm_id) {
                                    ite_confirm_block = ite_block_a;
                                }
                                  x_id = ite_block_a->fork_id;
                            }
                            else if(ite_block_a->fork_id <= 0x0FFF) {
                                ite_block_a->fork_id &= 0x00FF;
                                x_id = (ite_block_a->fork_id & 0x00F0) >> 4;
                            }
                            else{
                                if(pre_id == ite_block_a->fork_id) {
                                    ite_block_a->is_leaf = false;
                                }
                                ite_block_a->fork_id &= 0x0FFF;
                                x_id = (ite_block_a->fork_id & 0x0F00) >> 8;
                            }

                            //remove all children if it's not derived from current confirm block
                            if(x_id != confirm_id) {
                                ite_block_a = _subchain.unconfirmed_blocks.erase(ite_block_a);
                            }
                            else {
                                ++ite_block_a;
                            }
                        }
                        ultrainio_assert(ite_confirm_block != _subchain.unconfirmed_blocks.end(), "error, confirm block is not found");

                        _subchain.confirmed_block_number = ite_confirm_block->block_number;
                        if(ite_confirm_block->committee_mroot != _subchain.committee_mroot) {
                            _subchain.committee_mroot = ite_confirm_block->committee_mroot;
                            _subchain.is_schedulable  = true;
                        }
                        //record proposer for rewards
                        if(ite_confirm_block->to_be_paid) {
                            reportblocknumber( ite_chain->chain_name, ite_chain->chain_type, ite_confirm_block->proposer, 1);
                        }
                        //save confirmed block
                        confirmed_header.proposer          = ite_confirm_block->proposer;
                        confirmed_header.block_id          = ite_confirm_block->block_id;
                        confirmed_header.block_number      = ite_confirm_block->block_number;
                        confirmed_header.transaction_mroot = ite_confirm_block->transaction_mroot;
                    }
                    else {
                        auto ite_block_b = _subchain.unconfirmed_blocks.begin();
                        for(; ite_block_b != _subchain.unconfirmed_blocks.end(); ++ite_block_b) {
                            if(ite_block_b->fork_id == pre_id) {
                                ite_block_b->is_leaf = false;
                                break;
                            }
                        }
                    }
                    //add in current block
                    unconfirmed_block_header temp_header;
                    temp_header.fork_id       = my_sequence + uint16_t(pre_id << 4);
                    temp_header.block_id      = block_id;
                    temp_header.block_number  = block_number;
                    temp_header.to_be_paid    = need_report;
                    temp_header.is_leaf       = true;
                    temp_header.proposer      = headers[idx].proposer;
                    temp_header.committee_mroot = headers[idx].committee_mroot;
                    temp_header.transaction_mroot = headers[idx].transaction_mroot;
                    _subchain.unconfirmed_blocks.push_back(temp_header);

                    _subchain.unconfirmed_blocks.shrink_to_fit();
                });
            }
            if(confirmed_header.block_number != 0){
                block_table subchain_block_tbl(_self, chain_name);
                auto block_ite = subchain_block_tbl.find(uint64_t(confirmed_header.block_number));
                ultrainio_assert(block_ite == subchain_block_tbl.end(), "a block number can not be confirmed twice");
                if(confirmed_header.block_number > 1000) {
                    block_ite = subchain_block_tbl.find(uint64_t(confirmed_header.block_number - 1000));
                    if(block_ite != subchain_block_tbl.end()) {
                        subchain_block_tbl.erase(block_ite);
                    }
                }
                subchain_block_tbl.emplace([&]( auto& new_confirmed_header ) {
                    new_confirmed_header = confirmed_header;
                });
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
            _subchain.is_synced         = false;
            _subchain.is_schedulable    = false;
            _subchain.total_user_num    = 0;
            _subchain.chain_id          = checksum256();
            _subchain.committee_mroot   = checksum256();
            _subchain.confirmed_block_number = 0;
            _subchain.highest_block_number = 0;
            _subchain.unconfirmed_blocks.clear();
            _subchain.unconfirmed_blocks.shrink_to_fit();
        });
        print( "clearchain chain_name:", chain_name, " users_only:", users_only, "\n" );
        producers_table _producers(_self, chain_name);
        auto ite = _producers.begin();
        for(; ite != _producers.end(); ++ite) {
            if(ite->location == chain_name) {
                _producers.modify(ite, [&](auto & p) {
                     p.unpaid_balance = 0;
                     p.total_produce_block = 0;
                });
            }
        }
    }

    void system_contract::empoweruser(account_name user, uint64_t chain_name, const std::string& owner_pk, const std::string& active_pk) {
        require_auth(user);
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "this subchian is not existed");
        ultrainio_assert(!is_empowered(user, chain_name), "user has been empowered to this chain before");
        auto briefprod = _briefproducers.find(user);
        bool is_prod = true;
        if(briefprod == _briefproducers.end()) {
            is_prod = false;
            ultrainio_assert(owner_pk.size() == 53 || owner_pk.size() == 0, "owner public key should be of size 53 or empty");
            ultrainio_assert(active_pk.size() == 53 || active_pk.size() == 0, "avtive public key should be of size 53 or empty");
        }
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

    void system_contract::add_to_chain(uint64_t chain_name, const producer_info& producer) {
        if (chain_name != master_chain_name) {
            auto ite_chain = _subchains.find(chain_name);
            ultrainio_assert(ite_chain != _subchains.end(), "destination sidechain is not existed" );
            chaintypes_table type_tbl(_self, _self);
            auto typeiter = type_tbl.find(ite_chain->chain_type);
            ultrainio_assert(typeiter != type_tbl.end(), "destination sidechain type is not existed");
            ultrainio_assert(ite_chain->committee_sum < typeiter->stable_max_producers,
                "destination sidechain already has enough producers");
            _subchains.modify(ite_chain, [&](subchain& info) {
                role_base temp_prod;
                temp_prod.owner = producer.owner;
                temp_prod.producer_key = producer.producer_key;
                temp_prod.bls_key = producer.bls_key;
                info.changing_info.new_added_members.push_back(temp_prod);
                info.committee_sum += 1;
            });
        }

        producers_table _producer_chain(_self, chain_name);
        auto prod = _producer_chain.find( producer.owner );
        ultrainio_assert(prod == _producer_chain.end(), "producer has existed in destination chain");
        _producer_chain.emplace([&]( producer_info& _prod ) {
            _prod = producer;
            _prod.location = chain_name;
            //TODO, record operation block for manual action
        });
    }

    void system_contract::remove_from_chain(uint64_t chain_name, account_name producer_name) {
        producers_table _producer_chain(_self, chain_name);
        auto prod = _producer_chain.find( producer_name );
        ultrainio_assert(prod != _producer_chain.end(), "producer is not existed in source chain");

        if (chain_name != master_chain_name) {
            auto ite_chain = _subchains.find(chain_name);
            ultrainio_assert(ite_chain != _subchains.end(), "source sidechain is not existed");
            chaintypes_table type_tbl(_self, _self);
            auto typeiter = type_tbl.find(ite_chain->chain_type);
            ultrainio_assert(typeiter != type_tbl.end(), "source subchain type is not existed");
            ultrainio_assert(ite_chain->committee_sum > typeiter->stable_min_producers,
                "the producers in source sidechain is not enough for removing");

            _subchains.modify(ite_chain, [&](subchain& info) {
                role_base temp_prod;
                temp_prod.owner = prod->owner;
                temp_prod.producer_key = prod->producer_key;
                temp_prod.bls_key = prod->bls_key;
                info.changing_info.removed_members.push_back(temp_prod);
                info.committee_sum -= 1;
            });
        }
        _producer_chain.erase(prod);
    }

    void system_contract::activate_committee_update() {
        int32_t period_minutes = 60;
        if(_schedsetting.exists()) {
            auto temp = _schedsetting.get();
            period_minutes = int32_t(temp.committee_confirm_period);
        }
        int32_t curblocknum = (int32_t)head_block_number() + 1;
        if(curblocknum > 180 && curblocknum%(period_minutes * 6) != 0 ) {
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

        auto block_height = int32_t(head_block_number() + 1);
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
        producers_table _producers(_self, from_iter->chain_name);
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
        auto briefprod = _briefproducers.find(producer);
        if(briefprod != _briefproducers.end()) {
            add_to_chain(to_iter->chain_name, *producer_iter);
            remove_from_chain(from_iter->chain_name, producer);
            _briefproducers.modify(briefprod, [&](producer_brief& producer_brf) {
                producer_brf.location = to_iter->chain_name;
            });
        }

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

    void system_contract::forcesetblock(uint64_t chain_name, block_header_digest header_dig, checksum256 committee_mrt) {
        //set confirm block of subchain
        require_auth(N(ultrainio));
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "subchain not found");
        uint32_t current_confirmed_number = ite_chain->confirmed_block_number;
        _subchains.modify(ite_chain, [&]( auto& _subchain ) {
            _subchain.confirmed_block_number = header_dig.block_number;
            _subchain.highest_block_number   = header_dig.block_number;
            _subchain.unconfirmed_blocks.clear();
            _subchain.is_synced = false;
            _subchain.is_schedulable = true;

            unconfirmed_block_header temp_header;
            temp_header.fork_id       = 0x0001;
            temp_header.block_id      = header_dig.block_id;
            temp_header.block_number  = header_dig.block_number;
            temp_header.to_be_paid    = false;
            temp_header.is_leaf       = true;
            temp_header.proposer      = header_dig.proposer;
            temp_header.committee_mroot = committee_mrt;
            temp_header.transaction_mroot = header_dig.transaction_mroot;
            _subchain.unconfirmed_blocks.push_back(temp_header);
        });
        //handle block table of subchain
        block_table subchain_block_tbl(_self, chain_name);
        auto block_ite = subchain_block_tbl.find(uint64_t(header_dig.block_number));
        if(block_ite == subchain_block_tbl.end()) {
            block_ite = subchain_block_tbl.begin();
            for(; block_ite != subchain_block_tbl.end();) {
                block_ite = subchain_block_tbl.erase(block_ite);
            }
            subchain_block_tbl.emplace([&]( auto& new_confirmed_header ) {
                new_confirmed_header = header_dig;
            });
        }
        else {
            uint32_t number_to_remove = header_dig.block_number + 1;
            while(number_to_remove <= current_confirmed_number) {
                block_ite = subchain_block_tbl.find(uint64_t(number_to_remove));
                if(block_ite != subchain_block_tbl.end()) {
                    subchain_block_tbl.erase(block_ite);
                }
            }
        }
    }

    uint64_t system_contract::getdefaultchain() {
        auto ite_chain = _subchains.begin();
        auto ite_min = _subchains.end();
        uint32_t max_gap = 0;
        chaintypes_table type_tbl(_self, _self);
        for(; ite_chain != _subchains.end(); ++ite_chain) {
            if(!ite_chain->is_schedulable) {
                continue;
            }
            uint32_t my_committee_num = ite_chain->committee_sum;
            auto ite_type = type_tbl.find(ite_chain->chain_type);
            ultrainio_assert(ite_type != type_tbl.end(), "error: chain type is not existed");
            if(my_committee_num <=  ite_type->stable_max_producers ) {
                if (ite_type->stable_max_producers - my_committee_num > max_gap) {
                   max_gap = ite_type->stable_max_producers - my_committee_num;
                   ite_min = ite_chain;
                }
            }
        }
        if(ite_min == _subchains.end()) {
            //there's no schedulable or addable sidechain existed currently
            return master_chain_name;
        }
        else {
            return ite_min->chain_name;
        }
    }
} //namespace ultrainiosystem
