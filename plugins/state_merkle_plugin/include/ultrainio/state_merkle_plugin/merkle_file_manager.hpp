/**                                                                             
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <fc/exception/exception.hpp>
#include <list>
#include <string>
#include <fc/reflect/reflect.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <fc/crypto/sha256.hpp>
#include <ultrainio/chain/incremental_merkle.hpp>
//#include <ultrainio/chain/types.hpp>

namespace ultrainio {
    namespace bfs = boost::filesystem;
    constexpr int merkle_interval = 100;

    struct merkle_info{
        uint32_t block_num;
        std::string merkle;
        merkle_info(uint32_t blk,std::string m):block_num{blk},merkle{m}{}
    };

    struct merkle_infos{
        uint32_t block_num;
        std::string merkle;
        bool    isValid{false};
        std::vector<merkle_info> infos;

        void reset();
    };

    struct merkle_file_writer{
        merkle_infos m_infos{};
        ultrainio::chain::incremental_merkle mroot{};
        bfs::path m_path{};
        std::string         chainid;

        void write_data(const uint32_t block_num,const ultrainio::chain::incremental_merkle merkle);
        void commit_merkle(const uint32_t blk);
        merkle_file_writer(std::string chainid, uint32_t blk);
    };

    struct merkle_file_manager {
        merkle_file_writer  writer;
        uint32_t            max_file_count{10};
        merkle_file_manager(std::string chain,uint32_t blk,bfs::path dir);
        void check_files();
        void write_data(const uint32_t block_num,const ultrainio::chain::incremental_merkle merkle);

    };


} // namespace ultrainio

FC_REFLECT(ultrainio::merkle_info, (block_num)(merkle))
FC_REFLECT(ultrainio::merkle_infos, (block_num)(merkle)(isValid)(infos))
