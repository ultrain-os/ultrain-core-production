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
#include <ultrainio/chain/worldstate.hpp>

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
            void save_info(ws_info& node, std::string relative_dir="");
            void set_local_max_count(int number);
        private:
            bool load_local_info_file(const std::string file_name, ws_info& node);
            void start_delete_timer();
        private:
            std::string m_dir_path;
            int m_max_ws_count;
            boost::asio::steady_timer::duration   m_ws_delete_period;
            unique_ptr<boost::asio::steady_timer> m_ws_delete_check;
            std::map<ws_info, std::shared_ptr<ws_file_reader>> m_reader_map;
            std::map<ws_info, bool> m_is_reader_activate_map;
    };


    class ws_helper
    {
       public:
            ws_helper(std::string old_ws, std::string new_ws);
            ~ws_helper();        
        public:
            std::shared_ptr<istream_worldstate_reader> get_reader();
            std::shared_ptr<istream_worldstate_id_reader> get_id_reader();
            std::shared_ptr<ostream_worldstate_writer> get_writer();
            std::shared_ptr<ostream_worldstate_id_writer> get_id_writer();

            template<typename index_t> void restore_backup_indices(chainbase::database& worldstate_db, bool backup = true, void* data = nullptr){
                using value_t = typename index_t::value_type;
                
                if(!get_id_reader() || !get_reader()) { 
                    ilog("Don't exist old ws file, return"); 
                    return;
                }

                get_reader()->read_section<value_t>([&]( auto& reader_section ) {
                    get_id_reader()->read_start_id_section(boost::core::demangle(typeid(value_t).name()));
                
                    bool more = !reader_section.empty();
                    bool id_more = !get_id_reader()->empty();
                    ULTRAIN_ASSERT(more == id_more, worldstate_exception, "Restore to backup indices error: the ws data conflict ");
                    
                    while(more) {// insert the record to backup
                        uint64_t old_id = 0, size = 0;
                        get_id_reader()->read_id_row(old_id, size);
                        // ilog("test id reader: ${t} ${s}", ("t", old_id)("s", size));

                        index_utils<index_t>::create(worldstate_db, [&]( auto &row ) {
                            row.id._id = old_id;
                            more = reader_section.read_row(row, worldstate_db, backup, data);                           
                        }, true);
                        id_more = get_id_reader()->is_more();
                        ULTRAIN_ASSERT(more == id_more, worldstate_exception, "Restore to backup indices error: the ws data conflict ");
                    }

                    get_id_reader()->clear_id_section();
                });
            };

            template<typename index_t> void store_backup_indices(chainbase::database& worldstate_db, void* data = nullptr){
                using value_t = typename index_t::value_type;
                ilog("store_backup_indices");
                ULTRAIN_ASSERT(get_writer() && get_id_writer(), worldstate_exception, "Ws writer is not exist!");

                get_id_writer()->write_start_id_section(boost::core::demangle(typeid(value_t).name()));

                get_writer()->write_section<value_t>([&]( auto& section ){
                    index_utils<index_t>::walk(worldstate_db, [&]( const auto &row ) {
                        get_id_writer()->write_row_id(row.id._id, 0);
                        // ilog("test id writer:  ${t} ${s}", ("t", row.id._id)("s", 0));

                        section.add_row(row, worldstate_db, data);
                        auto length = get_writer()->write_length();
                    });
                });

                get_id_writer()->write_end_id_section();
                ilog("store_backup_indices done");
            };

            template<typename index_t> void handle_indices(chainbase::database& worldstate_db){
                using value_t = typename index_t::value_type;
                ilog("handle_indices: ${t}", ("t", boost::core::demangle(typeid(value_t).name())));

                auto& cache_node = worldstate_db.get_mutable_index<index_t>().cache().front();
                ilog("remove/modify/create size: ${s} ${t} ${y}", ("s", cache_node.removed_ids.size())("t", cache_node.modify_values.size())("y", cache_node.new_values.size()));
                ilog("Cache count: ${s}", ("s", worldstate_db.get_mutable_index<index_t>().cache().size()));
                ilog("Backup size: ${s}", ("s", worldstate_db.get_mutable_index<index_t>().backup_indices().size()));
                
                //1:  add to backup if exit old ws file
                restore_backup_indices<index_t>(worldstate_db);

                //2:  squach backup and cache
                worldstate_db.get_mutable_index<index_t>().process_cache();

                //3. read all record from backup, write to new ws file
                store_backup_indices<index_t>(worldstate_db);

                // 4. clear backup_indices
                (const_cast<index_t&>(worldstate_db.get_mutable_index<index_t>().backup_indices())).clear();
                ilog("handle_indices done");
            };

        private:
            std::string m_old_ws_path;
            std::string m_new_ws_path;
            std::shared_ptr<ostream_worldstate_writer> m_writer;
            std::shared_ptr<istream_worldstate_reader> m_reader;
            std::shared_ptr<ostream_worldstate_id_writer> m_id_writer;
            std::shared_ptr<istream_worldstate_id_reader> m_id_reader;
            std::ofstream m_writer_fd;
            std::ifstream m_reader_fd;
            std::ofstream m_id_writer_fd;
            std::ifstream m_id_reader_fd;
    };
}}

FC_REFLECT(ultrainio::chain::ws_info, (chain_id)(block_height)(hash_string)(file_size))
