#pragma once

#include <chainbase/chainbase.hpp>
#include <ultrainio/chain/block_header.hpp>
#include <ultrainio/chain/types.hpp>
#include <ultrainio/chain/worldstate.hpp>
#include <ultrainio/chain/worldstate_file_manager.hpp>

#include "multi_index_includes.hpp"

namespace ultrainio { namespace chain { namespace bls_votes {

    struct bls_votes_info {
        uint32_t block_num;

        bool end_epoch;

        bool valid_bls;

        std::string bls_str;

        bls_votes_info() {}

        bls_votes_info(uint32_t _block_num, bool _end_epoch, bool _valid_bls, const std::string& _bls_str)
                : block_num(_block_num), end_epoch(_end_epoch), valid_bls(_valid_bls), bls_str(_bls_str) {}
    };

    struct bls_votes_object : public chainbase::object<bls_votes_object_type, bls_votes_object> {
        OBJECT_CTOR(bls_votes_object)

        id_type id;

        uint32_t latest_confirmed_block_num = 0;

        block_id_type latest_check_point_id;

        std::vector<bls_votes_info> should_be_confirmed;
    };

    using bls_votes_index = chainbase::shared_multi_index_container<
        bls_votes_object,
        indexed_by<
            ordered_unique<tag<by_id>, member<bls_votes_object, bls_votes_object::id_type, &bls_votes_object::id>>
        >
    >;

    struct worldstate_bls_voters_object {
        uint32_t latest_confirmed_block_num = 0;
        block_id_type latest_check_point_id;
        std::vector<bls_votes_info> should_be_confirmed;
    };

    class bls_votes_manager {
    public:
        static worldstate_bls_voters_object to_worldstate_row(const bls_votes_object& value, const chainbase::database& db, void* data);

        static void from_worldstate_row(worldstate_bls_voters_object&& row, bls_votes_object& value, chainbase::database& db, bool backup, void* data = nullptr);

        explicit bls_votes_manager(chainbase::database& db) : _db(db) {}

        void add_indices(chainbase::database& db);

        void initialize_database();

        bool has_should_be_confirmed_bls(std::string& bls) const;

        bool should_be_confirmed(uint32_t block_num) const;

        void add_confirmed_bls_votes(uint32_t block_num, bool end_epoch, bool valid_bls, const std::string& bls_str);

        bool check_can_confirm(uint32_t block_nun) const;

        void confirm(uint32_t block_num);

        void add_to_worldstate( std::shared_ptr<ws_helper> ws_helper_ptr, chainbase::database& worldstate_db);

        void read_from_worldstate(std::shared_ptr<ws_helper> ws_helper_ptr, chainbase::database& worldstate_db);

    private:
        void clean_no_end_epoch(uint32_t block_num, uint32_t confirmed_block_num);

        chainbase::database& _db;

        int confirm_point_interval = 20;
    };

} } } // ultrainio::chain::bls_votes

CHAINBASE_SET_INDEX_TYPE(ultrainio::chain::bls_votes::bls_votes_object,        ultrainio::chain::bls_votes::bls_votes_index)

FC_REFLECT(ultrainio::chain::bls_votes::bls_votes_info, (block_num)(end_epoch)(valid_bls)(bls_str))
FC_REFLECT(ultrainio::chain::bls_votes::bls_votes_object, (latest_confirmed_block_num)(should_be_confirmed))
