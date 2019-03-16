#include "ultrainio.system.hpp"
#include <list>

namespace ultrainiosystem {

    struct committee_delta {
        std::set<account_name>    removed;
        std::set<account_name>    added;

        committee_delta(const std::vector<account_name>& previous, const std::vector<account_name>& current):
        removed(previous.begin(), previous.end()), added(current.begin(), current.end()) {
            workout_delta();
        }

        void workout_delta() {
            //erase all same items, the left in previous set are all removed, and the left in current set are all new added
            for (auto it_pre = removed.begin(); it_pre != removed.end();) {
                auto it_cur = added.find(*it_pre);
                if(it_cur != added.end()) {
                    it_pre = removed.erase(it_pre);
                    added.erase(it_cur);
                }
                else {
                    ++it_pre;
                }
            }
        }
    };

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
    void system_contract::reportsubchainhash(name subchain, uint64_t blocknum, checksum256 hash, uint64_t file_size) {
        require_auth(current_sender());

        uint32_t confirmed_blocknum = 0;
        uint32_t vote_threshold = 0;
        if (subchain == master_chain_name) {
            confirmed_blocknum = (uint32_t)head_block_number();
            vote_threshold = (uint32_t)ceil((double)_gstate.cur_committee_number * 2 / 3);
        } else {
            auto ite_chain = _subchains.find(subchain);
            ultrainio_assert(ite_chain != _subchains.end(), "subchain not found");
            confirmed_blocknum = ite_chain->confirmed_block_number;
            vote_threshold = (uint32_t)ceil((double)ite_chain->committee_num * 2 / 3);
        }

        producers_table _producers(_self, subchain);
        auto propos = _producers.find(current_sender());
        ultrainio_assert( propos != _producers.end(), "enabled producer not found this proposer" );

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
            auto ret = acc.find(current_sender());
            ultrainio_assert(ret == acc.end(), "the committee_members already report such ws hash");

            auto it = hashv.begin();
            for(; it != hashv.end(); it++) {
                if(hash == it->hash && file_size == it->file_size) {
                    ultrainio_assert(it->valid != true, "the hash value has been voted as valid");
                    if((it->votes+1) >= vote_threshold) {
                        //now let's make sure accounts voted for this hash all are current committee members
                        uint32_t cnt = 0;
                        for(auto itac : it->accounts) {
                            auto vacc = _producers.find(itac);
                            if( vacc == _producers.end()) {
                                hashTable.modify(wshash, [&](auto &p) {
                                    auto pos = static_cast<unsigned int>(it - hashv.begin());
                                    p.hash_v[pos].votes--;
                                    p.hash_v[pos].accounts.erase(itac);
                                    p.accounts.erase(itac);
                                });
                                cnt++;
                            }
                        }
                        if(cnt == 0) {
                            hashTable.modify(wshash, [&](auto &p) {
                                auto pos = static_cast<unsigned int>(it - hashv.begin());
                                p.hash_v[pos].votes++;
                                p.hash_v[pos].valid = true;
                                p.hash_v[pos].accounts.emplace(current_sender());
                                p.accounts.emplace(current_sender());
                            });
                        } else {
                            hashTable.modify(wshash, [&](auto &p) {
                                auto pos = static_cast<unsigned int>(it - hashv.begin());
                                p.hash_v[pos].votes++;
                                p.hash_v[pos].accounts.emplace(current_sender());
                                p.accounts.emplace(current_sender());
                            });
                        }
                    } else {
                        hashTable.modify(wshash, [&](auto &p) {
                            auto pos = static_cast<unsigned int>(it - hashv.begin());
                            p.hash_v[pos].votes++;
                            p.hash_v[pos].accounts.emplace(current_sender());
                            p.accounts.emplace(current_sender());
                       });
                    }
                    break;
                }
            }
            if(it == hashv.end()) {
                hashTable.modify(wshash, [&](auto& p) {
                    p.hash_v.emplace_back(hash, file_size, 1, false, current_sender());
                    p.accounts.emplace(current_sender());
                });
            }
        }
        else {
            hashTable.emplace([&](auto &p) {
                p.block_num = blocknum;
                p.hash_v.emplace_back(hash, file_size, 1, false, current_sender());
                p.accounts.emplace(current_sender());
            });

            // delete item which is old enough but need to keep it if it is the only one with valid flag
            if (blocknum > uint64_t(MAX_WS_COUNT * _gstate.worldstate_interval)){
                uint64_t latest_valid = 0;
                uint64_t expired = blocknum - uint64_t(MAX_WS_COUNT * _gstate.worldstate_interval);
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
    void system_contract::regsubchain(name chain_name, uint64_t chain_type) {
        require_auth(N(ultrainio));
        ultrainio_assert(chain_name != default_chain_name, "subchian cannot named as default.");
        ultrainio_assert(chain_name != master_chain_name, "subchian cannot named as 0");
        auto itor = _subchains.find(chain_name);
        ultrainio_assert(itor == _subchains.end(), "there has been a subchian with this name");
        chaintypes_table type_tbl(_self, _self);
        auto type_iter = type_tbl.find(chain_type);
        ultrainio_assert(type_iter != type_tbl.end(), "this chain type is not existed");

        _subchains.emplace( [&]( auto& new_subchain ) {
            new_subchain.chain_name         = chain_name;
            new_subchain.chain_type         = chain_type;
            new_subchain.is_synced          = false;
            new_subchain.is_schedulable     = false;
            new_subchain.committee_num      = 0;
            new_subchain.changing_block_num = 0;
            new_subchain.total_user_num     = 0;
            new_subchain.chain_id           = checksum256();
            new_subchain.committee_mroot    = checksum256();
            new_subchain.confirmed_block_number = 0;
        });
    }

    /// @abi action
    void system_contract::acceptmaster(const std::vector<ultrainio::block_header>& headers) {
        require_auth(current_sender());
        ultrainio_assert(headers.size() <= 10, "too many blocks are reported.");
        master_chain_infos masterinfos(_self, _self);
        ultrainio_assert(masterinfos.exists(), "master chain info has not been set");
        master_chain master_info = masterinfos.get();
        uint32_t confirmed_number_before = master_info.confirmed_block_number;
        checksum256 final_confirmed_id;
        for(uint32_t idx = 0; idx < headers.size(); ++idx) {
            bool has_accepted = false;
            auto block_number = headers[idx].block_num();
            if(block_number <= master_info.confirmed_block_number) {
                //ignore blocks whose number has been confirmed
                continue;
            }
            auto block_id = headers[idx].id();

            //check if it has been accepted, if not, find its previous block
            uint32_t pre_block_index = std::numeric_limits<uint32_t>::max();
            if(!master_info.unconfirmed_blocks.empty()) {
                uint32_t i = master_info.unconfirmed_blocks.size() - 1;
                //more efficient to find from the end of the vector
                while(true) {
                    if(master_info.unconfirmed_blocks[i].block_id == block_id &&
                       master_info.unconfirmed_blocks[i].block_number == block_number) {
                        //duplicated block
                        has_accepted = true;
                        break;
                    }
                    if(master_info.unconfirmed_blocks[i].block_id == headers[idx].previous &&
                        block_number == master_info.unconfirmed_blocks[i].block_number + 1) {
                        //previous block is found, duplicated block couldn't be added at front of it's previous
                        //so it can break from the loop
                        pre_block_index = i;
                        final_confirmed_id = master_info.unconfirmed_blocks[i].block_id; // to be deleted
                        break;
                    }
                    if(i == 0) {
                        break;
                    }
                    i--;
                }
                if(has_accepted || pre_block_index >= master_info.unconfirmed_blocks.size()) {
                    continue;
                }
            }

            char confirm_id[32];
            //TODO, need check whether use master_chain_name or master_info.owner
            if(master_info.confirmed_block_number == master_info.block_height) {
                checksum256 preid;//TODO, replace it with master_info.block_id
                if(!accept_initial_header(master_chain_name, preid, master_info.master_prods,
                                          headers[idx], confirm_id, sizeof(confirm_id))) {
                    continue;
                }
            }
            else if(!accept_block_header(master_chain_name, headers[idx], confirm_id, sizeof(confirm_id))) {
                //verification failed in light client
                continue;
            }
            memcpy(final_confirmed_id.hash, confirm_id, sizeof(confirm_id));

            //handle confirmed block
            auto ite_confirm_block = master_info.unconfirmed_blocks.begin();
            for(; ite_confirm_block != master_info.unconfirmed_blocks.end(); ++ite_confirm_block) {
                if(ite_confirm_block->block_id == final_confirmed_id) {
                    break;
                }
            }
            ultrainio_assert(ite_confirm_block != master_info.unconfirmed_blocks.end(), "error, confirm block of master is not found");

            master_info.confirmed_block_number = ite_confirm_block->block_number;
            if(ite_confirm_block->committee_mroot != master_info.committee_mroot) {
                master_info.committee_mroot = ite_confirm_block->committee_mroot;
                master_info.master_prods.swap(ite_confirm_block->committee_set);
            }
            //save current block
            master_info.unconfirmed_blocks[pre_block_index].is_leaf = false;
            unconfirmed_master_header uncfm_header(headers[idx], block_id, block_number);
            master_info.unconfirmed_blocks.push_back(uncfm_header);
        }
        masterinfos.set(master_info);
        //delete ancient confirmed blocks to keep only recent 1000 blocks
        if (confirmed_number_before < master_info.confirmed_block_number ) {
            //confirm block updated
            block_table subchain_block_tbl(_self, master_chain_name);
            //remove old blocks, only keep the recent 1000 blocks
            if(master_info.confirmed_block_number > 1000) {
                uint32_t start_num = 0;
                if(confirmed_number_before > 1000) {
                    start_num = confirmed_number_before - 1000;
                }
                for(uint32_t j = start_num; j < master_info.confirmed_block_number - 1000; ++j) {
                    auto block_ite = subchain_block_tbl.find(uint64_t(j));
                    if(block_ite != subchain_block_tbl.end()) {
                        subchain_block_tbl.erase(block_ite);
                    }
                }
            }

            auto ite_uncfm_block = master_info.unconfirmed_blocks.begin();
            for(; ite_uncfm_block != master_info.unconfirmed_blocks.end(); ) {
                if(ite_uncfm_block->block_number > master_info.confirmed_block_number) {
                    ++ite_uncfm_block;
                }
                // cannot filter fork blocks who have a follow-up block, we can only filter fork block as a leaf
                else if((ite_uncfm_block->block_number < master_info.confirmed_block_number &&
                         !ite_uncfm_block->is_leaf) ||
                        (ite_uncfm_block->block_number == master_info.confirmed_block_number &&
                         ite_uncfm_block->block_id == final_confirmed_id) ) {

                    //add to block table
                    auto block_ite = subchain_block_tbl.find(uint64_t(ite_uncfm_block->block_number));
                    if(block_ite == subchain_block_tbl.end()) {
                        print("error: a block number is confirmed twice\n");
                    }
                    subchain_block_tbl.emplace([&]( auto& new_confirmed_header ) {
                        new_confirmed_header = block_header_digest(ite_uncfm_block->proposer, ite_uncfm_block->block_id,
                                            ite_uncfm_block->block_number, ite_uncfm_block->transaction_mroot);
                    });
                    ite_uncfm_block = master_info.unconfirmed_blocks.erase(ite_uncfm_block);
                }
                else {
                    //fork branch, remove it
                    ite_uncfm_block = master_info.unconfirmed_blocks.erase(ite_uncfm_block);
                }
            }
        }
        masterinfos.set(master_info);
    }

    bool system_contract::checkblockproposer(account_name block_proposer, subchains_table::const_iterator chain_iter) {
        //find proposer and check if it should be paid for proposing this block.
        if(block_proposer != N() && block_proposer != N(genesis)) {
            auto briefprod = _briefproducers.find(block_proposer);
            if(briefprod == _briefproducers.end()) {
                print("acceptheader: block proposer is not a valid producer\n");
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
                         return false; //TODO, unsynced state is always no reward, is it reasonable and acceptable?
                    }
                }
            }
            //3. still not found, find in changing info
            for(auto ite_rm = chain_iter->changing_info.removed_members.begin();
            ite_rm != chain_iter->changing_info.removed_members.end(); ++ite_rm) {
                if(ite_rm->owner == block_proposer) {
                    return true; //TODO, check expire time by operating time
                }
            }
        }
        return false;
    }
    /// @abi action
    void system_contract::acceptheader (name chain_name,
                                  const std::vector<ultrainio::block_header>& headers) {
        require_auth(current_sender());
        ultrainio_assert(headers.size() <= 10, "too many blocks are reported.");
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "subchian is not existed.");
        bool synced = headers.size() == 10 ? false : true;
        uint32_t confirmed_number_before = ite_chain->confirmed_block_number;
        checksum256 final_confirmed_id;
        bool new_confirm = false;
        for(uint32_t idx = 0; idx < headers.size(); ++idx) {
            bool has_accepted = false;
            auto block_number = headers[idx].block_num();
            if(block_number <= ite_chain->confirmed_block_number) {
                //ignore blocks whose number has been confirmed
                continue;
            }
            auto block_id = headers[idx].id();

            //check if it has been accepted, if not, find its previous block
            uint32_t pre_block_index = std::numeric_limits<uint32_t>::max();
            if(!ite_chain->unconfirmed_blocks.empty()) {
                uint32_t i = ite_chain->unconfirmed_blocks.size() - 1;
                //more efficient to find from the end of the vector
                while(true) {
                    if(ite_chain->unconfirmed_blocks[i].block_id == block_id &&
                       ite_chain->unconfirmed_blocks[i].block_number == block_number) {
                        //duplicated block
                        has_accepted = true;
                        break;
                    }
                    if(ite_chain->unconfirmed_blocks[i].block_id == headers[idx].previous &&
                        block_number == ite_chain->unconfirmed_blocks[i].block_number + 1) {
                        //previous block is found, duplicated block couldn't be added at front of it's previous
                        pre_block_index = i;
                        break;
                    }
                    if(i == 0) {
                        break;
                    }
                    i--;
                }
                if(has_accepted) {
                    continue;
                }
            }
            if(block_number > 1 && pre_block_index >= ite_chain->unconfirmed_blocks.size() ) {
                    print("previous block is not found\n");
                    continue;
            }

            const account_name block_proposer = headers[idx].proposer;
            if(block_proposer == N(genesis) || block_number == 1 || !headers[idx].header_extensions.empty()) {
                char confirm_id[32];
                if(!accept_block_header(chain_name, headers[idx], confirm_id, sizeof(confirm_id))) {
                    //verification failed in light client
                    continue;
                }
                memcpy(final_confirmed_id.hash, confirm_id, sizeof(confirm_id));
                new_confirm = true;
            }
            bool need_report = checkblockproposer(block_proposer, ite_chain);

            _subchains.modify(ite_chain, [&]( auto& _subchain ) {
                unconfirmed_block_header uncfm_header(headers[idx], block_id, block_number, need_report, synced);
                if(block_number > 1) {
                    _subchain.unconfirmed_blocks[pre_block_index].is_leaf = false;
                }
                _subchain.unconfirmed_blocks.push_back(uncfm_header);

                if(!new_confirm) {
                    return;
                }
                auto ite_confirm_block = _subchain.unconfirmed_blocks.end();
                // currenr block is confirmed immediately，it must be block 1 or producerd by genesis
                if(final_confirmed_id == block_id) {
                    if(1 == block_number) {
                        //same block case has been filtered above
                        ultrainio_assert(_subchain.unconfirmed_blocks.size() == 1, "a sidechain can only accept one genesis block");
                        _subchain.chain_id          = headers[idx].action_mroot; //save chain id
                        _subchain.genesis_time      = headers[idx].timestamp;
                        _subchain.is_schedulable    = true;
                    }

                    ite_confirm_block = _subchain.unconfirmed_blocks.end();
                    --ite_confirm_block;
                }
                else {
                    ite_confirm_block = _subchain.unconfirmed_blocks.begin();
                    for(; ite_confirm_block != _subchain.unconfirmed_blocks.end(); ++ite_confirm_block) {
                        if(ite_confirm_block->block_id == final_confirmed_id) {
                            break;
                        }
                    }
                }
                ultrainio_assert(ite_confirm_block != _subchain.unconfirmed_blocks.end(), "error, confirm block is not found");

                if (confirmed_number_before == ite_confirm_block->block_number) {
                    return;
                }
                //handle new confirmed block
                _subchain.confirmed_block_number = ite_confirm_block->block_number;
                if(!_subchain.is_synced && ite_confirm_block->is_synced && !_subchain.deprecated_committee.empty()) {
                    _subchain.deprecated_committee.clear();
                }
                _subchain.is_synced = ite_confirm_block->is_synced;
            });
        }
        if (new_confirm && confirmed_number_before < ite_chain->confirmed_block_number ) {
            //confirm block updated
            block_table subchain_block_tbl(_self, chain_name);
            //remove old blocks, only keep the recent 1000 blocks
            if(ite_chain->confirmed_block_number > 1000) {
                uint32_t start_num = 0;
                if(confirmed_number_before > 1000) {
                    start_num = confirmed_number_before - 1000;
                }
                for(uint32_t j = start_num; j < ite_chain->confirmed_block_number - 1000; ++j) {
                    auto block_ite = subchain_block_tbl.find(uint64_t(j));
                    if(block_ite != subchain_block_tbl.end()) {
                        subchain_block_tbl.erase(block_ite);
                    }
                }
            }
            _subchains.modify(ite_chain, [&]( auto& _subchain ) {
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
                        else if(ite_uncfm_block->to_be_paid) {
                            //apply reward for its proposer
                            reportblocknumber(chain_name, ite_chain->chain_type, ite_uncfm_block->proposer, 1);
                        }
                        subchain_block_tbl.emplace([&]( auto& new_confirmed_header ) {
                            new_confirmed_header = block_header_digest(ite_uncfm_block->proposer, ite_uncfm_block->block_id,
                                                   ite_uncfm_block->block_number, ite_uncfm_block->transaction_mroot);
                        });
                        //handle committee update
                        if(ite_uncfm_block->committee_mroot != _subchain.committee_mroot) {
                            if(ite_uncfm_block->committee_set.empty()) {
                                print("error: committee mroot changed but committee set is empty");
                            }
                            _subchain.committee_mroot = ite_uncfm_block->committee_mroot;
                            //get committee delta
                            committee_delta compare_delta(_subchain.committee_set, ite_uncfm_block->committee_set);
                            for(auto it_rm = _subchain.changing_info.removed_members.begin();
                                     it_rm != _subchain.changing_info.removed_members.end();) {
                                if(compare_delta.removed.find(it_rm->owner) != compare_delta.removed.end()) {
                                    it_rm = _subchain.changing_info.removed_members.erase(it_rm);
                                }
                                else {
                                    ++it_rm;
                                }
                            }
                            for(auto it_add = _subchain.changing_info.new_added_members.begin();
                                     it_add != _subchain.changing_info.new_added_members.end();) {
                                if(compare_delta.added.find(it_add->owner) != compare_delta.added.end()) {
                                    it_add = _subchain.changing_info.new_added_members.erase(it_add);
                                }
                                else {
                                    ++it_add;
                               }
                            }
                            if(_subchain.changing_info.empty()) {
                                _subchain.is_schedulable = true;
                            }
                            _subchain.committee_set.swap(ite_uncfm_block->committee_set);//confirmed block will be erased
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
    }

    void system_contract::clearchain(name chain_name, bool users_only) {
        require_auth(N(ultrainio));
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "This subchian is not existed.");
        //TODO, check if the subchain is avtive?
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
            _subchain.unconfirmed_blocks.clear();
            _subchain.unconfirmed_blocks.shrink_to_fit();
            _subchain.changing_info.clear();
            _subchain.deprecated_committee.clear();
        });
        print( "clearchain chain_name:", name{chain_name}, " users_only:", users_only, "\n" );
        producers_table _producers(_self, chain_name);
        auto ite = _producers.begin();
        for(; ite != _producers.end(); ++ite) {
            _producers.modify(ite, [&](auto & p) {
                p.unpaid_balance = 0;
                p.total_produce_block = 0;
            });
        }
        block_table subchain_block_tbl(_self, chain_name);
        auto it_blk = subchain_block_tbl.begin();
        while (it_blk != subchain_block_tbl.end()) {
            it_blk = subchain_block_tbl.erase(it_blk);
        }
    }

    void system_contract::empoweruser(account_name user, name chain_name, const std::string& owner_pk, const std::string& active_pk) {
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

    void system_contract::add_to_chain(name chain_name, const producer_info& producer, uint64_t current_block_number) {
        if (chain_name != master_chain_name) {
            auto ite_chain = _subchains.find(chain_name);
            ultrainio_assert(ite_chain != _subchains.end(), "destination sidechain is not existed" );
            chaintypes_table type_tbl(_self, _self);
            auto typeiter = type_tbl.find(ite_chain->chain_type);
            ultrainio_assert(typeiter != type_tbl.end(), "destination sidechain type is not existed");
            ultrainio_assert(ite_chain->committee_num < typeiter->stable_max_producers,
                "destination sidechain already has enough producers");
            //check if this account has been empowered to this chain
            if(!is_empowered(producer.owner, chain_name)) {
                user_info tempuser;
                tempuser.user_name = producer.owner;
                tempuser.is_producer = true;
                tempuser.emp_time = now();

                _subchains.modify(ite_chain, [&]( auto& _chain ) {
                    _chain.recent_users.push_back(tempuser);
                    _chain.total_user_num += 1;
                });
                empower_to_chain(producer.owner, chain_name);
            }
            _subchains.modify(ite_chain, [&](subchain& info) {
                role_base temp_prod;
                temp_prod.owner = producer.owner;
                temp_prod.producer_key = producer.producer_key;
                temp_prod.bls_key = producer.bls_key;
                info.changing_info.new_added_members.push_back(temp_prod);
                info.is_schedulable = false;
                info.committee_num += 1;
            });
        }else{
           _gstate.cur_committee_number++;
        }

        producers_table _producer_chain(_self, chain_name);
        auto prod = _producer_chain.find( producer.owner );
        ultrainio_assert(prod == _producer_chain.end(), "producer has existed in destination chain");
        _producer_chain.emplace([&]( producer_info& _prod ) {
            _prod = producer;
            _prod.last_operate_blocknum = current_block_number;
        });
    }

    void system_contract::remove_from_chain(name chain_name, account_name producer_name) {
        producers_table _producer_chain(_self, chain_name);
        auto prod = _producer_chain.find( producer_name );
        ultrainio_assert(prod != _producer_chain.end(), "producer is not existed in source chain");

        if (chain_name != master_chain_name) {
            auto ite_chain = _subchains.find(chain_name);
            ultrainio_assert(ite_chain != _subchains.end(), "source sidechain is not existed");
            chaintypes_table type_tbl(_self, _self);
            auto typeiter = type_tbl.find(ite_chain->chain_type);
            ultrainio_assert(typeiter != type_tbl.end(), "source subchain type is not existed");
            ultrainio_assert(ite_chain->committee_num > typeiter->stable_min_producers,
                "the producers in source sidechain is not enough for removing");

            _subchains.modify(ite_chain, [&](subchain& info) {
                role_base temp_prod;
                temp_prod.owner = prod->owner;
                temp_prod.producer_key = prod->producer_key;
                temp_prod.bls_key = prod->bls_key;
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
        _producer_chain.erase(prod);
    }

    struct chain_sched_info {
        uint16_t                          sched_out_num = 0;
        uint16_t                          gap_to_next_level = 0;
        uint8_t                           sched_random_factor = 0; //used for random order
        uint16_t                          inc_step = 1;
        subchains_table::const_iterator   chain_ite;

        chain_sched_info(uint16_t out, uint16_t gap_to_nl, uint8_t random_factor, uint16_t is, subchains_table::const_iterator ite)
         :sched_out_num(out),gap_to_next_level(gap_to_nl),sched_random_factor(random_factor),inc_step(is),chain_ite(ite) {}
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
            if((chain_iter->committee_num < uint32_t(type_iter->stable_min_producers)) ||
               !(chain_iter->is_schedulable) || !(chain_iter->is_synced)) {
                continue;
            }
            auto out_num = (chain_iter->committee_num - type_iter->stable_min_producers)/type_iter->sched_inc_step;
            auto gap = type_iter->sched_inc_step - (chain_iter->committee_num - type_iter->stable_min_producers)%type_iter->sched_inc_step;
            uint8_t rf = head_block_hash.hash[31 - index_in_list % 30] + uint8_t(index_in_list / 30);//first 2 bytes are always 0, so here we use 30 =32 -2
            out_list.emplace_back(out_num, gap, rf, type_iter->sched_inc_step, chain_iter);
        }

        print( "[schedule] out list size: ", out_list.size(), "\n" );
        if(out_list.size() < 2) {
            return;
        }

        //sort by committee sum
        out_list.sort(compare_gt);

        ////start 1st scheduling, this step is for balance
        auto min_sched_chain  = out_list.end();
        --min_sched_chain;

        print("[schedule] step 1:\n");
        auto out_iter = out_list.begin();
        for(; out_iter != min_sched_chain && out_iter != out_list.end() && compare_gt(*out_iter, *min_sched_chain); ++out_iter) {
            print("[schedule] out_chain: ", name{out_iter->chain_ite->chain_name});
            print(" can move out at most: ", uint32_t(out_iter->sched_out_num), " producers\n");
            bool quit_loop = false;
            for(uint16_t out_idx = 0; out_idx < out_iter->sched_out_num; ++out_idx ) {
                if(!move_producer(head_block_hash, out_iter->chain_ite, min_sched_chain->chain_ite, uint64_t(block_height)) ) {
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
            if(!move_producer(head_block_hash, chain_from->chain_ite, chain_to->chain_ite, uint64_t(block_height)) ) {
                continue;
            }
        }
        chain_from = out_list.end();
        --chain_from;
        chain_to = out_list.begin();
        print("[schedule] from chain: ", name{chain_from->chain_ite->chain_name});
        print(", to chain: ", name{chain_to->chain_ite->chain_name}, "\n");
        move_producer(head_block_hash, chain_from->chain_ite, chain_to->chain_ite, uint64_t(block_height));
    }

    bool system_contract::move_producer(checksum256 head_id, subchains_table::const_iterator from_iter,
                                        subchains_table::const_iterator to_iter, uint64_t current_block_number) {
        uint32_t x = uint32_t(head_id.hash[31]);
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
        auto producer_iter = _producers.find(producer);
        if(producer_iter == _producers.end()) {
            print("[schedule] error: producer to move out is not found in _producers\n");
            return false;
        }
        print("[schedule] move ", name{producer}, " to ", name{to_iter->chain_name}, "\n");
        //move producer
        auto briefprod = _briefproducers.find(producer);
        if(briefprod != _briefproducers.end()) {
            add_to_chain(to_iter->chain_name, *producer_iter, current_block_number);
            remove_from_chain(from_iter->chain_name, producer);
            _briefproducers.modify(briefprod, [&](producer_brief& producer_brf) {
                producer_brf.location = to_iter->chain_name;
            });
        }

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

//TODO, need whole block header info
    void system_contract::forcesetblock(name chain_name, block_header_digest header_dig, checksum256 committee_mrt) {
        //set confirm block of subchain
        require_auth(N(ultrainio));
        auto ite_chain = _subchains.find(chain_name);
        ultrainio_assert(ite_chain != _subchains.end(), "subchain is not found");
        uint32_t current_confirmed_number = ite_chain->confirmed_block_number;
        _subchains.modify(ite_chain, [&]( auto& _subchain ) {
            _subchain.confirmed_block_number = header_dig.block_number;
            _subchain.unconfirmed_blocks.clear();
            _subchain.is_synced = false;
            _subchain.is_schedulable = true;

            unconfirmed_block_header temp_header;
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

    name system_contract::getdefaultchain() {
        auto ite_chain = _subchains.begin();
        auto ite_min = _subchains.end();
        uint32_t max_gap = 0;
        chaintypes_table type_tbl(_self, _self);
        for(; ite_chain != _subchains.end(); ++ite_chain) {
            if(!ite_chain->is_schedulable) {
                continue;
            }
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
        if(ite_min == _subchains.end()) {
            //there's no schedulable or addable sidechain existed currently
            return master_chain_name;
        }
        else {
            return ite_min->chain_name;
        }
    }
} //namespace ultrainiosystem
