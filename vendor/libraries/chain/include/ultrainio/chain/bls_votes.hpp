#pragma once

#include <chainbase/chainbase.hpp>
#include <ultrainio/chain/block_header.hpp>
#include <ultrainio/chain/types.hpp>
#include <ultrainio/chain/worldstate.hpp>
#include <ultrainio/chain/worldstate_file_manager.hpp>

#include "multi_index_includes.hpp"

namespace ultrainio { namespace chain { namespace bls_votes {

    struct shared_bls_votes_info;
    struct bls_votes_info {
        uint32_t block_num = 0;

        bool end_epoch = false;

        bool valid_bls = false;

        std::string bls_str = std::string();

        bls_votes_info() {}

        bls_votes_info(uint32_t _block_num, bool _end_epoch, bool _valid_bls, const std::string& _bls_str)
                : block_num(_block_num), end_epoch(_end_epoch), valid_bls(_valid_bls), bls_str(_bls_str) {}
    };

    struct shared_bls_votes_info {
        template <typename Allocator>
        shared_bls_votes_info(uint32_t _block_num, bool _end_epoch, bool _valid_bls, const std::string& _bls_str, chainbase::allocator<Allocator> alloc)
                : block_num(_block_num), end_epoch(_end_epoch), valid_bls(_valid_bls), bls_str(_bls_str.begin(), _bls_str.end(), alloc) {}

        uint32_t block_num = 0;

        bool end_epoch = false;

        bool valid_bls = false;

        shared_string bls_str;

        bls_votes_info to_info() {
            return bls_votes_info(this->block_num, this->end_epoch, this->valid_bls, std::string(this->bls_str.begin(), this->bls_str.end()));
        }
    };

    struct bls_votes_object : public chainbase::object<bls_votes_object_type, bls_votes_object> {
        OBJECT_CTOR(bls_votes_object, (should_be_confirmed))

        id_type id;

        uint32_t latest_confirmed_block_num = 0;

        block_id_type latest_check_point_id = block_id_type();

        shared_vector<shared_bls_votes_info> should_be_confirmed;
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
        explicit bls_votes_manager(chainbase::database& db) : _db(db) {}

        void add_indices(chainbase::database& db);

        void initialize_database();

        block_id_type get_latest_check_point_id() const;

        void set_latest_check_point_id(block_id_type id);

        bool has_should_be_confirmed_bls(std::string& bls) const;

        bool should_be_confirmed(uint32_t block_num) const;

        void add_confirmed_bls_votes(uint32_t block_num, bool end_epoch, bool valid_bls, const std::string& bls_str);

        bool check_can_confirm(uint32_t block_nun) const;

        void confirm(uint32_t block_num);

        void add_to_worldstate( std::shared_ptr<ws_helper> ws_helper_ptr, chainbase::database& worldstate_db);

        void read_from_worldstate(std::shared_ptr<ws_helper> ws_helper_ptr, chainbase::database& worldstate_db);

    private:
        chainbase::database& _db;

        int confirm_point_interval = 20;
    };

} } } // ultrainio::chain::bls_votes

CHAINBASE_SET_INDEX_TYPE(ultrainio::chain::bls_votes::bls_votes_object,        ultrainio::chain::bls_votes::bls_votes_index)

FC_REFLECT(ultrainio::chain::bls_votes::bls_votes_info, (block_num)(end_epoch)(valid_bls)(bls_str))
FC_REFLECT(ultrainio::chain::bls_votes::bls_votes_object, (latest_confirmed_block_num)(latest_check_point_id)(should_be_confirmed))
FC_REFLECT(ultrainio::chain::bls_votes::worldstate_bls_voters_object, (latest_confirmed_block_num)(latest_check_point_id)(should_be_confirmed))
