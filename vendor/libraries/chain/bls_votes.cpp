#include <ultrainio/chain/bls_votes.hpp>

#include <ultrainio/chain/database_utils.hpp>

namespace ultrainio { namespace chain { namespace bls_votes {
    using bls_index_set = index_set<bls_votes_index>;

    void bls_votes_manager::add_indices(chainbase::database& db) {
        bls_index_set::add_indices(db);
    }

    void bls_votes_manager::initialize_database() {
        _db.create<bls_votes_object>([&](bls_votes_object& obj){
            // set default
        });
    }

    bool bls_votes_manager::has_should_be_confirmed_bls(std::string& bls) const {
        const auto& o = _db.get<bls_votes_object>();
        bool result = false;
        if (o.should_be_confirmed.size() > 0) {
            result = true;
            bls = o.should_be_confirmed.front().bls_str;
        }
        return result;
    }

    bool bls_votes_manager::should_be_confirmed(uint32_t block_num) const {
        const auto& o = _db.get<bls_votes_object>();
        ilog("latest_confirmed_block_num : ${latest}", ("latest", o.latest_confirmed_block_num));
        for (auto itor = o.should_be_confirmed.begin(); itor != o.should_be_confirmed.end(); itor++) {
            ilog("unconfirmed block num : ${num}, end_epoch : ${end_epoch}, bls_valid : ${bls_valid}, bls : ${bls}",
                    ("num", itor->block_num)("end_epoch", itor->end_epoch)("bls_valid", itor->valid_bls)("bls", itor->bls_str));
        }
        if (o.should_be_confirmed.size() > 0) {
            if (o.should_be_confirmed.back().block_num + confirm_point_interval == block_num) {
                return true;
            }
        } else if (o.latest_confirmed_block_num + confirm_point_interval == block_num) {
            return true;
        }
        return false;
    }

    void bls_votes_manager::add_confirmed_bls_votes(uint32_t block_num, bool end_epoch, bool valid_bls, const std::string& bls_str) {
        const auto& o = _db.get<bls_votes_object>();
        _db.modify(o, [&](bls_votes_object& obj) {
            obj.should_be_confirmed.emplace_back(block_num, end_epoch, valid_bls, bls_str);
        });
    }

    bool bls_votes_manager::check_can_confirm(uint32_t block_num) const {
        // no end epoch before
        const auto& o = _db.get<bls_votes_object>();
        if (o.latest_confirmed_block_num >= block_num) {
            elog("confirm block num ${block_num} great than confirmed ${confirmed}", ("block_num", block_num)("confirmed", o.latest_confirmed_block_num));
            return false;
        }
        if (o.should_be_confirmed.size() > 0 && o.should_be_confirmed.front().block_num == block_num) {
            return true;
        }
        return false;
    }

    void bls_votes_manager::confirm(uint32_t block_num) {
        clean_no_end_epoch(block_num, block_num);
    }

    void bls_votes_manager::clean_no_end_epoch(uint32_t block_num, uint32_t confirmed_block_num) {
        const auto& o = _db.get<bls_votes_object>();

        _db.modify(o, [&](bls_votes_object& obj) {
            ilog("latest_confirmed_block_num : ${latest}", ("latest", o.latest_confirmed_block_num));
            for (auto itor = o.should_be_confirmed.begin(); itor != o.should_be_confirmed.end(); itor++) {
                ilog("unconfirmed block num : ${num}, end_epoch : ${end_epoch}, bls_valid : ${bls_valid}, bls : ${bls}",
                     ("num", itor->block_num)("end_epoch", itor->end_epoch)("bls_valid", itor->valid_bls)("bls", itor->bls_str));
            }
            auto begin = obj.should_be_confirmed.begin();
            auto itor = begin;
            for (; itor != obj.should_be_confirmed.end(); itor++) {
                if (itor->block_num > block_num || itor->end_epoch) {
                    if (itor->block_num <= block_num) {
                        elog("try to clear end epoch ${num}", ("num", itor->block_num));
                    }
                    break;
                }
            }
            if (itor != begin) {
                obj.should_be_confirmed.erase(begin, itor);
            }
            if (confirmed_block_num != 0) {
                obj.latest_confirmed_block_num = confirmed_block_num;
            }
            ilog("latest_confirmed_block_num : ${latest}", ("latest", o.latest_confirmed_block_num));
            for (auto itor = o.should_be_confirmed.begin(); itor != o.should_be_confirmed.end(); itor++) {
                ilog("unconfirmed block num : ${num}, end_epoch : ${end_epoch}, bls_valid : ${bls_valid}, bls : ${bls}",
                     ("num", itor->block_num)("end_epoch", itor->end_epoch)("bls_valid", itor->valid_bls)("bls", itor->bls_str));
            }
        });
    }

} } } // ultrainio::chain::bls_votes
