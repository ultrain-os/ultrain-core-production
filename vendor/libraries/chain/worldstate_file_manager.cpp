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

namespace bfs = boost::filesystem;

namespace ultrainio { namespace chain {

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
    if(m_fd.is_open())
        m_fd.close();
}

void ws_file_reader::destory()
{
    delete this;
}

std::vector<char> ws_file_reader::get_data(uint32_t slice_id, bool& isEof)
{
    if(slice_id*m_len_per_slice >= m_info.file_size)
        return std::vector<char>();

    m_fd.seekg(slice_id*m_len_per_slice, std::ios::beg);
    std::vector<char>  ret_data;
    ret_data.resize(m_len_per_slice);
    m_fd.read(ret_data.data(), m_len_per_slice);
    ret_data.resize(m_fd.gcount());
    isEof = m_fd.eof();
    return ret_data;
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
    m_file_name = dir + "/" + to_file_name(m_info.chain_id, m_info.block_height) + ".ws";
    m_fd.open(m_file_name.c_str(), std::ios::out | std::ios::binary);    
    m_is_write = true;
}

ws_file_writer::~ws_file_writer()
{
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
        m_manager.save_info(m_info);
    }

    delete this;
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
    fc::sha256::encoder enc;
    char buffer[1024];
    m_fd.seekg(0, std::ios::beg);

    for(uint32_t i = 0; i < m_write_size; i += 1024){
        memset(buffer, 0, sizeof(buffer));
        m_fd.read(buffer, sizeof(buffer));

        int cnt = m_fd.gcount();
        enc.write(buffer, cnt);
    }

    auto result = enc.result();
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
{
    if(m_dir_path.empty()){
        m_dir_path = (fc::app_path() / "nodultrain/data/worldstates").string();
    }
    
    if (!bfs::is_directory(m_dir_path)){
        bfs::create_directories(m_dir_path);
    }
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
    }
    return true;
}

std::string ws_file_manager::get_file_path_by_info(fc::sha256& chain_id, uint32_t block_height)
{
    std::string ws_file_name  = m_dir_path + "/" +  to_file_name(chain_id, block_height) + ".ws";
    return ws_file_name;
}

std::list<ws_info> ws_file_manager::get_local_ws_info()
{
    try {
        std::list<ws_info> retList;

        std::string file_format = ".info";
        for (bfs::directory_iterator iter(m_dir_path); iter != bfs::directory_iterator(); ++iter){
            ilog("getLocalInfo: path ${chain_id}", ("chain_id",iter->path().string()));
            if (!bfs::is_regular_file(iter->status()) || bfs::extension(iter->path().string()) != file_format)
                continue;            

            ws_info node;
            if(!load_local_info_file(iter->path().string(), node))
                continue;      

            fc::sha256 chain_id;
            uint32_t block_height;
            if(!from_file_name(iter->path().string(), chain_id, block_height))
                continue;

            ilog("getLocalInfo: ${chain_id} ${block_height}", ("chain_id",chain_id)("block_height",block_height));
            if(chain_id != node.chain_id || block_height != node.block_height)
                continue;

            std::string ws_file_name  = m_dir_path + "/" +  to_file_name(node.chain_id, node.block_height) + ".ws";
            ilog("ws_file_name: ${ws_file_name}", ("ws_file_name", ws_file_name));
            if(!bfs::exists(ws_file_name) || !bfs::is_regular_file(ws_file_name) || bfs::file_size(ws_file_name) != node.file_size)
                continue;

            fc::variant var;
            fc::to_variant(node, var); 
            ilog("getLocalInfo: ${var}", ("var", var));
            retList.push_back(node); 
        }
        return retList;
    } catch( ... ) {  
      elog("ERROR: get ws info fail");
      return std::list<ws_info>();
   }
   return std::list<ws_info>();
}

void ws_file_manager::save_info(ws_info& node)
{
    std::string info_file_name  = m_dir_path + "/" +  to_file_name(node.chain_id, node.block_height) + ".info";
    fc::json::save_to_file(node, info_file_name, true);
}

ws_file_reader* ws_file_manager::get_reader(ws_info node, uint32_t len_per_slice)
{
    auto node_list = get_local_ws_info();
    for(auto &it : node_list){
        if(it == node){
            return new ws_file_reader(node, m_dir_path, len_per_slice);       
        }
    }

    return nullptr;
}

ws_file_writer* ws_file_manager::get_writer(ws_info node, uint32_t len_per_slice)
{
    auto node_list = get_local_ws_info();
    for(auto &it : node_list){
        if(it == node){
            return nullptr;
        }
    }
    return new ws_file_writer(node, len_per_slice, m_dir_path,  *this);
}

}}