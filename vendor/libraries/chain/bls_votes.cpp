#include <ultrainio/chain/bls_votes.hpp>

#include <ultrainio/chain/database_utils.hpp>

namespace ultrainio {
    namespace chain {
        namespace detail {
            using bls_votes::bls_votes_info;
            using bls_votes::bls_votes_object;
            using bls_votes::shared_bls_votes_info;
            using bls_votes::worldstate_bls_voters_object;

            template<>
            struct worldstate_row_traits<bls_votes_object> {
                using value_type = bls_votes_object;
                using worldstate_type = worldstate_bls_voters_object;

                static worldstate_bls_voters_object
                to_worldstate_row(const bls_votes_object &value, const chainbase::database &db, void *data) {
                    ilog("to_worldstate_row bls_votes");
                    worldstate_bls_voters_object res;
                    res.latest_confirmed_block_num = value.latest_confirmed_block_num;
                    res.latest_check_point_id = value.latest_check_point_id;
                    for (auto e : value.should_be_confirmed) {
                        bls_votes_info info = e.to_info();
                        info.valid_bls = false;
                        info.bls_str = std::string();
                        res.should_be_confirmed.push_back(info);
                    }
                    return res;
                }

                static void from_worldstate_row(worldstate_bls_voters_object &&row, bls_votes_object &value,
                                                chainbase::database &db, bool backup, void *data) {
                    ilog("from_worldstate_row bls_votes");
                    value.latest_confirmed_block_num = row.latest_confirmed_block_num;
                    value.latest_check_point_id = row.latest_check_point_id;
                    for (auto e : row.should_be_confirmed) {
                        shared_bls_votes_info info(e.block_num, e.end_epoch, e.valid_bls, e.bls_str, value.should_be_confirmed.get_allocator());
                        value.should_be_confirmed.push_back(info);
                    }
                }
            };
        }

        namespace bls_votes {
            using bls_index_set = index_set<bls_votes_index,
                                            bls_votes_current_index>;

            int bls_votes_manager::s_confirm_point_interval = 0;

            void bls_votes_manager::add_indices(chainbase::database &db) {
                bls_index_set::add_indices(db);
                ilog("add_indices");
            }

            void bls_votes_manager::initialize_database() {
                ilog("initialize_database");
                _db.create<bls_votes_object>([&](bls_votes_object &obj) {
                });
                _db.create<bls_votes_current_object>([&](bls_votes_current_object &obj) {
                });
            }

            void bls_votes_manager::add_to_worldstate(std::shared_ptr<ws_helper> ws_helper_ptr,
                                                      chainbase::database &worldstate_db) {
                bls_index_set::walk_indices([this, &worldstate_db, &ws_helper_ptr](auto utils) {
                    using index_t = typename decltype(utils)::index_t;
                    using value_t = typename index_t::value_type;
                    if (std::is_same<value_t, bls_votes_object>::value) {
                        ws_helper_ptr->add_table_to_worldstate<index_t>(worldstate_db);
                    }
                });
            }

            void bls_votes_manager::set_confirm_point_interval(int interval) {
                s_confirm_point_interval = interval;
            }

            void bls_votes_manager::read_from_worldstate(std::shared_ptr<ws_helper> ws_helper_ptr,
                                                         chainbase::database &worldstate_db) {
                bls_index_set::walk_indices([&](auto utils) {
                    using index_t = typename decltype(utils)::index_t;
                    using value_t = typename index_t::value_type;
                    if (std::is_same<value_t, bls_votes_object>::value) {
                        ws_helper_ptr->read_table_from_worldstate<index_t>(worldstate_db);

                        // create bls_votes_current_object
                        _db.create<bls_votes_current_object>([&](bls_votes_current_object &obj) {
                        });
                    }
                });
            }

            block_id_type bls_votes_manager::get_latest_check_point_id() const {
                const auto &o = _db.get<bls_votes_object>();
                return o.latest_check_point_id;
            }

            void bls_votes_manager::set_latest_check_point_id(block_id_type id) {
                const auto &o = _db.get<bls_votes_object>();
                _db.modify(o, [&](bls_votes_object &obj) {
                    obj.latest_check_point_id = id;
                });
            }

            bool bls_votes_manager::has_should_be_confirmed_bls(std::string &bls) const {
                const auto &o = _db.get<bls_votes_object>();
                bool result = false;
                if (o.should_be_confirmed.size() > 0) {
                    shared_bls_votes_info info(o.should_be_confirmed.front());
                    result = true;
                    bls = std::string(info.bls_str.c_str(), info.bls_str.size());
                }
                return result;
            }

            bool bls_votes_manager::should_be_confirmed(uint32_t block_num) const {
                const auto &o = _db.get<bls_votes_object>();
                ULTRAIN_ASSERT(s_confirm_point_interval > 0, chain_exception, "not set s_confirm_point_interval");
                if (o.should_be_confirmed.size() > 0 && o.should_be_confirmed.back().block_num + s_confirm_point_interval == block_num) {
                    wlog("there are too many unconfirmed block;");
                    for (auto itor = o.should_be_confirmed.begin(); itor != o.should_be_confirmed.end(); itor++) {
                        ilog("unconfirmed block num : ${num}, end_epoch : ${end_epoch}, bls_valid : ${bls_valid}, bls : ${bls}",
                             ("num", itor->block_num)("end_epoch", itor->end_epoch)("bls_valid", itor->valid_bls)("bls", std::string(itor->bls_str.begin(), itor->bls_str.end())));
                    }
                    return true;
                } else if ((block_num - o.latest_confirmed_block_num) % s_confirm_point_interval == 0) {
                    return true;
                }
                return false;
            }

            void bls_votes_manager::add_confirmed_bls_votes(uint32_t block_num, bool end_epoch, bool valid_bls,
                                                            const std::string &bls_str) {
//                ilog("add_confirmed_bls_votes block num : ${num}, end_epoch : ${end_epoch}, bls_valid : ${bls_valid}, bls : ${bls}",
//                     ("num", block_num)("end_epoch", end_epoch)("bls_valid", valid_bls)("bls", bls_str));
                const auto &o = _db.get<bls_votes_object>();
                _db.modify(o, [&](bls_votes_object &obj) {
                    shared_bls_votes_info info(block_num, end_epoch, valid_bls, bls_str, obj.should_be_confirmed.get_allocator());
                    if (obj.should_be_confirmed.size() > 0) {
                        for (auto itor = obj.should_be_confirmed.begin(); itor != obj.should_be_confirmed.end();) {
                            if (!itor->end_epoch) {
                                ilog("erase should be confirmed block, num = ${num}", ("num", itor->block_num));
                                itor = obj.should_be_confirmed.erase(itor);
                            } else {
                                itor++;
                            }
                        }
                    }
                    obj.should_be_confirmed.push_back(info);
                });
            }

            bool bls_votes_manager::check_can_confirm(uint32_t block_num) const {
                const auto &o = _db.get<bls_votes_object>();
                if (o.latest_confirmed_block_num >= block_num) {
                    elog("confirm block num ${block_num} great than confirmed ${confirmed}",
                         ("block_num", block_num)("confirmed", o.latest_confirmed_block_num));
                    return false;
                }
                if (o.should_be_confirmed.size() > 0 && o.should_be_confirmed.front().block_num == block_num) {
                    return true;
                }
                elog("latest_confirmed_block_num : ${latest}", ("latest", o.latest_confirmed_block_num));
                return false;
            }

            void bls_votes_manager::confirm(uint32_t block_num) {
                ilog("confirm block num : ${num}", ("num", block_num));
                const auto &o = _db.get<bls_votes_object>();
                _db.modify(o, [&](bls_votes_object &obj) {
                    auto itor = obj.should_be_confirmed.begin();
                    for (; itor != obj.should_be_confirmed.end(); itor++) {
                        if (itor->block_num > block_num) {
                            break;
                        }
                    }
                    if (itor != obj.should_be_confirmed.begin()) {
                        obj.should_be_confirmed.erase(obj.should_be_confirmed.begin(), itor);
                    }
                    if (block_num != 0) {
                        obj.latest_confirmed_block_num = block_num;
                    }
                });
            }

            void bls_votes_manager::save_current_bls_votes(const std::string& bls_str) {
                const auto &o = _db.get<bls_votes_current_object>();
                _db.modify(o, [&](bls_votes_current_object &obj) {
                    obj.current_bls_str = shared_string(bls_str.begin(), bls_str.end(), obj.current_bls_str.get_allocator());
                });
            }

            std::string bls_votes_manager::get_current_bls_votes() const {
                try {
                    const auto& o = _db.get<bls_votes_current_object>();
                    return std::string(o.current_bls_str.begin(), o.current_bls_str.end());
                } catch (std::exception& e) {
                    ilog("get_current_bls_votes exception ${e}", ("e", std::string(e.what())));
                    _db.create<bls_votes_current_object>([&](bls_votes_current_object &obj) {
                    });
                }
                return std::string();
            }
        }
    }
} // ultrainio::chain::bls_votes
