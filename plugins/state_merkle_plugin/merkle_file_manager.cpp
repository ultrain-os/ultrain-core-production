/**                                                                             
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>
#include <iostream>
#include <fc/io/raw.hpp>
#include <sstream>
#include <fc/crypto/sha256.hpp>
#include <fc/io/json.hpp>
#include <fc/scoped_exit.hpp>
#include <appbase/application.hpp>

#include <ultrainio/state_merkle_plugin/merkle_file_manager.hpp>
#include <ultrainio/chain/exceptions.hpp>
namespace bfs = boost::filesystem;
namespace ultrainio {

    namespace chain{
    extern bool from_file_name(fc::path name, fc::sha256& chain_id, uint32_t& block_height);
    }

    void merkle_infos::reset()
    {
        infos.clear();
        merkle.clear();
        isValid=false;
    }

    merkle_file_writer::merkle_file_writer(std::string id,uint32_t blk):chainid{id}
    {
        if (chainid.empty()){
            chainid="00000000000000000000000000000000";
        }
        m_infos.block_num=(1+(blk-1)/merkle_interval)*merkle_interval;
    }

    void merkle_file_writer::write_data(const uint32_t block_num,const ultrainio::chain::incremental_merkle merkle)
    {
        mroot.append(merkle.get_root());
        m_infos.infos.emplace_back(block_num,merkle.get_root().str());
        if (block_num % merkle_interval == 0)
            commit_merkle(block_num);
    }

    void merkle_file_writer::commit_merkle(const uint32_t blk)
    {
        m_infos.merkle=mroot.get_root().str();
        if (m_infos.infos.size() == merkle_interval)
            m_infos.isValid=true;
        auto m_file_name=m_path/(chainid+"-"+std::to_string(m_infos.block_num)+".merkle");
        fc::json::save_to_file(m_infos, m_file_name.string(), true);
        mroot={};
        m_infos.reset();
        m_infos.block_num=blk+merkle_interval;
    }

    merkle_file_manager::merkle_file_manager(std::string chain,uint32_t blk,bfs::path dir):writer{chain,blk}
    {
        writer.m_path=dir;
        if (!fc::is_directory(writer.m_path)){
            fc::create_directories(writer.m_path);
        }
    }

    void merkle_file_manager::check_files()
    {
        std::string file_format = ".merkle";
        for (bfs::directory_iterator iter(writer.m_path); iter != bfs::directory_iterator(); ++iter){
             if (!bfs::is_regular_file(iter->status()) || bfs::extension(iter->path().string()) != file_format)
                 continue;
             fc::sha256 chain_id;
             uint32_t block_height;
             if(!chain::from_file_name(iter->path().string(), chain_id, block_height))
                 continue;
             if(chain_id.str() != writer.chainid)
                 continue;
             if(block_height+max_file_count*merkle_interval < writer.m_infos.block_num)
                 fc::remove(iter->path());
         }
    }
    void merkle_file_manager::write_data(const uint32_t block_num,const ultrainio::chain::incremental_merkle merkle)
    {
        writer.write_data(block_num,merkle);
        check_files();
    }
} // namespace ultrainio
