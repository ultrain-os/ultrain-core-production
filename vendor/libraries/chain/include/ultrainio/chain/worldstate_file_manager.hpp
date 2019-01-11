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
#include <boost/asio/steady_timer.hpp>
#include <fc/crypto/sha256.hpp>
#include <ultrainio/chain/types.hpp>

#define MAX_WS_COUNT 5

namespace ultrainio { namespace chain {
    struct ws_info{
        fc::sha256 chain_id;
        uint32_t block_height; 
        std::string  hash_string;//hash of worldstate file
        uint32_t  file_size;
        friend bool operator == ( const ws_info& a, const ws_info& b ) {
            return a.chain_id == b.chain_id && a.block_height == b.block_height && a.hash_string == b.hash_string && a.file_size == b.file_size;
        }
        friend bool operator != ( const ws_info& a, const ws_info& b ) {
            return a.chain_id != b.chain_id || a.block_height != b.block_height || a.hash_string != b.hash_string || a.file_size != b.file_size;
        }
        friend bool operator < ( const ws_info& a, const ws_info& b ) {
            if(a.block_height < b.block_height )
                return true;
            else if (a.block_height == b.block_height && a.chain_id < b.chain_id)
                return true;

            return false;
        }
    };

    class ws_file_manager;
    class ws_file_reader
    {
        public:
            std::vector<char> get_data(uint32_t slice_id, bool& isEof);            
            ws_file_reader(ws_info node, std::string dir, uint32_t len_per_slice);
            ~ws_file_reader();
            
        private:
            void destory();
        private:
            ws_info m_info;
            std::ifstream m_fd;
            std::string m_dir_path;
            uint32_t m_len_per_slice;
            friend class ws_file_manager;
    };

    class ws_file_writer
    {
        public:
            void write_data(uint32_t slice_id, const std::vector<char>& data, uint32_t data_len);
            void write_finished();
            bool is_valid();                  
            ws_file_writer(ws_info node, uint32_t len_per_slice, std::string dir, ws_file_manager& m);
            ~ws_file_writer();

        private:
            void open_write();
            void open_read();
            void destory();  
        private:
            ws_info m_info;
            std::fstream m_fd;
            int m_valid;
            std::string m_file_name;
            ws_file_manager& m_manager;
            bool m_is_write;
            uint32_t m_write_size;
            uint32_t m_len_per_slice;
            friend class ws_file_manager;
    };

    class ws_file_manager
    {
        public:
            ws_file_manager(std::string dir = std::string());
            ~ws_file_manager();        
        public:   
            std::list<ws_info> get_local_ws_info();
            std::shared_ptr<ws_file_reader> get_reader(ws_info node, uint32_t len_per_slice);
            std::shared_ptr<ws_file_writer> get_writer(ws_info node, uint32_t len_per_slice);
            fc::sha256 calculate_file_hash(std::string file_name);
            std::string get_file_path_by_info(fc::sha256& chain_id, uint32_t block_height);
            void save_info(ws_info& node);
            void set_local_max_count(int number);
        private:
            bool load_local_info_file(const std::string file_name, ws_info& node);
            void start_delete_timer();
        private:
            std::string m_dir_path;
            int m_max_ws_count{MAX_WS_COUNT};
            boost::asio::steady_timer::duration   m_ws_delete_period;
            unique_ptr<boost::asio::steady_timer> m_ws_delete_check;
            std::map<ws_info, std::shared_ptr<ws_file_reader>> m_reader_map;
            std::map<ws_info, bool> m_is_reader_activate_map;
    };
}}

FC_REFLECT(ultrainio::chain::ws_info, (chain_id)(block_height)(hash_string)(file_size))
