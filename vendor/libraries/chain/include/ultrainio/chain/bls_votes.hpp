#pragma once

#include <chainbase/chainbase.hpp>
#include <ultrainio/chain/block_header.hpp>
#include <ultrainio/chain/types.hpp>

#include "multi_index_includes.hpp"

namespace ultrainio { namespace chain { namespace bls_votes {

    struct bls_votes_info {
        uint32_t block_num;

        bool end_epoch;

        bool valid_bls;

        std::string bls_str;

        bls_votes_info(uint32_t _block_num, bool _end_epoch, bool _valid_bls, const std::string& _bls_str)
                : block_num(_block_num), end_epoch(_end_epoch), valid_bls(_valid_bls), bls_str(_bls_str) {}
    };

    struct bls_votes_object : public chainbase::object<bls_votes_object_type, bls_votes_object> {
        OBJECT_CTOR(bls_votes_object)

        id_type id;

        uint32_t latest_confirmed_block_num = 0;

        std::vector<bls_votes_info> should_be_confirmed;
    };

    using bls_votes_index = chainbase::shared_multi_index_container<
        bls_votes_object,
        indexed_by<
            ordered_unique<tag<by_id>, member<bls_votes_object, bls_votes_object::id_type, &bls_votes_object::id>>
        >
    >;


    class bls_votes_manager {
    public:
        explicit bls_votes_manager(chainbase::database& db) : _db(db) {}

        void add_indices(chainbase::database& db);

        void initialize_database();

        bool has_should_be_confirmed_bls(std::string& bls);

        bool should_be_confirmed(uint32_t block_num);

        void add_confirmed_bls_votes(uint32_t block_num, bool end_epoch, bool valid_bls, const std::string& bls_str);

        bool check_can_confirm(uint32_t block_nun);

        void confirm(uint32_t block_num);

    private:
        void clean_no_end_epoch(uint32_t block_num, uint32_t confirmed_block_num);

        chainbase::database& _db;
    };

} } } // ultrainio::chain::bls_votes

CHAINBASE_SET_INDEX_TYPE(ultrainio::chain::bls_votes::bls_votes_object,        ultrainio::chain::bls_votes::bls_votes_index)

FC_REFLECT(ultrainio::chain::bls_votes::bls_votes_info, (block_num)(end_epoch)(valid_bls)(bls_str))
FC_REFLECT(ultrainio::chain::bls_votes::bls_votes_object, (latest_confirmed_block_num)(should_be_confirmed))
