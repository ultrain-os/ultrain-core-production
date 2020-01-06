/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
//#include <ultrainio.system/ultrainio.system.hpp>
#include <ultrainiolib/asset.hpp>
#include <ultrainiolib/time.hpp>
#include <ultrainiolib/singleton.hpp>
#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/types.hpp>
#include <ultrainiolib/privileged.hpp>
#include <string>
#include <vector>
#include <ultrainio.system/ultrainio.system.hpp>

namespace ultrainiores {
    using namespace ultrainio;
    using namespace ultrainiosystem;

    static constexpr uint32_t seconds_per_period = 365*24*3600;

    struct resource_global_params {
        bool                 is_allow_buy_res;
        bool                 is_pending_check;
        uint64_t             max_resources_number = 10'000;
        uint64_t             total_resources_used_number = 0;
        uint64_t             max_ram_size = 32ll*1024 * 1024 * 1024;
        uint16_t             free_account_per_res = 50;
        uint64_t             res_transfer_res = 10000;
        uint64_t             resource_fee = 4000'000;
        exten_types          table_extension;

        ULTRAINLIB_SERIALIZE(resource_global_params, (is_allow_buy_res)(is_pending_check)(max_resources_number)
                                    (total_resources_used_number)(max_ram_size)(free_account_per_res)
                                    (res_transfer_res)(resource_fee)(table_extension))
    };
    typedef ultrainio::singleton<N(global), resource_global_params> global_state_singleton;

    struct resources_lease {
        account_name   owner;
        uint16_t       lease_num = 0;
        uint16_t       locked_num = 0;
        uint32_t       start_block_height = 0;
        uint32_t       end_block_height = 0;
        uint32_t       modify_block_height = 0;
        exten_types    table_extension;
        uint64_t  primary_key()const { return owner; }

        // explicit serialization macro is not necessary, used here only to improve compilation time
        ULTRAINLIB_SERIALIZE( resources_lease, (owner)(lease_num)(locked_num)(start_block_height)
                                (end_block_height)(modify_block_height)(table_extension) )
    };
    typedef ultrainio::multi_index< N(reslease), resources_lease>      resources_lease_table;

    struct resources_periods {
        uint64_t       periods = 0;
        account_name   owner;
        uint32_t       modify_block_height = 0;
        exten_types    table_extension;

        uint64_t  primary_key()const { return periods; }

        ULTRAINLIB_SERIALIZE( resources_periods, (periods)(owner)(modify_block_height)(table_extension) )
    };
    typedef ultrainio::multi_index< N(resperiods), resources_periods>      resources_periods_table;

    struct pending_deltable {
        account_name   owner;
        uint64_t  primary_key()const { return owner; }
        ULTRAINLIB_SERIALIZE( pending_deltable, (owner) )
    };
    typedef ultrainio::multi_index< N(penddeltab), pending_deltable>   penddeltable;

    struct res_free_account {
        account_name   owner;
        uint32_t       free_account_number = 0;
        exten_types    table_extension;
        uint64_t  primary_key()const { return owner; }
        ULTRAINLIB_SERIALIZE( res_free_account, (owner)(free_account_number)(table_extension) )
    };
    typedef ultrainio::multi_index< N(resfreeacc), res_free_account>      resfreeaccount;

    struct resource_sale {
        account_name owner;
        uint16_t    lease_num;
        uint64_t    initial_unit_price;
        bool        decrease_by_day;
        uint32_t    modify_block_height;
        exten_types    table_extension;

        uint64_t  primary_key()const { return owner; }

        ULTRAINLIB_SERIALIZE(resource_sale, (owner)(lease_num)(initial_unit_price)(decrease_by_day)(modify_block_height)(table_extension))
    };
    typedef ultrainio::multi_index< N(ressale), resource_sale>  ressaletable;

    class resource : public contract {
        public:
            resource( account_name s );
            ~resource();
            void setresparams( const resource_global_params& params );
            void resourcelease( account_name from, account_name receiver,
                                uint16_t combosize, uint64_t period, name location );
            void transresource( account_name from, account_name to, uint16_t combosize, uint64_t period );
            void transaccount( account_name from, account_name to, uint32_t number );
            void recycleresource( const account_name owner );
            void onblock();
            void modifyfreeaccount( const account_name owner, uint32_t number );
            void copyresource();
            void putorder(account_name owner, uint64_t period, uint16_t combosize, asset price, bool decrease);
            void updateorder(account_name owner, uint64_t period, asset price, bool decrease);
            void cancelorder(account_name owner, uint64_t period);
            void buyin(account_name owner, uint64_t period, account_name buyer, uint16_t combosize);

        private:
            global_state_singleton   _global;
            resource_global_params   _gstate;
            void set_next_period_res();
            void check_res_expire();
            void del_expire_table();
            void clear_expire_contract( account_name owner );
            void check_res_order_expire();

    };
}/// namespace ultrainiores
