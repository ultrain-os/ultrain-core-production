#include "ultrainio.res.hpp"
#include "expire_check.cpp"
#include <ultrainio.token/ultrainio.token.hpp>
namespace ultrainiores {
    using namespace ultrainiosystem;
    resource::resource( account_name s )
    :contract(s),
    _global(_self,_self) {
        if(_global.exists()) {
            _gstate = _global.get();
        }
    }
    resource::~resource() {
        _global.set( _gstate );
    }

    void resource::setresparams( const resource_global_params& params ) {
        require_auth( _self );
        _gstate.is_allow_buy_res = params.is_allow_buy_res;
        _gstate.max_ram_size = params.max_ram_size;
        _gstate.max_resources_number = params.max_resources_number;
        _gstate.free_account_per_res = params.free_account_per_res;
        _gstate.res_transfer_res = params.res_transfer_res;
        _gstate.resource_fee = params.resource_fee;
        _gstate.table_extension.assign(params.table_extension.begin(),params.table_extension.end());
    }

    void resource::resourcelease( account_name from, account_name receiver,
                                uint16_t combosize, uint64_t period, name location ) {
        ultrainiosystem::global_state_singleton   gstatesingle(N(ultrainio),N(ultrainio));
        ultrainio_assert( gstatesingle.exists(), "global table not found");
        ultrainiosystem::ultrainio_global_state globalst = gstatesingle.get();
        if( !globalst.is_master_chain() || !_gstate.is_allow_buy_res )
            ultrainio_assert( from == ultrainio_account_name, "only allow ultrainio account resourcelease" );
        ultrainio_assert(location != default_chain_name && location != N(master) , "wrong location");
        uint64_t bytes = 0;
        //get location confirm block
        uint32_t cur_block_height = 0;
        ultrainiosystem::chains_table   _chains(N(ultrainio),N(ultrainio));
        auto chain_itr = _chains.end();
        if( location != self_chain_name ) {
            chain_itr = _chains.find(location);
            ultrainio_assert(chain_itr != _chains.end(), "this subchian location is not existed");
            ultrainio_assert(is_empowered(receiver, location), "the receiver is not yet empowered to this chain before");
            cur_block_height = chain_itr->confirmed_block_number;
        } else {
            bytes = _gstate.max_ram_size/_gstate.max_resources_number;
            _gstate.total_resources_used_number += combosize;
            cur_block_height = (uint32_t)head_block_number();
        }
        std::string assert_des("combosize must less then ");
        assert_des.append(std::to_string(_gstate.max_resources_number));
        ultrainio_assert(combosize <= _gstate.max_resources_number, assert_des.c_str());

        if(!has_auth(ultrainio_account_name)) {
            require_auth( from );
            ultrainio_assert(combosize == _gstate.max_resources_number, ("must buy full resource: " + std::to_string(_gstate.max_resources_number) ).c_str());
        }

        uint64_t should_buy_period = 1;     //Start with the first period
        resources_periods_table _resperiods_tbl( _self,location );
        auto res_itr = _resperiods_tbl.rbegin() ;
        if( res_itr != _resperiods_tbl.rend() ) {
            should_buy_period = res_itr->periods + 1;
        }

        uint64_t cur_period = cur_block_height * block_interval_seconds()/ seconds_per_period + 1;
        if ( period == 0 ) {
            period = cur_period;
        }
        if ( should_buy_period < cur_period ) {
            should_buy_period = cur_period;
        }

        auto ite_period = _resperiods_tbl.find(period);
        if(ite_period != _resperiods_tbl.end()) {
            ultrainio_assert( period >= cur_period, ("expired period, current period is " + std::to_string(cur_period) ).c_str());
            ultrainio_assert(ite_period->total_lease_num + combosize <= _gstate.max_resources_number, "total combosize of this period exceeds full size");
            ultrainio_assert(ite_period->owner == receiver, "only period owner can continue to receive resource of this period");

            _resperiods_tbl.modify(ite_period, [&]( auto& tot ) {
                tot.total_lease_num += combosize;
                tot.modify_block_height = cur_block_height;
            });
        } else {
            ultrainio_assert( should_buy_period == period, ( std::to_string(should_buy_period)+ " should buy period not equal to period:" + std::to_string(period)).c_str() );
            _resperiods_tbl.emplace( [&]( auto& tot ) {
                tot.periods = period;
                tot.owner = receiver;
                tot.total_lease_num = combosize;
                tot.modify_block_height = cur_block_height;
            });
        }
        if ( globalst.is_master_chain() ) {
            auto resourcefee = (int64_t)(_gstate.resource_fee * combosize);
            ultrainio_assert(resourcefee > 0, "resource lease resourcefee is abnormal" );
            INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {from,N(active)},
                            { from, N(utrio.resfee), asset(resourcefee), std::string("buy resource lease") } );
            print("resourcelease calculatefee receiver:", name{receiver}," combosize:", uint32_t(combosize), " resourcefee:",resourcefee);
            resfreeaccount _resacc_tbl( _self, _self );
            auto resacc_to_itr = _resacc_tbl.find(receiver);
            if(resacc_to_itr == _resacc_tbl.end()) {
                resacc_to_itr = _resacc_tbl.emplace([&]( auto& res ) {
                    res.owner = receiver;
                    res.free_account_number = (uint32_t)(combosize* _gstate.free_account_per_res);
                });
            } else {
                _resacc_tbl.modify( resacc_to_itr, [&]( auto& res ) {
                    res.free_account_number += (uint32_t)(combosize* _gstate.free_account_per_res);
                });
            }
        }
        if( location == self_chain_name ) { //Purchase resources for this chain
            uint32_t last_period_blockheight = seconds_per_period / block_interval_seconds() * ((uint32_t)period - 1) + 1;
            uint32_t next_period_blockheight = seconds_per_period / block_interval_seconds() * (uint32_t)period;
            if ( period == cur_period ) {
                resources_lease_table _reslease_tbl( _self, 0 );
                auto reslease_itr = _reslease_tbl.find( receiver );
                if( reslease_itr ==  _reslease_tbl.end() ) {
                    penddeltable pendingdel(_self,_self);
                    auto deltab_itr = pendingdel.find(receiver);
                    ultrainio_assert(deltab_itr == pendingdel.end(), "legacy resource is pending for clean up, please apply new resources later");
                    _reslease_tbl.emplace( [&]( auto& tot ) {
                        tot.owner = receiver;
                        tot.lease_num = combosize;
                        tot.locked_num = 0;
                        tot.start_block_height = last_period_blockheight;
                        tot.end_block_height = next_period_blockheight;
                        tot.modify_block_height = cur_block_height;
                    });
                } else {
                    ultrainio_assert( false , " resource already exist" );
                }
                set_resource_limits( receiver, int64_t(bytes*combosize), int64_t(combosize), int64_t(combosize) );
                print("current resource limit  receiver:", name{receiver}, " net:", uint32_t(combosize)," cpu:", uint32_t(combosize)," ram:",int64_t(bytes*combosize));
            } else {
                resources_lease_table _reslease_tbl( _self, period );
                _reslease_tbl.emplace([&]( auto& res ) {
                    res.owner               = receiver;
                    res.lease_num           = combosize;
                    res.locked_num          = 0;
                    res.start_block_height  = last_period_blockheight;
                    res.end_block_height    = next_period_blockheight;
                    res.modify_block_height = cur_block_height;
                });
            }
        }
    }

    void resource::transresource(account_name from, account_name to, uint16_t combosize, uint64_t period){
        require_auth( from );
        resources_lease_table _reslease_tbl( _self, period );
        const auto& lease_from = _reslease_tbl.get( from , ( name{from}.to_string() + " has no resource in chain ").c_str() );
        ultrainio_assert(lease_from.lease_num >= combosize, ( name{from}.to_string() + " has not enough resources to transfer ").c_str() );
        ultrainio_assert(combosize > 0, "resource combosize must be greater than 0" );
        uint32_t cur_block_height = (uint32_t)head_block_number() + 1;
        uint64_t cur_period = cur_block_height * block_interval_seconds()/ seconds_per_period + 1;
        if ( period == 0 ) {
            period = cur_period;
        }
        _reslease_tbl.modify( lease_from, [&]( auto& res ) {
            res.lease_num -= combosize;
            res.modify_block_height = cur_block_height;
        });
        auto lease_to_itr = _reslease_tbl.find(to);
        if(lease_to_itr == _reslease_tbl.end()) {
            lease_to_itr = _reslease_tbl.emplace([&]( auto& res ) {
                res.owner               = to;
                res.lease_num           = combosize;
                res.locked_num          = 0;
                res.start_block_height  = seconds_per_period / block_interval_seconds() * ((uint32_t)period - 1) + 1;
                res.end_block_height    = seconds_per_period / block_interval_seconds() * (uint32_t)period;
                res.modify_block_height = cur_block_height;
            });
        }
        else {
            _reslease_tbl.modify( lease_to_itr, [&]( auto& res ) {
                res.lease_num += combosize;
                res.modify_block_height = cur_block_height;
            });
        }
        if ( period == cur_period ) { //Process current period resources
            uint64_t bytes_per_combo = _gstate.max_ram_size/_gstate.max_resources_number;
            set_resource_limits( lease_from.owner, int64_t(bytes_per_combo*lease_from.lease_num), int64_t(lease_from.lease_num), int64_t(lease_from.lease_num) );
            print("current resource limit  sender:", name{lease_from.owner}, " net:",uint32_t(lease_from.lease_num)," cpu:",uint32_t(lease_from.lease_num)," ram:",int64_t(bytes_per_combo*lease_from.lease_num));
            set_resource_limits( to, int64_t(bytes_per_combo*lease_to_itr->lease_num), int64_t(lease_to_itr->lease_num), int64_t(lease_to_itr->lease_num) );
            print("current resource limit  receiver:", name{to}, " net:", uint32_t(lease_to_itr->lease_num)," cpu:", uint32_t(lease_to_itr->lease_num)," ram:",int64_t(bytes_per_combo*lease_to_itr->lease_num));
        }
        int64_t res_trans_fee = (int64_t)(_gstate.res_transfer_res * combosize);
        ultrainio_assert(res_trans_fee > 0, "resource lease res_trans_fee is abnormal" );
        INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {from,N(active)},
                            { from, N(utrio.fee), asset(res_trans_fee), std::string("transresource fee") } );
    }

    void resource::transaccount( account_name from, account_name to, uint32_t number ) {
        require_auth( from );
        resfreeaccount _resacc_tbl( _self, _self );
        auto resacc_itr = _resacc_tbl.find( from );
        ultrainio_assert( resacc_itr !=  _resacc_tbl.end(), ( name{from}.to_string() + " has no account in chain ").c_str() );
        ultrainio_assert( resacc_itr->free_account_number >= number, ( name{from}.to_string() + " has not enough account to transfer ").c_str() );
        ultrainio_assert(number > 0, "transfer account must be greater than 0" );
        if ( resacc_itr->free_account_number == number ) {
            _resacc_tbl.erase( resacc_itr );
        } else {
            _resacc_tbl.modify( resacc_itr, [&]( auto& res ) {
                res.free_account_number -= number;
            });
        }
        auto resacc_to_itr = _resacc_tbl.find(to);
        if(resacc_to_itr == _resacc_tbl.end()) {
            resacc_to_itr = _resacc_tbl.emplace([&]( auto& res ) {
                res.owner = to;
                res.free_account_number = number;
            });
        }
        else {
            _resacc_tbl.modify( resacc_to_itr, [&]( auto& res ) {
                res.free_account_number += number;
            });
        }
        INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {from,N(active)},
                            { from, N(utrio.fee), asset(10000), std::string("transfer account fee") } );
    }

    void resource::set_next_period_res() {
        uint32_t block_height = (uint32_t)head_block_number() + 1;
        uint32_t cur_periods = block_height/(seconds_per_period / block_interval_seconds()) + 1;
        uint32_t next_periods_start_block = seconds_per_period / block_interval_seconds() * cur_periods + 1;
        uint32_t next_periods_end_block = seconds_per_period / block_interval_seconds() * (cur_periods + 1);
        uint64_t bytes = _gstate.max_ram_size/_gstate.max_resources_number;
        //Set next period resource within the last three day of the year expiration
        if ( (block_height + seconds_per_day / block_interval_seconds()) < next_periods_start_block ) {
            return;
        }
        int32_t calc_num = 0;
        resources_lease_table _reslease_tbl( _self, cur_periods + 1 );
        for(auto leaseiter = _reslease_tbl.begin(); leaseiter != _reslease_tbl.end(); ) {
            if( leaseiter->lease_num == 0 ) {
                print("set_next_period_res  owner:", name{leaseiter->owner}, " next_periods:",(cur_periods + 1)," resouce number is 0");
                leaseiter = _reslease_tbl.erase(leaseiter);
                continue;
            }
            calc_num++;
            if(calc_num > 100) {
                break;
            }
            resources_lease_table cur_periods_res( _self, 0 );
            auto cur_periods_itr = cur_periods_res.find( leaseiter->owner );
            if( cur_periods_itr ==  cur_periods_res.end() ) {
                cur_periods_res.emplace( [&]( auto& tot ) {
                    tot.owner = leaseiter->owner;
                    tot.lease_num = leaseiter->lease_num;
                    tot.locked_num = 0;
                    tot.start_block_height = next_periods_start_block;
                    tot.end_block_height = next_periods_end_block;
                    tot.modify_block_height = block_height;
                });
                _gstate.total_resources_used_number += leaseiter->lease_num;
            } else {
                _gstate.total_resources_used_number += leaseiter->lease_num - cur_periods_itr->lease_num;
                cur_periods_res.modify( cur_periods_itr, [&]( auto& tot ) {
                    tot.lease_num = leaseiter->lease_num;
                    tot.end_block_height = next_periods_end_block;
                    tot.modify_block_height = block_height;
                });
            }
            int64_t  used_ram = 0;
            get_account_ram_usage( leaseiter->owner, &used_ram );
            if ( (uint64_t)used_ram <= leaseiter->lease_num*bytes ) {
                set_resource_limits( leaseiter->owner, int64_t(bytes*leaseiter->lease_num), int64_t(leaseiter->lease_num), int64_t(leaseiter->lease_num) );
                print("set_next_period_res resource limit  receiver:", name{leaseiter->owner}, " net:",uint32_t(leaseiter->lease_num)," cpu:", uint32_t(leaseiter->lease_num)," ram:",int64_t(bytes*leaseiter->lease_num));
            }
            leaseiter = _reslease_tbl.erase(leaseiter);
        }
    }

    void resource::modifyfreeaccount( const account_name owner, uint32_t number) {
        require_auth( _self );
        ultrainio_assert( number > 0, "modifyfreeaccount number is abnormal" );
        resfreeaccount _resacc_tbl( _self, _self );
        auto resacc_itr = _resacc_tbl.find( owner );
        ultrainio_assert( resacc_itr !=  _resacc_tbl.end(), "The current free account is insufficient, please purchase free account" );
        ultrainio_assert( resacc_itr->free_account_number >= number, "Insufficient number of transfer free accounts" );
        if ( resacc_itr->free_account_number == number ) {
            _resacc_tbl.erase( resacc_itr );
            return;
        }
        _resacc_tbl.modify( resacc_itr, [&]( auto& tot ) {
            tot.free_account_number -= number;
        });
    }

    void resource::copyresource() {
        require_auth(_self);

        //copy resource from legacy table in system contract to new table in this contract
        ultrainiosystem::global_state_singleton   gstatesingle(N(ultrainio),N(ultrainio));
        ultrainiosystem::ultrainio_global_state globalst = gstatesingle.get();
        ultrainiosystem::resources_lease_table _legacy_resource( N(ultrainio), N(ultrainio) ); //copy self_chain first
        resources_lease_table _reslease_tbl(_self, 0);// hard code as period 0, because there's only period 0 currently.
        auto legacy_ite = _legacy_resource.cbegin();
        for(; legacy_ite != _legacy_resource.cend(); ++legacy_ite) {
            if(_reslease_tbl.find(legacy_ite->owner) != _reslease_tbl.end()) {
                continue;
            }
            //special handling to filter test record
            if(legacy_ite->owner == N(ray123) || legacy_ite->owner == N(raymond)) {
                set_resource_limits(legacy_ite->owner, 0 , 0, 0);
                continue;
            }
            _reslease_tbl.emplace([&](auto& new_res) {
                new_res.owner = legacy_ite->owner;
                new_res.lease_num = uint16_t(legacy_ite->lease_num);
                new_res.locked_num = 0;
                //special handling
                if(N(u.unitopia.1) == legacy_ite->owner && legacy_ite->lease_num > 9990) {
                    set_resource_limits(legacy_ite->owner, int64_t(_gstate.max_ram_size), 10000, 10000);
                    new_res.lease_num = 10000;
                }
                new_res.start_block_height = legacy_ite->start_block_height;
                new_res.end_block_height = legacy_ite->end_block_height;
                new_res.modify_block_height = legacy_ite->modify_block_height;
                for(const auto& ext : legacy_ite->table_extension) {
                    new_res.table_extension.emplace_back(ext.key, ext.value);
                }
            });
            _gstate.total_resources_used_number += legacy_ite->lease_num;
            //add period info
            if(legacy_ite->lease_num >= 5000) {
                resources_periods_table _resperiods_tbl( _self, N(ultrainio));
                _resperiods_tbl.emplace( [&]( auto& tot ) {
                    tot.periods = 1;
                    tot.owner = legacy_ite->owner;
                    tot.total_lease_num = 10000;
                    tot.modify_block_height = legacy_ite->start_block_height;
                });
            }

            //set free account number if it's master chain
            if(globalst.is_master_chain()) {
                resfreeaccount _resacc_tbl( _self, _self );
                auto resacc_to_itr = _resacc_tbl.find(legacy_ite->owner);
                if(legacy_ite->free_account_number > 0) {
                    if(resacc_to_itr == _resacc_tbl.end()) {
                        resacc_to_itr = _resacc_tbl.emplace([&]( auto& acc_num ) {
                            acc_num.owner = legacy_ite->owner;
                            acc_num.free_account_number = legacy_ite->free_account_number;
                        });
                    } else {
                        _resacc_tbl.modify( resacc_to_itr, [&]( auto& acc_num ) {
                            acc_num.free_account_number += legacy_ite->free_account_number;
                        });
                    }
                }
            }
        }
        if(_gstate.total_resources_used_number > 10000) {
            _gstate.total_resources_used_number = 10000;
        }

        //if master chain, set period records of all sidechains
        ultrainiosystem::chains_table all_chains(N(ultrainio),N(ultrainio));
        auto chain_ite = all_chains.cbegin();
        for(; chain_ite != all_chains.cend(); ++chain_ite) {
            if(chain_ite->chain_name == N(master) || chain_ite->chain_name == N(default)) {
                continue;
            }
            bool has_res_period = false;
            resources_periods_table _resperiods_tbl( _self, chain_ite->chain_name);
            if(_resperiods_tbl.find(0) != _resperiods_tbl.end()) {
                has_res_period = true;
            }
            ultrainiosystem::resources_lease_table _legacy_sidechain_resource( N(ultrainio), chain_ite->chain_name );
            auto res_ite = _legacy_sidechain_resource.cbegin();
            for(; res_ite != _legacy_sidechain_resource.cend(); ++res_ite) {
                if(res_ite->owner == N(ray123) || res_ite->owner == N(raymond)) {
                    continue;
                }
                if(!has_res_period && res_ite->lease_num >= 5000) {
                    _resperiods_tbl.emplace( [&]( auto& tot ) {
                        tot.periods = 1;
                        tot.total_lease_num = 10000;
                        tot.owner = res_ite->owner;
                        tot.modify_block_height = res_ite->start_block_height;
                    });
                }

                //set free account number

                //special handling for sanguo123, no need to set free account numver
                if(N(sanguo123) == res_ite->owner) {
                    continue;
                }

                resfreeaccount _resacc_tbl( _self, _self );
                auto resacc_to_itr = _resacc_tbl.find(res_ite->owner);
                if(resacc_to_itr == _resacc_tbl.end()) {
                    resacc_to_itr = _resacc_tbl.emplace([&]( auto& res ) {
                        res.owner = res_ite->owner;
                        res.free_account_number = res_ite->free_account_number;
                        if(N(u.unitopia.1) == res_ite->owner) {
                            res.free_account_number = uint32_t(10000 * _gstate.free_account_per_res);
                        }
                    });
                } else {
                    _resacc_tbl.modify( resacc_to_itr, [&]( auto& res ) {
                        res.free_account_number += res_ite->free_account_number;
                    });
                }
            }
        }
    }

    void resource::putorder(account_name owner, uint64_t period, uint16_t combosize, asset price, bool decrease) {
        require_auth(owner);

        ultrainio_assert(price.amount > 0, "price cannot be set as zero");
        uint64_t real_period = period;
        uint32_t cur_block_height = (uint32_t)head_block_number() + 1;
        uint32_t period_end_block = 1;
        bool current_period = true;
        uint64_t cur_period_id = cur_block_height * block_interval_seconds()/ seconds_per_period + 1;
        if(period > 0) {
            ultrainio_assert( period >= cur_period_id, "this order is out of date");
            period_end_block = seconds_per_period / block_interval_seconds() * uint32_t(period);
            if(period > uint16_t(cur_period_id)) {
                current_period = false;
            } else {
               period = 0;
            }
        } else {
            period_end_block = seconds_per_period / block_interval_seconds() * uint32_t(cur_period_id);
            real_period = cur_period_id;
        }
        resources_lease_table _reslease_tbl( _self, period );
        auto ite_res = _reslease_tbl.find(owner);
        ultrainio_assert(ite_res != _reslease_tbl.end(), "you don't have resource of this period");
        ultrainio_assert(ite_res->lease_num >= combosize, "you don't have enough resource");
        int64_t new_total_ram = 0;
        if(current_period) {
            //check if has enough rest resource
            int64_t  total_ram = 0;
            int64_t  total_net = 0;
            int64_t  total_cpu = 0;
            get_resource_limits(owner, &total_ram, &total_net, &total_cpu);
            int64_t  used_ram = 0;
            get_account_ram_usage(owner, &used_ram);
            uint64_t bytes_per_combo = _gstate.max_ram_size/_gstate.max_resources_number;
            int64_t need_ram = int64_t(bytes_per_combo * combosize);
            ultrainio_assert(need_ram <= total_ram - used_ram, "you don't have enough rest resource of current period");
            new_total_ram = total_ram - need_ram;
        }
        print("putorder real period:", real_period, "reslease period ", period, "\n");
        ressaletable resorders(_self, real_period);
        auto ite_order = resorders.find(owner);
        if(resorders.end() == ite_order) {
            resorders.emplace([&]( auto& order ) {
                order.owner = owner;
                order.lease_num = combosize;
                order.initial_unit_price = uint64_t(price.amount);
                order.decrease_by_day = decrease;
                order.modify_block_height = cur_block_height;
            });
        } else {
            ultrainio_assert(ite_order->decrease_by_day == decrease, "you already have an order with different price strategy");
            auto realtime_unit_price = ite_order->initial_unit_price;
            if(decrease && current_period) {
                //calculate real time price
                auto rest_seconds = (period_end_block - cur_block_height) * block_interval_seconds();
                auto rest_days = rest_seconds / seconds_per_day;
                if(rest_seconds % seconds_per_day > 0) {
                    rest_days += 1;
                }
                realtime_unit_price = ite_order->initial_unit_price * rest_days / 365;
            }
            ultrainio_assert(realtime_unit_price == uint64_t(price.amount), "you already have an order with different price");

            resorders.modify(ite_order, [&]( auto& order ) {
                order.initial_unit_price = realtime_unit_price;  //change price since the modify_block_height was changed
                order.lease_num += combosize;
                order.modify_block_height = cur_block_height;
            });
        }

        _reslease_tbl.modify(ite_res, [&]( auto& res ) {
            res.lease_num -= combosize;
            res.locked_num += combosize;
        });

        //charge for this action
        INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {owner, N(active)},
                    { owner, N(utrio.fee), asset(1000), std::string("put order") } );

        if(current_period) {
            //reset ram, cpu net limit, the resource in this order will be locked
            set_resource_limits(owner, new_total_ram, int64_t(ite_res->lease_num), int64_t(ite_res->lease_num));
        }
    }

    void resource::updateorder(account_name owner, uint64_t period, asset price, bool decrease) {
        require_auth(owner);
        ultrainio_assert(price.amount > 0, "price cannot be set as zero");
        uint32_t cur_block_height = (uint32_t)head_block_number() + 1;
        uint64_t real_period = period;
        uint64_t cur_period_id = cur_block_height * block_interval_seconds()/ seconds_per_period + 1;
        if(0 == period) {
            real_period = cur_period_id;
        } else {
            ultrainio_assert(period >= cur_period_id, "this order is out of date");
        }

        ressaletable resorders(_self, real_period);
        auto ite_order = resorders.find(owner);
        ultrainio_assert(ite_order != resorders.end(), "order is not found");
        resorders.modify(ite_order, [&]( auto& order ) {
            order.initial_unit_price = uint64_t(price.amount);
            order.decrease_by_day = decrease;
            order.modify_block_height = cur_block_height;
        });

        INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {owner, N(active)},
                    { owner, N(utrio.fee), asset(200), std::string("update order") } );
    }

    void resource::cancelorder(account_name owner, uint64_t period) {
        require_auth(owner);
        uint32_t cur_block_height = (uint32_t)head_block_number() + 1;
        uint64_t real_period = period;
        uint64_t cur_period_id = cur_block_height * block_interval_seconds()/ seconds_per_period + 1;
        bool current_period = true;
        if(0 == period) {
            real_period = cur_period_id;
        } else {
            ultrainio_assert(period >= cur_period_id, "this order is out of date");
            if(period > cur_period_id) {
                current_period = false;
            } else {
                period = 0;
            }
        }

        ressaletable resorders(_self, real_period);
        auto ite_order = resorders.find(owner);
        ultrainio_assert(ite_order != resorders.end(), "order is not found");

        resources_lease_table _reslease_tbl( _self, period );
        auto ite_res = _reslease_tbl.find(owner);
        ultrainio_assert(ite_res != _reslease_tbl.end(), "error: resource is not found");
        ultrainio_assert(ite_order->lease_num == ite_res->locked_num, "resource size in the order is mismatch with locked size");
        uint16_t new_lease_num = ite_res->lease_num + ite_order->lease_num;
        _reslease_tbl.modify(ite_res, [&]( auto& tot ) {
            tot.lease_num = new_lease_num;
            tot.locked_num = 0;
        });

        resorders.erase(ite_order);

        if(current_period) {
            //reset resource
            uint64_t bytes_per_combo = _gstate.max_ram_size/_gstate.max_resources_number;
            set_resource_limits(owner, int64_t(bytes_per_combo * new_lease_num), int64_t(new_lease_num), int64_t(new_lease_num));
        }
    }

    void resource::buyin(account_name owner, uint64_t period, account_name buyer, uint16_t combosize) {
        require_auth(buyer);
        uint32_t cur_block_height = (uint32_t)head_block_number() + 1;

        //get correct period because time is passing
        uint32_t period_end_block = 1;
        bool current_period = true;
        uint64_t real_period = period;
        uint64_t cur_period_id = cur_block_height * block_interval_seconds()/ seconds_per_period + 1;
        if(period > 0) {
            ultrainio_assert(period >= cur_period_id, "this order is out of date");
            period_end_block = seconds_per_period / block_interval_seconds() * uint32_t(period);
            if(period > cur_period_id) {
                current_period = false;
            } else {
                period = 0;
            }
        } else {
            real_period = cur_period_id;
            period_end_block = seconds_per_period / block_interval_seconds() * uint32_t(cur_period_id);
        }

        print("buyin order, real period: ", real_period, " reslease period: ", period, "\n");

        ressaletable resorders(_self, real_period);
        auto ite_order = resorders.find(owner);
        ultrainio_assert(ite_order != resorders.end(), "order is not found");
        ultrainio_assert(ite_order->lease_num >= combosize, "the required combo size is more than the offered size");

        //handle resource table
        resources_lease_table _reslease_tbl( _self, period );
        auto ite_res = _reslease_tbl.find(owner);
        ultrainio_assert(ite_res != _reslease_tbl.end(), "error: resource is not found");
        ultrainio_assert(ite_order->lease_num == ite_res->locked_num, "resource size in the order is mismatch with locked size");

        auto ite_buyer_res = _reslease_tbl.find(buyer);
        uint16_t buyer_new_size = combosize;
        if(ite_buyer_res == _reslease_tbl.end()) {
            _reslease_tbl.emplace([&]( auto& tot ) {
                tot.owner = buyer;
                tot.lease_num = combosize;
                tot.locked_num = 0;
                tot.start_block_height = ite_res->start_block_height;
                tot.end_block_height = ite_res->end_block_height;
                tot.modify_block_height = cur_block_height;
            });
        } else {
            buyer_new_size += ite_buyer_res->lease_num;
            _reslease_tbl.modify(ite_buyer_res, [&]( auto& tot ) {
                tot.lease_num = buyer_new_size;
                tot.modify_block_height = cur_block_height;
            });
        }

        uint16_t left_order_size = ite_order->lease_num - combosize;
        if(left_order_size > 0 || ite_res->lease_num > 0) {
            _reslease_tbl.modify(ite_res, [&]( auto& tot ) {
                tot.locked_num = left_order_size;
                tot.modify_block_height = cur_block_height;
            });
        } else {
            _reslease_tbl.erase(ite_res);
        }

        //charge
        auto realtime_unit_price = ite_order->initial_unit_price;
        if(ite_order->decrease_by_day && current_period) {
            auto rest_days = (period_end_block - cur_block_height)*block_interval_seconds() / seconds_per_day;
            realtime_unit_price = ite_order->initial_unit_price * rest_days / 365;
        }

        INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {buyer, N(active)},
                    { buyer, N(utrio.fee), asset(1000), std::string("buy resource order") } );

        INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {buyer, N(active)},
                    { buyer, owner, asset(int64_t(realtime_unit_price * combosize)), std::string("resource sale") } );

        //handle order table
        if(left_order_size > 0) {
            resorders.modify(ite_order, [&]( auto& order ) {
                order.lease_num = left_order_size;
                order.modify_block_height = (uint32_t)head_block_number() + 1;;
            });
        } else {
            resorders.erase(ite_order);
        }

        if(current_period) {
            //reset resource of buyer
            uint64_t bytes_per_combo = _gstate.max_ram_size/_gstate.max_resources_number;
            set_resource_limits(buyer, int64_t(buyer_new_size * bytes_per_combo), int64_t(buyer_new_size), int64_t(buyer_new_size));
        }
    }

}/// namespace ultrainiores

ULTRAINIO_ABI( ultrainiores::resource, (setresparams)(resourcelease)(transresource)(transaccount)(recycleresource)(onblock)(modifyfreeaccount)(copyresource)(putorder)(updateorder)(cancelorder)(buyin) )
