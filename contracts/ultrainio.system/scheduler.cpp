#include "ultrainio.system.hpp"
#include "confirm_point.h"
#include <list>

namespace ultrainiosystem {

    ///@abi action
    void system_contract::regchaintype(uint64_t type_id, uint16_t min_producer_num, uint16_t max_producer_num,
                                       uint16_t sched_step, uint16_t consensus_period) {
        require_auth(N(ultrainio));
        ultrainio_assert(_gstate.is_master_chain(), "only master chain can register chain type");
        ultrainio_assert(type_id != 0, "0 has been occupied by ultrain system");
        ultrainio_assert(min_producer_num >= 4, "wrong min_producer_num, at least 4 producers is required for a chain");
        ultrainio_assert(min_producer_num < max_producer_num, "max_producer_num must grater than min_producer_num");
        ultrainio_assert(sched_step > 1 && sched_step <= 100, "sched_step should in scope [2, 100]");
        ultrainio_assert(consensus_period > 1 && consensus_period <= 10, "consensus_period should in scope [2, 10]");
        chaintypes_table type_tbl(_self, _self);
        auto typeiter = type_tbl.find(type_id);
        if (typeiter == type_tbl.end()) {
            type_tbl.emplace( [&]( auto& new_chain_type ) {
                new_chain_type.type_id = type_id;
                new_chain_type.stable_min_producers = min_producer_num;
                new_chain_type.stable_max_producers = max_producer_num;
                new_chain_type.sched_inc_step = sched_step;
                new_chain_type.consensus_period = consensus_period;
            });
        } else {
            type_tbl.modify(typeiter, [&]( auto& _chain_type ) {
                _chain_type.stable_min_producers = min_producer_num;
                _chain_type.stable_max_producers = max_producer_num;
                _chain_type.sched_inc_step = sched_step;
                _chain_type.consensus_period = consensus_period;
            });
        }
    }
    ///@abi action
    void system_contract::reportsubchainhash(name subchain, uint64_t blocknum, checksum256 hash, uint64_t file_size) {
        auto sender = current_sender();
        producers_table _producers(_self, subchain);
        auto propos = _producers.find(sender);
        ultrainio_assert( propos != _producers.end(), "enabled producer not found this proposer" );

        uint32_t confirmed_blocknum = 0;
        uint32_t vote_threshold = 0;
        if (subchain == self_chain_name) {
            confirmed_blocknum = (uint32_t)head_block_number();
            vote_threshold = (uint32_t)ceil((double)_gstate.cur_committee_number * 2 / 3);
        } else {
            auto ite_chain = _chains.find(subchain);
            ultrainio_assert(ite_chain != _chains.end(), "subchain not found");
            confirmed_blocknum = ite_chain->confirmed_block_number;
            vote_threshold = (uint32_t)ceil((double)ite_chain->committee_num * 2 / 3);
        }

        auto checkBlockNum = [&](uint64_t  bn) -> bool {
            return (bn%_gstate.worldstate_interval) == 0;
        };
        ultrainio_assert(checkBlockNum(blocknum), "report an invalid blocknum ws");
        ultrainio_assert(blocknum <= confirmed_blocknum, "report a blocknum larger than current block");
        ultrainio_assert(confirmed_blocknum - blocknum <= _gstate.worldstate_interval, "report a too old blocknum");

        subchain_hash_table     hashTable(_self, subchain);

        auto wshash = hashTable.find(blocknum);
        if(wshash != hashTable.end()) {
            auto & hashv = wshash->hash_v;
            auto & acc   = wshash->accounts;
            auto ret = acc.find(sender);
            ultrainio_assert(ret == acc.end(), "the committee_members already report such ws hash");

            auto it = hashv.begin();
            for(; it != hashv.end(); it++) {
                if(hash == it->hash && file_size == it->file_size) {
                    ultrainio_assert(it->valid != true, "the hash value has been voted as valid");
                    if((it->votes+1) >= vote_threshold) {
                        //now let's make sure accounts voted for this hash all are current committee members
                        uint32_t cnt = 0;
                        std::set<account_name> del_accs;
                        for(auto& itac : it->accounts) {
                            auto vacc = _producers.find(itac);
                            if( vacc == _producers.end()) {
                                del_accs.emplace(itac);
                                cnt++;
                            }
                        }
                        for(auto& itac : del_accs) {
                            hashTable.modify(wshash, [&](auto &p) {
                                auto pos = static_cast<unsigned int>(it - hashv.begin());
                                p.hash_v[pos].votes--;
                                p.hash_v[pos].accounts.erase(itac);
                                p.accounts.erase(itac);
                            });
                        }
                        if(cnt == 0) {
                            hashTable.modify(wshash, [&](auto &p) {
                                auto pos = static_cast<unsigned int>(it - hashv.begin());
                                p.hash_v[pos].votes++;
                                p.hash_v[pos].valid = true;
                                p.hash_v[pos].accounts.emplace(sender);
                                p.accounts.emplace(sender);
                            });
                        } else {
                            hashTable.modify(wshash, [&](auto &p) {
                                auto pos = static_cast<unsigned int>(it - hashv.begin());
                                p.hash_v[pos].votes++;
                                p.hash_v[pos].accounts.emplace(sender);
                                p.accounts.emplace(sender);
                            });
                        }
                    } else {
                        hashTable.modify(wshash, [&](auto &p) {
                            auto pos = static_cast<unsigned int>(it - hashv.begin());
                            p.hash_v[pos].votes++;
                            p.hash_v[pos].accounts.emplace(sender);
                            p.accounts.emplace(sender);
                       });
                    }
                    break;
                }
            }
            if(it == hashv.end()) {
                hashTable.modify(wshash, [&](auto& p) {
                    p.hash_v.emplace_back(hash, file_size, 1, false, sender);
                    p.accounts.emplace(sender);
                });
            }
        }
        else {
            hashTable.emplace([&](auto &p) {
                p.block_num = blocknum;
                p.hash_v.emplace_back(hash, file_size, 1, false, sender);
                p.accounts.emplace(sender);
            });

            // delete item which is old enough but need to keep it if it is the only one with valid flag
            if (blocknum > uint64_t(NEAR_WS_CNT * _gstate.worldstate_interval)){
                uint64_t latest_valid = 0;
                uint64_t expired = blocknum - uint64_t(NEAR_WS_CNT * _gstate.worldstate_interval);
                uint64_t far_int = uint64_t(FAR_WS_MUL * _gstate.worldstate_interval);
                uint64_t far_val = uint64_t(FAR_WS_CNT * far_int);
                uint64_t far_exp = blocknum > far_val ? blocknum - far_val : 0;

                std::vector<uint64_t> expired_nums;

                for(auto itr = hashTable.begin(); itr != hashTable.end(); itr++) {
                    auto & hashv = itr->hash_v;
                    for(auto it = hashv.begin(); it != hashv.end(); it++) {
                        if(it->valid == true && itr->block_num > latest_valid) {
                            latest_valid = itr->block_num;
                            break;
                        }
                    }
                    if(itr->block_num <= expired) {
                        if( itr->block_num % far_int != 0 || itr->block_num <= far_exp ) {
                            expired_nums.push_back(itr->block_num);
                        }
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
    void system_contract::regsubchain(name chain_name, uint64_t chain_type, const std::string& genesis_producer_pubkey) {
        require_auth(N(ultrainio));
        ultrainio_assert(_gstate.is_master_chain(), "only master chain can register new chain");
        ultrainio_assert(chain_name != default_chain_name && chain_name != self_chain_name && chain_name != N(master), "unusable chian name");
        auto itor = _chains.find(chain_name);
        ultrainio_assert(itor == _chains.end(), "there has been a subchian with this name");
        chaintypes_table type_tbl(_self, _self);
        auto type_iter = type_tbl.find(chain_type);
        ultrainio_assert(type_iter != type_tbl.end(), "this chain type is not existed");
        ultrainio_assert(genesis_producer_pubkey.size() == 64, "subchain genesis producer public key should be of size 64");
        _chains.emplace( [&]( auto& new_subchain ) {
            new_subchain.chain_name         = chain_name;
            new_subchain.chain_type         = chain_type;
            new_subchain.is_synced          = false;
            new_subchain.is_schedulable     = false;
            new_subchain.schedule_on        = true;
            new_subchain.committee_num      = 0;
            new_subchain.total_user_num     = 0;
            new_subchain.chain_id           = checksum256();
            new_subchain.committee_mroot    = checksum256();
            new_subchain.confirmed_block_number = 0;
            new_subchain.confirmed_block_id = checksum256();
            new_subchain.table_extension.push_back( exten_type( chain_info::chains_state_exten_type_key::genesis_producer_public_key, genesis_producer_pubkey) );
        });
    }

    /// @abi action
    void system_contract::acceptmaster(const std::vector<ultrainio::signed_block_header>& headers) {
        ultrainio_assert(!_gstate.is_master_chain(), "master chain can not perform this action");
        acceptheader(name{N(master)}, headers);
    }

    bool system_contract::check_block_proposer(account_name block_proposer, chains_table::const_iterator chain_iter) {
        //find proposer and check if it should be paid for proposing this block.
        if(block_proposer != N(utrio.empty) && block_proposer != N(genesis)) {
            auto briefprod = _briefproducers.find(block_proposer);
            if(briefprod == _briefproducers.end()) {
                print("block proposer ", name{block_proposer}, " is not a valid producer\n");
                return false;
            }
            //1. find proposer in current committee
            producers_table _producers(_self, chain_iter->chain_name);
            auto prod = _producers.find(block_proposer);
            if(prod != _producers.end() ) {
                return true;
            }
            //2. if not found, find in deprecated committee(only for un-synced state)
            else if(!chain_iter->deprecated_committee.empty() ) {
                for(auto ite_prod = chain_iter->deprecated_committee.begin();
                    ite_prod != chain_iter->deprecated_committee.end(); ++ite_prod) {
                    if(ite_prod->owner == block_proposer) {
                         return true;
                    }
                }
            }
            //3. still not found, find in changing info
            for(auto ite_rm = chain_iter->changing_info.removed_members.begin();
            ite_rm != chain_iter->changing_info.removed_members.end(); ++ite_rm) {
                if(ite_rm->owner == block_proposer) {
                    uint32_t block_height = uint32_t(head_block_number() + 1);
                    uint16_t expire_time = 15;
                    if(_schedsetting.exists()) {
                        auto temp = _schedsetting.get();
                        expire_time = temp.expire_minutes;
                    }
                    if(block_height > ite_rm->block_num && block_height - ite_rm->block_num <= 6 * expire_time) {
                        return true;
                    } else {
                        print("producer ", name{block_proposer}, " moving out expired\n");
                        return false;
                    }
                }
            }
            print("error: block proposer ", name{block_proposer}, " is not found in the end\n");
        }
        return false;
    }
    /// @abi action
    void system_contract::acceptheader (name chain_name,
                                  const std::vector<ultrainio::signed_block_header>& headers) {
        ultrainio_assert(!headers.empty(), "at least one block should be contained");
        ultrainio_assert(headers.size() <= 10, "too many blocks are reported.");
        auto ite_chain = _chains.find(chain_name);
        ultrainio_assert(ite_chain != _chains.end(), "subchian is not existed.");
        name scope_name(chain_name);
        if(chain_name == N(master)) {
            scope_name = self_chain_name;
        }
        account_name sender = current_sender();
        producers_table _producer_chain(_self, scope_name);
        auto prod = _producer_chain.find(sender);
        if(_producer_chain.end() == prod) {
            auto ite_cmt = ite_chain->committee_set.begin();
            bool valid_sender = false;
            for(; ite_cmt != ite_chain->committee_set.end(); ++ite_cmt) {
                if(ite_cmt->owner == sender) {
                    valid_sender = true;
                    break;
                }
            }
            ultrainio_assert(valid_sender, "invalid sender");
        }
        bool synced = headers.size() == 10 ? false : true;
        uint32_t confirmed_number_before = ite_chain->confirmed_block_number;
        checksum256 final_confirmed_id;
        bool new_confirm = false;
        uint64_t initial_block_number = get_initial_block_num(chain_name);
        uint32_t block_number = 0;
        for(uint32_t idx = 0; idx < headers.size(); ++idx) {
            block_number = headers[idx].block_num();
            ultrainio_assert(block_number > ite_chain->confirmed_block_number, "block has been confirmed");
            auto block_id = headers[idx].id();
            const account_name block_proposer = headers[idx].proposer;
            if(block_proposer == N(genesis) && block_number > 1) {
                ultrainio_assert(headers[idx].signature.size() == 128, "signature should be of size 128");
            }

            //check if it has been accepted, if not, find its previous block
            uint32_t pre_block_index = find_previous_block(ite_chain->unconfirmed_blocks, block_number, block_id, headers[idx].previous);
            if(block_number > initial_block_number) {
                ultrainio_assert(pre_block_index < ite_chain->unconfirmed_blocks.size(), "previous block is not found\n");
            }

            bool need_report = chain_name == N(master) ? false : true;
            if((block_proposer == N(genesis) && block_number != initial_block_number) ||
                confirm_point::is_confirm_point(headers[idx])) {
                char confirm_id[32];
                ultrainio_assert(accept_block_header(chain_name, headers[idx], confirm_id, sizeof(confirm_id)), "light client check failed");
                memcpy(final_confirmed_id.hash, confirm_id, sizeof(confirm_id));
                uint32_t confirm_num = block_header::num_from_id(final_confirmed_id);
                if(confirm_num > ite_chain->confirmed_block_number) {
                    new_confirm = true;
                }
            }
            if(need_report) {
                need_report = check_block_proposer(block_proposer, ite_chain);
            }
            _chains.modify(ite_chain, [&]( auto& _subchain ) {
                unconfirmed_block_header uncfm_header(headers[idx], block_id, block_number, need_report, synced);
                if(block_number > initial_block_number) {
                    _subchain.unconfirmed_blocks[pre_block_index].is_leaf = false;
                }
                _subchain.unconfirmed_blocks.push_back(uncfm_header);

                if(!new_confirm) {
                    return;
                }
                handle_new_confirm_block(_subchain, final_confirmed_id);

            });
        }
        if (new_confirm && confirmed_number_before < ite_chain->confirmed_block_number ) {
            //handle all confirmed blocks
            rm_overdue_blocks(chain_name, confirmed_number_before, ite_chain->confirmed_block_number);
            _chains.modify(ite_chain, [&]( auto& _subchain ) {
                block_table subchain_block_tbl(_self, chain_name);
                auto ite_uncfm_block = _subchain.unconfirmed_blocks.begin();
                for(; ite_uncfm_block != _subchain.unconfirmed_blocks.end(); ) {
                    if(ite_uncfm_block->block_number > _subchain.confirmed_block_number) {
                        ++ite_uncfm_block;
                    }
                    // cannot filter fork blocks who have a follow-up block, we can only filter fork block as a leaf
                    else if((confirmed_number_before < ite_uncfm_block->block_number &&
                             ite_uncfm_block->block_number < _subchain.confirmed_block_number &&
                             !ite_uncfm_block->is_leaf) ||
                            (ite_uncfm_block->block_number == _subchain.confirmed_block_number &&
                             ite_uncfm_block->block_id == final_confirmed_id) ) {

                        //add to block table
                        auto block_ite = subchain_block_tbl.find(uint64_t(ite_uncfm_block->block_number));
                        if(block_ite != subchain_block_tbl.end()) {
                            //pay for the first one, since it's hard to distigush which is right
                            print("error: a block number is confirmed twice\n");
                        }
                        else{
                            if(ite_uncfm_block->to_be_paid) {
                                //apply reward for its proposer
                                report_subchain_block( ite_uncfm_block->proposer, ite_uncfm_block->block_number );
                            }
                            subchain_block_tbl.emplace([&]( auto& new_confirmed_header ) {
                                new_confirmed_header = block_header_digest(ite_uncfm_block->block_number,
                                                                      ite_uncfm_block->transaction_mroot);
                            });
                            if(1 == ite_uncfm_block->block_number) {
                                _subchain.chain_id          = ite_uncfm_block->action_mroot; //save chain id
                                _subchain.genesis_time      = ite_uncfm_block->timestamp;
                                _subchain.is_schedulable    = true;
                            }
                        }

                        if(ite_uncfm_block->block_number == _subchain.confirmed_block_number) {
                            //don't remove current confirmed block
                            ++ite_uncfm_block;
                        }
                        else {
                            ite_uncfm_block = _subchain.unconfirmed_blocks.erase(ite_uncfm_block);
                        }
                    }
                    else {
                        //confirmed_number_before(has been added before, but still kept in subchain) or fork branch, just remove it
                        ite_uncfm_block = _subchain.unconfirmed_blocks.erase(ite_uncfm_block);
                    }
                }
            });
        }
        ultrainio_assert(block_number - ite_chain->confirmed_block_number <= 100, "too many unconfirmed blocks");
    }

    void system_contract::clearchain(name chain_name, bool users_only) {
        require_auth(N(ultrainio));
        auto ite_chain = _chains.find(chain_name);
        ultrainio_assert(ite_chain != _chains.end(), "clearchain: this chian is not existed.");
        if(users_only) {
            _chains.modify(ite_chain, [&]( auto& _subchain ) {
                _subchain.recent_users.clear();
                _subchain.recent_users.shrink_to_fit();
            });
            return;
        }

        if(chain_name == N(master)) {
            master_chain_infos masterinfos(_self, _self);
            auto ite = masterinfos.begin();
            while(ite != masterinfos.end()) {
                ite = masterinfos.erase(ite);
            }
            _chains.erase(ite_chain);
        } else {
            _chains.modify(ite_chain, [&]( auto& _subchain ) {
                _subchain.recent_users.clear();
                _subchain.recent_users.shrink_to_fit();
                _subchain.is_synced         = false;
                _subchain.is_schedulable    = false;
                _subchain.total_user_num    = 0;
                _subchain.chain_id          = checksum256();
                _subchain.committee_mroot   = checksum256();
                _subchain.confirmed_block_number = 0;
                _subchain.confirmed_block_id = checksum256();
                _subchain.unconfirmed_blocks.clear();
                _subchain.unconfirmed_blocks.shrink_to_fit();
                _subchain.changing_info.clear();
                _subchain.committee_set.clear();
                _subchain.deprecated_committee.clear();
            });
        }
        print( "clearchain chain_name:", name{chain_name}, " users_only:", users_only, "\n" );
        block_table chain_block_tbl(_self, chain_name);
        auto it_blk = chain_block_tbl.begin();
        while (it_blk != chain_block_tbl.end()) {
            it_blk = chain_block_tbl.erase(it_blk);
        }
    }

    void system_contract::empoweruser(account_name user, name chain_name, const std::string& owner_pk, const std::string& active_pk, bool updateable) {
        if ( !has_auth(_self) ) {
           require_auth( user );
        }
        auto ite_chain = _chains.find(chain_name);
        ultrainio_assert(ite_chain != _chains.end(), "this subchian is not existed");
        ultrainio_assert(!is_empowered(user, chain_name), "user has been empowered to this chain before");
        auto briefprod = _briefproducers.find(user);
        bool is_prod = true;
        if(briefprod == _briefproducers.end()) {
            is_prod = false;
        }
        ultrainio_assert(owner_pk.size() == 53, "owner public key should be of size 53");
        ultrainio_assert(active_pk.size() == 53, "avtive public key should be of size 53");
        empower_to_chain(user, chain_name);
        user_info tempuser;
        tempuser.user_name = user;
        tempuser.is_producer = is_prod;
        tempuser.emp_time = now();
        tempuser.block_height = (uint64_t)head_block_number() + 1;
        tempuser.updateable = updateable;
        _chains.modify(ite_chain, [&]( auto& _chain ) {
            for(const auto& _user : _chain.recent_users) {
                ultrainio_assert(_user.user_name != user, "User has published in this chain recently.");
            }
            _chain.recent_users.push_back(tempuser);
            _chain.total_user_num += 1;
        });
    }

    void system_contract::add_to_chain(name chain_name, const producer_info& producer, uint64_t current_block_number) {
        uint64_t chain_block_height = current_block_number;
        if(chain_name != default_chain_name) {
            if (chain_name != self_chain_name) {
                auto ite_chain = _chains.find(chain_name);
                ultrainio_assert(ite_chain != _chains.end(), "destination chain is not existed" );
                chaintypes_table type_tbl(_self, _self);
                auto typeiter = type_tbl.find(ite_chain->chain_type);
                ultrainio_assert(typeiter != type_tbl.end(), "destination chain type is not existed");
                ultrainio_assert(ite_chain->committee_num < typeiter->stable_max_producers,
                    "destination chain already has enough producers");
                //check if this account has been empowered to this chain
                ultrainio_assert(is_empowered(producer.owner, chain_name), "account has not been empower to destination chain");
                _chains.modify(ite_chain, [&](chain_info& info) {
                    committee_info temp_prod;
                    temp_prod.owner = producer.owner;
                    temp_prod.producer_key = producer.producer_key;
                    temp_prod.bls_key = producer.bls_key;
                    info.changing_info.new_added_members.push_back(temp_prod);
                    info.is_schedulable = false;
                    info.committee_num += 1;
                });
                chain_block_height = ite_chain->confirmed_block_number;
            }else{
                _gstate.cur_committee_number++;
            }
        }

        producers_table _producer_chain(_self, chain_name);
        auto prod = _producer_chain.find( producer.owner );
        ultrainio_assert(prod == _producer_chain.end(), "producer has existed in destination chain");
        _producer_chain.emplace([&]( producer_info& _prod ) {
            _prod = producer;
            _prod.last_record_blockheight = chain_block_height;
            _prod.last_operate_blocknum = current_block_number;
            if(default_chain_name == chain_name) {
                _prod.table_extension.emplace_back(producer_info::producers_state_exten_type_key::enqueue_block_height, std::to_string(current_block_number));
            }
        });
    }

    void system_contract::remove_from_chain(name chain_name, account_name producer_name, uint64_t current_block_number) {
        producers_table _producer_chain(_self, chain_name);
        auto prod = _producer_chain.find( producer_name );
        ultrainio_assert(prod != _producer_chain.end(), "producer is not existed in source chain");

        if(chain_name != default_chain_name) {
            if (chain_name != self_chain_name) {
                auto ite_chain = _chains.find(chain_name);
                ultrainio_assert(ite_chain != _chains.end(), "source sidechain is not existed");
                chaintypes_table type_tbl(_self, _self);
                auto typeiter = type_tbl.find(ite_chain->chain_type);
                ultrainio_assert(typeiter != type_tbl.end(), "source chain type is not existed");
                ultrainio_assert(ite_chain->committee_num > typeiter->stable_min_producers,
                           "the producers in source sidechain is not enough for removing");

                _chains.modify(ite_chain, [&](chain_info& info) {
                    changing_producer temp_prod;
                    temp_prod.owner = prod->owner;
                    temp_prod.producer_key = prod->producer_key;
                    temp_prod.bls_key = prod->bls_key;
                    temp_prod.block_num = uint32_t(current_block_number);
                    info.changing_info.removed_members.push_back(temp_prod);
                    info.is_schedulable = false;
                    if(!info.is_synced) {
                        info.deprecated_committee.push_back(temp_prod);
                    }
                    info.committee_num -= 1;
                });
            }else{
                ultrainio_assert(_gstate.cur_committee_number > 0, "local chain cur_committee_number is abnormal");
                _gstate.cur_committee_number--;
            }
        }
        _producer_chain.erase(prod);
    }

    void system_contract::add_pending_producer(name chain_name, const committee_info& producer, uint32_t num) {
        print("[schedule] add pending producer ", name{producer.owner}, " to ", name{chain_name}, "\n");

        moveprod_param mv_prod(producer.owner, producer.producer_key, producer.bls_key, false, default_chain_name, false, chain_name);
        uint128_t sendid = _self + N(moveprod);
        sendid += num;
        cancel_deferred(sendid);
        ultrainio::transaction out;
        out.actions.emplace_back( permission_level{ _self, N(active) }, _self, NEX(moveprod), mv_prod );
        out.delay_sec = 0;
        out.send( sendid, _self, true );
    }

    struct chain_sched_info {
        uint16_t                          sched_out_num = 0;
        uint16_t                          gap_to_next_level = 0;
        uint8_t                           sched_random_factor = 0; //used for random order
        uint16_t                          inc_step = 1;
        chains_table::const_iterator   chain_ite;

        chain_sched_info(uint16_t out, uint16_t gap_to_nl, uint8_t random_factor, uint16_t is, chains_table::const_iterator ite)
         :sched_out_num(out),gap_to_next_level(gap_to_nl),sched_random_factor(random_factor),inc_step(is),chain_ite(ite) {}
    };

    bool compare_gt(const chain_sched_info& chain1, const chain_sched_info& chain2) {
        if((chain1.sched_out_num > chain2.sched_out_num) ||
            (chain1.sched_out_num == chain2.sched_out_num && chain1.gap_to_next_level < chain2.gap_to_next_level)) {
                return true;
        }
        return false;
    }

    void system_contract::pre_schedule() {
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
        auto chain_iter = _chains.begin();
        for(; chain_iter != _chains.end(); ++chain_iter, ++index_in_list) {
            if(chain_iter->chain_name == N(master))
                continue;
            auto type_iter = type_tbl.find(chain_iter->chain_type);
            if(type_iter == type_tbl.end()) {
                print("[schedule] error: the chain type is not existed\n");
                return;
            }
            //below cases will not take part in scheduling
            //1. producer sum doesn't reach stable_min
            //2. not schedulable: committee mroot did't update since last committee update
            //3. not synced: current block reported to master is far from the head block on chain
            //4. schedule is off
            if((chain_iter->committee_num < uint32_t(type_iter->stable_min_producers)) ||
               !(chain_iter->is_schedulable) || !(chain_iter->is_synced) || !(chain_iter->schedule_on)) {
                continue;
            }
            auto out_num = (chain_iter->committee_num - type_iter->stable_min_producers)/type_iter->sched_inc_step;
            auto gap = type_iter->sched_inc_step - (chain_iter->committee_num - type_iter->stable_min_producers)%type_iter->sched_inc_step;
            uint8_t rf = head_block_hash.hash[31 - index_in_list % 30] + uint8_t(index_in_list / 30);//first 2 bytes are always 0, so here we use 30 =32 -2
            out_list.emplace_back(out_num, gap, rf, type_iter->sched_inc_step, chain_iter);
        }

        print( "[schedule] out list size: ", out_list.size(), "\n" );

        ////start 1st scheduling, this step is for balance
        //if pending que has satisfied nodes, move them into chains in this step
        std::vector<committee_info> available_producers;
        producers_table pending_que(_self, default_chain_name);
        for(auto pending_prod = pending_que.begin(); pending_prod != pending_que.end(); ++pending_prod) {
            auto enqueue_blocknum = pending_prod->get_enqueue_block_height();
            ultrainio_assert(enqueue_blocknum > 0 && enqueue_blocknum < uint64_t(block_height), "error: wrong enqueue block height");
            auto interval_blocks = uint64_t(block_height) - enqueue_blocknum;
            auto queue_block = getglobalextenuintdata(ultrainio_global_state::pending_producer_queue_blocks, 259200);//30 days as default
            if(interval_blocks > queue_block ) {
                available_producers.emplace_back(pending_prod->owner, pending_prod->producer_key, pending_prod->bls_key);
            }
        }

        print("[schedule] ", available_producers.size(), " new producers in pending queue\n");

        auto scheduable_chain_num = out_list.size();
        if(!available_producers.empty()) {
            scheduable_chain_num += 1; //count pending queue in schedul
        }
        if(scheduable_chain_num < 2) {
            return;
        }
        uint32_t  sched_counter = 0;
        //sort by committee sum
        out_list.sort(compare_gt);
        if(!available_producers.empty()) {
          auto min_sched_chain  = out_list.end();
          --min_sched_chain;
          for(auto prod = available_producers.begin(); prod != available_producers.end(); ++prod, ++sched_counter) {
              add_pending_producer(min_sched_chain->chain_ite->chain_name, *prod, sched_counter);
              --min_sched_chain->gap_to_next_level;
              if(0 == min_sched_chain->gap_to_next_level) {
                  if(min_sched_chain == out_list.begin()) {
                      break;
                  }
                  --min_sched_chain;
              }
          }
        } else {
            auto min_sched_chain  = out_list.end();
            --min_sched_chain;

            print("[schedule] step 1:\n");
            auto out_iter = out_list.begin();
            for(; out_iter != min_sched_chain && out_iter != out_list.end() && compare_gt(*out_iter, *min_sched_chain); ++out_iter) {
                print("[schedule] out_chain: ", name{out_iter->chain_ite->chain_name});
                print(" can move out at most: ", uint32_t(out_iter->sched_out_num), " producers\n");
                bool quit_loop = false;
                for(uint16_t out_idx = 0; out_idx < out_iter->sched_out_num; ++out_idx ) {
                    if(!move_producer(head_block_hash, out_iter->chain_ite, min_sched_chain->chain_ite, uint64_t(block_height), sched_counter++) ) {
                        continue;
                    }
                    ++out_iter->gap_to_next_level;
                    --min_sched_chain->gap_to_next_level;
                    if(0 == min_sched_chain->gap_to_next_level) {
                        if(min_sched_chain == out_list.begin()) {
                            quit_loop = true;
                            break;
                        }
                        --min_sched_chain; //one chain can only increase one level in a scheduling loop
                        if(min_sched_chain == out_iter) {
                            quit_loop = true;
                            break;
                        }
                    }
                    if(out_iter->gap_to_next_level > out_iter->inc_step) {
                        out_iter->gap_to_next_level -= out_iter->inc_step;
                        if (out_iter->sched_out_num > 0 ) {
                           out_iter->sched_out_num -= 1;
                        }
                    }
                    if(!compare_gt(*out_iter, *min_sched_chain)) {
                        //get balanced
                        quit_loop = true;
                        break;
                    }
                }
                if(quit_loop) {
                    break;
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
            print("[schedule] from_chain: ", name{chain_from->chain_ite->chain_name});
            print(", to_chain: ", name{chain_to->chain_ite->chain_name}, "\n");
            if(!move_producer(head_block_hash, chain_from->chain_ite, chain_to->chain_ite, uint64_t(block_height), sched_counter++) ) {
                continue;
            }
        }
        chain_from = out_list.end();
        --chain_from;
        chain_to = out_list.begin();
        print("[schedule] from chain: ", name{chain_from->chain_ite->chain_name});
        print(", to chain: ", name{chain_to->chain_ite->chain_name}, "\n");
        move_producer(head_block_hash, chain_from->chain_ite, chain_to->chain_ite, uint64_t(block_height), sched_counter);
    }

    bool system_contract::move_producer(checksum256 head_id, chains_table::const_iterator from_iter,
                                        chains_table::const_iterator to_iter, uint64_t current_block_number, uint32_t num) {
        uint32_t i = num % 24;
        uint32_t x = uint32_t(head_id.hash[31 - i]);
        //filter the the producers which has been scheduled out in this loop.
        std::vector<account_name> schedule_producers;
        producers_table _producers(_self, from_iter->chain_name);
        auto prod = _producers.begin();
        for(; prod != _producers.end(); ++prod) {
            if(prod->last_operate_blocknum < current_block_number) {
                schedule_producers.push_back(prod->owner);
            }
        }
        if(schedule_producers.empty()) {
            print("[schedule] error: no movable producer\n");
            return false;
        }
        //get a producder for scheduling out
        uint32_t totalsize = schedule_producers.size();
        auto producer = schedule_producers[x % totalsize];
        if(!is_empowered(producer, to_iter->chain_name)) {
            print("[schedule] error: ", name{producer}, " is not empowered to chain ", name{to_iter->chain_name}, "\n");
            return false;
        }

        print("[schedule] ", num, " move ", name{producer}, " from ", name{from_iter->chain_name}, " to ", name{to_iter->chain_name}, "\n");
        prod = _producers.find(producer);
        if(prod == _producers.end()) {
            return false;
        }
        moveprod_param mv_prod(producer, prod->producer_key, prod->bls_key, false, from_iter->chain_name, false, to_iter->chain_name);
        uint128_t sendid = _self + N(moveprod);
        sendid += num;
        cancel_deferred(sendid);
        ultrainio::transaction out;
        out.actions.emplace_back( permission_level{ _self, N(active) }, _self, NEX(moveprod), mv_prod );
        out.delay_sec = 0;
        out.send( sendid, _self, true );

        return true;
    }

    void system_contract::setsched(bool is_enabled, uint16_t sched_period, uint16_t expire_time) {
        require_auth(N(ultrainio));

        ultrainio_assert(sched_period <= 1440*7, "scheduling period is overlong, it's supposed at least once a week."); //60m*24h = minutes per day, perform scheduling at least once a week;
        ultrainio_assert(expire_time <= 30, "committee update confirm time is overlong, its maximum time is 30 minutes");
        ultrainio_assert(expire_time < sched_period, "committee update confirm time must lesser than scheduling period");

        schedule_setting temp;
        temp.is_schedule_enabled = is_enabled;
        temp.schedule_period = sched_period;
        temp.expire_minutes = expire_time;
        _schedsetting.set(temp);
    }

    void system_contract::check_bulletin() {
        auto ct = now();
        uint64_t check_period = 30; //unit is minutes
        for(auto extension : _gstate.table_extension){
           if(extension.key == ultrainio_global_state::global_state_exten_type_key::check_user_bulletin) {
               std::string str = extension.value;
               check_period = uint64_t(std::stoi(str));
               break;
           }
        }
        auto chain_it = _chains.begin();
        for(; chain_it != _chains.end(); ++chain_it) {
            if(chain_it->chain_name == N(master))
                continue;
            if(chain_it->recent_users.empty()) {
                continue;
            }
            if( (ct > chain_it->recent_users[0].emp_time) && (ct - chain_it->recent_users[0].emp_time >= check_period*60 ) ) {
                _chains.modify(chain_it, [&](auto& _subchain) {
                    auto user_it = _subchain.recent_users.begin();
                    for(; user_it != _subchain.recent_users.end(); ) {
                        if(ct > user_it->emp_time && (ct - user_it->emp_time < 30*60)) {
                            break;
                        } else {
                            ++user_it;
                        }
                    }
                    if(user_it != _subchain.recent_users.begin()) {
                        _subchain.recent_users.erase(_subchain.recent_users.begin(), user_it);
                        _subchain.recent_users.shrink_to_fit();
                    }
                });
            }
        }
    }

    void system_contract::forcesetblock(name chain_name, const signed_block_header& signed_header, const std::vector<committee_info>& cmt_set) {
        //set confirm block of a chain
        require_auth(N(ultrainio));
        auto ite_chain = _chains.find(chain_name);
        ultrainio_assert(ite_chain != _chains.end(), "chain is not found");
        uint32_t current_confirmed_number = ite_chain->confirmed_block_number;
        uint32_t block_number = signed_header.block_num();
        _chains.modify(ite_chain, [&]( auto& _chain ) {
            auto block_id = signed_header.id();
            _chain.confirmed_block_number = block_number;
            _chain.confirmed_block_id = block_id;
            _chain.unconfirmed_blocks.clear();
            _chain.is_synced = false;
            _chain.is_schedulable = true;
            _chain.changing_info.clear();
            _chain.committee_mroot = signed_header.committee_mroot;
            if(cmt_set.size() >= 4 || signed_header.proposer == N(genesis)) {
                _chain.committee_set = cmt_set;
                _chain.committee_num = uint16_t(cmt_set.size());
            }

            unconfirmed_block_header temp_header(signed_header, block_id, block_number, false, false);
            _chain.unconfirmed_blocks.push_back(temp_header);
        });
        //handle block table of chain
        block_table chain_block_tbl(_self, chain_name);
        auto block_ite = chain_block_tbl.find(uint64_t(block_number));
        if(block_ite == chain_block_tbl.end()) {
            block_ite = chain_block_tbl.begin();
            for(; block_ite != chain_block_tbl.end();) {
                block_ite = chain_block_tbl.erase(block_ite);
            }
            chain_block_tbl.emplace([&]( auto& new_confirmed_header ) {
                new_confirmed_header = block_header_digest(block_number, signed_header.committee_mroot);
            });
        }
        else {
            uint32_t number_to_remove = block_number + 1;
            while(number_to_remove <= current_confirmed_number) {
                block_ite = chain_block_tbl.find(uint64_t(number_to_remove));
                if(block_ite != chain_block_tbl.end()) {
                    chain_block_tbl.erase(block_ite);
                }
                ++number_to_remove;
            }
        }
    }

    name system_contract::get_default_chain() {
        auto ite_chain = _chains.begin();
        auto ite_min = _chains.end();
        uint32_t max_gap = 0;
        chaintypes_table type_tbl(_self, _self);
        for(; ite_chain != _chains.end(); ++ite_chain) {
            if(!ite_chain->is_schedulable || !ite_chain->schedule_on) {
                continue;
            }
            if(ite_chain->chain_name == N(master))
                continue;
            uint32_t my_committee_num = ite_chain->committee_num;
            auto ite_type = type_tbl.find(ite_chain->chain_type);
            ultrainio_assert(ite_type != type_tbl.end(), "error: chain type is not existed");
            if(my_committee_num <=  ite_type->stable_max_producers ) {
                if (ite_type->stable_max_producers - my_committee_num > max_gap) {
                   max_gap = ite_type->stable_max_producers - my_committee_num;
                   ite_min = ite_chain;
                }
            }
        }

        //there's no schedulable or addable sidechain existed currently
        ultrainio_assert(ite_min != _chains.end(), "currently no available chain for new producer to join");
        return ite_min->chain_name;
    }

    uint32_t system_contract::find_previous_block(const std::vector<unconfirmed_block_header>& block_vct, uint32_t block_num,
                                                const block_id_type& block_id, const block_id_type& previous_id) {
        uint32_t pre_index = std::numeric_limits<uint32_t>::max();
        if(block_vct.empty()) {
            return pre_index;
        }
        uint32_t i = block_vct.size();
        //more efficient to find from the end of the vector
        do{
            --i;
            if(block_vct[i].block_number == block_num) {
                //duplicated block
                ultrainio_assert(block_vct[i].block_id != block_id, "block has been accepted");
            }
            if(block_vct[i].block_id == previous_id && block_num == block_vct[i].block_number + 1) {
                //previous block is found, duplicated block couldn't be added at front of it's previous
                pre_index = i;
                break;
            }
        }while(i > 0);
        return pre_index;
    }

    uint64_t system_contract::get_initial_block_num(name chain_name) {
        if(chain_name == N(master)) {
            master_chain_infos masterinfos(_self, _self);
            auto initmasteriter = masterinfos.find(N(ultrainio));
            ultrainio_assert(initmasteriter != masterinfos.end(), "master_chain_infos is not existed.");
            return initmasteriter->block_height;
        }
        return 1;
    }

    void system_contract::rm_overdue_blocks(name chain_name, uint32_t last_confirm_num, uint32_t confirm_num) {
        block_table subchain_block_tbl(_self, chain_name);
        //remove overdue blocks, only keep recent 30000 blocks
        if(confirm_num > _lwc.save_blocks_num) {
            uint32_t start_num = 0;
            if(last_confirm_num > _lwc.save_blocks_num) {
                start_num = last_confirm_num - _lwc.save_blocks_num;
            }
            auto sum = confirm_num - _lwc.save_blocks_num;
            for(uint32_t j = start_num; j < sum; ++j) {
                auto block_ite = subchain_block_tbl.find(uint64_t(j));
                if(block_ite != subchain_block_tbl.end()) {
                    subchain_block_tbl.erase(block_ite);
                }
            }
        }
    }

    void system_contract::handle_new_confirm_block(chain_info& _chain, const block_id_type& confirm_block_id) {
        auto ite_last_confirm_block = _chain.unconfirmed_blocks.begin();
        auto ite_confirm_block = _chain.unconfirmed_blocks.begin();
        for(; ite_confirm_block != _chain.unconfirmed_blocks.end(); ++ite_confirm_block) {
            if(ite_confirm_block->block_id == confirm_block_id) {
                break;
            }
            else if(ite_confirm_block->block_id == _chain.confirmed_block_id) {
                ite_last_confirm_block = ite_confirm_block;
            }
        }
        ultrainio_assert(ite_confirm_block != _chain.unconfirmed_blocks.end(), "error, confirm block is not found");

        if (_chain.confirmed_block_number == ite_confirm_block->block_number) {
            return;
        }
        //handle new confirmed block
        auto ite_block = ite_last_confirm_block;
        do{
            ++ite_block;
            if(ite_block == _chain.unconfirmed_blocks.end()) {
                break;
            }
            if(ite_block->block_number > ite_confirm_block->block_number ||
               ite_block->block_number <= ite_last_confirm_block->block_number) {
                continue;
            }
            if(ite_block->is_leaf && ite_block->block_number < ite_confirm_block->block_number) {
                continue;
            }
            if(ite_block->committee_mroot != _chain.committee_mroot) {
                auto new_committee_set = ite_block->get_committee_set();
                if(new_committee_set.empty()) {
                    print("error: committee mroot changed but new committee set is empty\n");
                } else {
                    //get committee delta
                    committee_set pre_committee_set(_chain.committee_set);
                    auto cmt_delta = new_committee_set.diff(pre_committee_set);
                    _chain.handle_committee_update(cmt_delta);
                    new_committee_set.swap(_chain.committee_set);
                    if(_chain.is_schedulable) {
                        clear_committee_bulletin(_chain.chain_name);
                    }
                }
                _chain.committee_mroot = ite_block->committee_mroot;
            }
        } while(ite_block != ite_confirm_block);
        _chain.confirmed_block_number = ite_confirm_block->block_number;
        _chain.confirmed_block_id = ite_confirm_block->block_id;
        if(!_chain.is_synced && ite_confirm_block->is_synced && !_chain.deprecated_committee.empty()) {
            _chain.deprecated_committee.clear();
        }
        _chain.is_synced = ite_confirm_block->is_synced;
    }

    void system_contract::setlwcparams(uint32_t keep_blocks_num) {
        require_auth(N(ultrainio));
        _lwc.save_blocks_num = keep_blocks_num;
        _lwcsingleton.set(_lwc);
    }

    void system_contract::setchainparam(name chain_name, uint64_t chain_type, bool is_sched_on) {
        require_auth(N(ultrainio));
        ultrainio_assert(_gstate.is_master_chain(), "only master chain can perform this action");
        auto ite_chain = _chains.find(chain_name);
        ultrainio_assert(ite_chain != _chains.end(), "chain is not found");
        if(chain_type != ite_chain->chain_type) {
            chaintypes_table type_tbl(_self, _self);
            auto type_iter = type_tbl.find(chain_type);
            ultrainio_assert(type_iter != type_tbl.end(), "this chain type is not existed");
        }
        _chains.modify(ite_chain, [&]( auto& _subchain ) {
            _subchain.schedule_on = is_sched_on;
            _subchain.chain_type = chain_type;
        });
    }

    void system_contract::clear_committee_bulletin(name chain_name) {
        cmtbulletin  cb_tbl(_self, chain_name);
        auto record = cb_tbl.begin();
        while(record != cb_tbl.end()) {
            record = cb_tbl.erase(record);
        }
    }
} //namespace ultrainiosystem
