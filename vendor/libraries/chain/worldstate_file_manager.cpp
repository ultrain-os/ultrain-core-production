/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/chain/worldstate_file_manager.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>
#include <iostream>
#include <fc/io/raw.hpp>
#include <sstream>
#include <fc/crypto/sha256.hpp>
#include <fc/io/json.hpp>
#include <appbase/application.hpp>

namespace bfs = boost::filesystem;

namespace ultrainio { namespace chain {

#define WS_DATA_DIR "ultrainio/wssultrain/data/worldstate"
#define WS_DOWNLOAD_RELATIVE_DIR "download"

std::string to_file_name(fc::sha256 chain_id, uint32_t block_height)
{
    fc::variant chain_var;
    fc::to_variant(chain_id, chain_var);

    fc::variant height_var;
    fc::to_variant(block_height, height_var);

    std::string name =  chain_var.as_string() + "-" + height_var.as_string();
    return name;   
}

bool from_file_name(fc::path name, fc::sha256& chain_id, uint32_t& block_height)
{

    std::string text = name.stem().string();
    if(text.empty())
        return false;

    int pos = 0;
    while(text[pos] != '-' && pos < text.size()){
        pos++;
    }

    if(pos >= text.size())
        return false;

    fc::variant chain_var(text.substr(0, pos));
    fc::from_variant(chain_var, chain_id);

    fc::variant height_var(text.substr(pos+1, text.size() - pos - 1));
    fc::from_variant(height_var, block_height);
    return true;   
}


ws_file_reader::ws_file_reader(ws_info node, std::string dir, uint32_t len_per_slice)
:m_info(node)
,m_dir_path(dir)
,m_len_per_slice(len_per_slice)
{

    std::string filePath = m_dir_path + "/" + to_file_name(m_info.chain_id, m_info.block_height) + ".ws";
    m_fd.open(filePath.c_str(), (std::ios::in | std::ios::binary));
}

ws_file_reader::~ws_file_reader()
{
    destory();
    if(m_fd.is_open())
        m_fd.close();
}

void ws_file_reader::destory()
{
    ilog("ws_file_reader destory ${info}", ("info", m_info));
    // delete this;
}

std::vector<char> ws_file_reader::get_data(uint32_t slice_id, bool& isEof)
{
    if(slice_id*m_len_per_slice >= m_info.file_size) {
        return std::vector<char>();
    }

    try {
        std::vector<char>  ret_data;
        int readCnt = 0;
        m_fd.seekg(slice_id*m_len_per_slice, std::ios::beg);
        
        ret_data.resize(m_len_per_slice);
        m_fd.read(ret_data.data(), m_len_per_slice);
        readCnt = m_fd.gcount();
        ret_data.resize(readCnt);
        isEof = m_fd.eof();
        if(isEof)
            m_fd.clear();
        return ret_data;
    } catch (...){
        elog("Error, ws_file_reader error");
        return std::vector<char>(); 
    }
}

ws_file_writer::ws_file_writer(ws_info node, uint32_t len_per_slice, std::string dir, ws_file_manager& m)
:m_info(node)
,m_valid(-1)
,m_file_name()
,m_manager(m),
m_is_write(false)
,m_write_size(0)
,m_len_per_slice(len_per_slice)
{
    std::string download_dir = dir + "/" + WS_DOWNLOAD_RELATIVE_DIR + "/";
    if (!bfs::is_directory(download_dir)){
        bfs::create_directories(download_dir);
    }

    m_file_name = download_dir + to_file_name(m_info.chain_id, m_info.block_height) + ".ws";
    m_fd.open(m_file_name.c_str(), std::ios::out | std::ios::binary);    
    m_is_write = true;
}

ws_file_writer::~ws_file_writer()
{
    destory();
    if(m_fd.is_open())
        m_fd.close();
}

void ws_file_writer::destory()
{
    if(m_fd.is_open())
        m_fd.close();

    if(m_valid != 1){//Delete file
        bfs::remove(m_file_name.c_str());
    } else {
        m_manager.save_info(m_info, WS_DOWNLOAD_RELATIVE_DIR);
    }
}

void ws_file_writer::write_data(uint32_t slice_id, const std::vector<char>& data, uint32_t data_len)
{
    open_write();
    m_fd.seekp(0, std::ios::end);
    int count = m_fd.tellp();

    if (count > 0) {
        char placeholder[1024] = {0};
        while(count < slice_id*m_len_per_slice){
            int cnt = slice_id*m_len_per_slice - count > sizeof(placeholder) ? sizeof(placeholder) : slice_id*m_len_per_slice - count;
            m_fd.write(placeholder, cnt);
            count += cnt;
        }
    }

    m_fd.seekp(slice_id*m_len_per_slice, std::ios::beg);
    m_fd.write(data.data(), data_len);
    m_write_size += data_len;
    return;
}

bool ws_file_writer::is_valid()
{
    if(m_write_size != m_info.file_size)
        return false;

    open_read();
    auto result = m_manager.calculate_file_hash(m_file_name);

    // ilog("isValid ${result} ==? ${hash_string}",  ("result", result.str())("hash_string", m_info.hash_string));
    if (result.str() == m_info.hash_string ) {
        m_valid = 1;
        return true;
    }
    
    m_valid = 0;
    return false;
}

void ws_file_writer::write_finished(){

}

void ws_file_writer::open_write()
{
    if(m_is_write)
        return;

    if(m_fd.is_open())
        m_fd.close();

    m_fd.open(m_file_name.c_str(), std::ios::out | std::ios::binary);    
    m_is_write = true;
}
    
void ws_file_writer::open_read()
{
    if(!m_is_write)
        return;

    if(m_fd.is_open())
        m_fd.close();

    m_fd.open(m_file_name.c_str(), std::ios::in | std::ios::binary);    
    m_is_write = false;
}

ws_file_manager::ws_file_manager(std::string dir)
:m_dir_path(dir)
,m_max_ws_count(MAX_WS_COUNT)
{
    if(m_dir_path.empty()){
        m_dir_path = (fc::app_path() / WS_DATA_DIR).string();
    }
    
    if (!bfs::is_directory(m_dir_path)){
        bfs::create_directories(m_dir_path);
    }

    m_ws_delete_period = {std::chrono::seconds{30}};
    m_ws_delete_check.reset(new boost::asio::steady_timer(appbase::app().get_io_service()));
    start_delete_timer();
}

ws_file_manager::~ws_file_manager()
{

}

bool ws_file_manager::load_local_info_file(const std::string file_name, ws_info& node)
{
    try {
        auto var =  fc::json::from_file(file_name);
        var.as<ws_info>(node);
        return true;
    } catch (...) {
        elog("Error, load ws info error");
        return false;
    }
    return true;
}

std::string ws_file_manager::get_file_path_by_info(fc::sha256& chain_id, uint32_t block_height)
{
    std::string ws_file_name  = m_dir_path + "/" +  to_file_name(chain_id, block_height);
    return ws_file_name;
}

std::list<ws_info> ws_file_manager::get_local_ws_info()
{
    try {
        std::list<ws_info> retList;

        std::string file_format = ".info";
        for (bfs::directory_iterator iter(m_dir_path); iter != bfs::directory_iterator(); ++iter){
            if (!bfs::is_regular_file(iter->status()) || bfs::extension(iter->path().string()) != file_format)
                continue;            

            ws_info node;
            if(!load_local_info_file(iter->path().string(), node))
                continue;      

            fc::sha256 chain_id;
            uint32_t block_height;
            if(!from_file_name(iter->path().string(), chain_id, block_height))
                continue;

            // ilog("getLocalInfo: ${chain_id} ${block_height}", ("chain_id",chain_id)("block_height",block_height));
            if(chain_id != node.chain_id || block_height != node.block_height)
                continue;

            std::string ws_file_name  = m_dir_path + "/" +  to_file_name(node.chain_id, node.block_height) + ".ws";
            // ilog("ws_file_name: ${ws_file_name}", ("ws_file_name", ws_file_name));
            if(!bfs::exists(ws_file_name) || !bfs::is_regular_file(ws_file_name) || bfs::file_size(ws_file_name) != node.file_size)
                continue;

            fc::variant var;
            fc::to_variant(node, var); 
            // ilog("getLocalInfo: ${var}", ("var", var));
            retList.push_back(node); 
        }
        return retList;
    } catch( ... ) {  
      elog("ERROR: get ws info fail");
      return std::list<ws_info>();
   }
   return std::list<ws_info>();
}

void ws_file_manager::save_info(ws_info& node, std::string relative_dir)
{
    std::string info_file_name;
    if (relative_dir.empty()){
      info_file_name = m_dir_path + "/" +  to_file_name(node.chain_id, node.block_height) + ".info";
    } else {
      info_file_name = m_dir_path + "/" + relative_dir + "/" +  to_file_name(node.chain_id, node.block_height) + ".info";
    }

    fc::json::save_to_file(node, info_file_name, true);
}

std::shared_ptr<ws_file_reader> ws_file_manager::get_reader(ws_info node, uint32_t len_per_slice)
{
    auto it = m_reader_map.find(node);
    if (it != m_reader_map.end()) {
        m_is_reader_activate_map[node] = true;
        return it->second;
    }

    auto node_list = get_local_ws_info();
    for(auto &it : node_list){
        if(it == node){
            m_reader_map[node] = std::make_shared<ws_file_reader>(node, m_dir_path, len_per_slice);
            m_is_reader_activate_map[node] = true;
            return m_reader_map[node];       
        }
    }

    return nullptr;
}

std::shared_ptr<ws_file_writer>  ws_file_manager::get_writer(ws_info node, uint32_t len_per_slice)
{
    // auto node_list = get_local_ws_info();
    // for(auto &it : node_list){
    //     if(it == node){
    //         return nullptr;
    //     }
    // }
    return std::make_shared<ws_file_writer>(node, len_per_slice, m_dir_path,  *this);
}

fc::sha256 ws_file_manager::calculate_file_hash(std::string file_name)
{
    std::ifstream fd(file_name.c_str(), std::ios::in | std::ios::binary);    
    fc::sha256::encoder enc;
    char buffer[1024];
    fd.seekg(0, std::ios::beg);

    auto size = bfs::file_size(file_name);
    for(uint32_t i = 0; i < size; i += 1024){
        memset(buffer, 0, sizeof(buffer));
        fd.read(buffer, sizeof(buffer));

        int cnt = fd.gcount();
        enc.write(buffer, cnt);
    }

    return enc.result();
}

void ws_file_manager::set_local_max_count(int number)
{
    m_max_ws_count = number;
}

void ws_file_manager::start_delete_timer()
{    
    m_ws_delete_check->expires_from_now(m_ws_delete_period);
    m_ws_delete_check->async_wait([this](boost::system::error_code ec) {
        ilog("delete worldstate files timer start");
        if (ec.value() == boost::asio::error::operation_aborted) { //cancelled
            ilog("delete timer cancelled");
            return;
        } 

        auto node_list = get_local_ws_info();
        if (node_list.size() > m_max_ws_count){
            node_list.sort([](const ws_info &a, const ws_info &b){
                return a.block_height > b.block_height;
            });

            auto count = m_max_ws_count;
            for(auto &node : node_list){
                auto reader_itor = m_reader_map.find(node);
                auto is_activate_itor = m_is_reader_activate_map.find(node);
                if (is_activate_itor != m_is_reader_activate_map.end() ) {
                    if(m_is_reader_activate_map[node] == true){
                        //set to false, erase from map in next timeout
                        m_is_reader_activate_map[node] = false;
                        continue;
                    } else {
                        if (reader_itor != m_reader_map.end()) {
                            m_is_reader_activate_map.erase(node);
                            m_reader_map.erase(node);
                        }
                    }               
                }

                if (m_max_ws_count == -1) //not set max count, keep all files
                    continue;

                if(count > 0){
                    count--;
                    continue;
                }

                std::string name = to_file_name(node.chain_id, node.block_height);
                std::string ws_file_name  = m_dir_path + "/" +  name + ".ws";
                std::string info_file_name  = m_dir_path + "/" +  name + ".info";
                fc::remove(path(info_file_name));
                fc::remove(path(ws_file_name));
            }
        }
        start_delete_timer();
        ilog("delete  worldstate files timer end");
    });
}

ws_helper::ws_helper(std::string old_ws, std::string new_ws)
:m_old_ws_path(old_ws)
,m_new_ws_path(new_ws)
{
    //output stream
    if(bfs::exists(m_old_ws_path + ".ws") && !bfs::exists(m_old_ws_path + ".id")){
        m_reader_fd = std::ifstream(m_old_ws_path + ".ws", (std::ios::in | std::ios::binary));
        m_reader = std::make_shared<istream_worldstate_reader>(m_reader_fd);

        m_id_reader_fd = std::ifstream(m_old_ws_path + ".id", (std::ios::in | std::ios::binary));
        m_id_reader = std::make_shared<istream_worldstate_id_reader>(m_id_reader_fd);
    }

    //input stream
    m_writer_fd = std::ofstream(m_new_ws_path + ".ws", (std::ios::out | std::ios::binary));
    m_writer = std::make_shared<ostream_worldstate_writer>(m_writer_fd);

    m_id_writer_fd = std::ofstream(m_new_ws_path + ".id", (std::ios::out | std::ios::binary));
    m_id_writer = std::make_shared<ostream_worldstate_id_writer>(m_id_writer_fd);
}

ws_helper::~ws_helper()
{
    if (m_reader_fd.is_open()) 
        m_reader_fd.close();
    if (m_id_reader_fd.is_open()) 
        m_id_reader_fd.close();

    if (m_writer_fd.is_open()) { 
        m_writer_fd.flush();
        m_writer_fd.close();
    }

    if (m_id_writer_fd.is_open()) {
        m_id_writer_fd.flush();
        m_id_writer_fd.close();
    }
}

std::shared_ptr<istream_worldstate_reader> ws_helper::get_reader()
{
    return m_reader;
}

std::shared_ptr<istream_worldstate_id_reader> ws_helper::get_id_reader()
{
    return m_id_reader;
}

std::shared_ptr<ostream_worldstate_writer> ws_helper::get_writer()
{
    return m_writer;
}

std::shared_ptr<ostream_worldstate_id_writer> ws_helper::get_id_writer()
{
    return m_id_writer;
}

}}