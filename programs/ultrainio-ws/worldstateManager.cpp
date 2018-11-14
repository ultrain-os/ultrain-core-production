/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include "worldstateManager.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>
#include <iostream>
#include <fc/io/raw.hpp>
#include <sstream>
#include <fc/crypto/sha256.hpp>

namespace bfs = boost::filesystem;

// std::string path = "ws.conf";
namespace ultrainio { namespace worldstate {


worldstateFileReader::worldstateFileReader(wsNode& node, std::string dir)
:ws(node)
,dirPath(dir)
{
    std::string filePath = dirPath + "/" + node.hashString + ".bin";
    fd.open(filePath.c_str(), (std::ios::in | std::ios::binary));
}

worldstateFileReader::~worldstateFileReader()
{
    if(fd.is_open())
        fd.close();
}

void worldstateFileReader::destory()
{
    delete this;
}

std::vector<char>  worldstateFileReader::getWsData(uint64_t len, uint64_t startPos)
{
    if(startPos >= ws.totalSize || startPos + len >= ws.totalSize)
        return std::vector<char>();

    fd.seekg(startPos, std::ios::beg);
    std::vector<char>  retData;
    retData.resize(len);
    fd.read(retData.data(), len);
    return retData;
}

worldstateFileWriter::worldstateFileWriter(std::string hash, uint32_t blockHeight, std::string dir, worldstateManager& m)
:valid(-1)
,fileName()
,manager(m),
isWrite(false)
{
    ws.blockHeight = blockHeight;
    ws.hashString = hash;
    ws.totalSize = 0;
    fileName = dir + "/" + hash + ".bin";

    fd.open(fileName.c_str(), std::ios::out | std::ios::binary);    
    isWrite = true;
}

worldstateFileWriter::~worldstateFileWriter()
{
    if(fd.is_open())
        fd.close();
}

void worldstateFileWriter::destory()
{
    if(fd.is_open())
        fd.close();

    if(valid != 1){//Delete the file
        bfs::remove(fileName.c_str());
    } else {
        manager.saveWsInfo(ws);
    }

    delete this;
}

void worldstateFileWriter::writeWsData(std::vector<char>& data, uint64_t len)
{
    open_write();
    fd.seekp(ws.totalSize);
    fd.write(data.data(), len);
    ws.totalSize += len;
    return;
}

bool worldstateFileWriter::isValid()
{
    open_read();
    fc::sha256::encoder enc;
    char buffer[1024];
    fd.seekg(0, std::ios::beg);

    for(uint64_t i = 0; i < ws.totalSize; i += 1024){
        memset(buffer, 0, sizeof(buffer));
        fd.read(buffer, sizeof(buffer));
        
        int cnt = fd.gcount();
        enc.write(buffer, cnt);
    }

    auto result = enc.result();
    std::cout << "isValid ? " << ws.totalSize << "== " << result.str() << std::endl;
    if (result.str() == ws.hashString ) {
        valid = 1;
        return false;
    }
    
    valid = 0;
    return false;
}

void worldstateFileWriter::writeFinished(){

}

void worldstateFileWriter::open_write()
{
    if(isWrite)
        return;

    if(fd.is_open())
        fd.close();

    fd.open(fileName.c_str(), std::ios::out | std::ios::binary);    
    isWrite = true;
}
    
void worldstateFileWriter::open_read()
{
    if(!isWrite)
        return;

    if(fd.is_open())
        fd.close();

    fd.open(fileName.c_str(), std::ios::in | std::ios::binary);    
    isWrite = false;
}

worldstateManager::worldstateManager(std::string dir)
:dirPath(dir)
{
    if (!bfs::is_directory(dirPath)){
        bfs::create_directories(dirPath);
    }
}

worldstateManager::~worldstateManager()
{

}

std::list<wsNode> worldstateManager::getLocalInfo()
{

    try {
        std::string configFile = dirPath + "/ws.conf";
        if(!bfs::exists(configFile) || !bfs::is_regular_file(configFile)){
            return std::list<wsNode>();
        }
        
        auto wsConfFd = std::ifstream(configFile.c_str(), (std::ios::in | std::ios::binary));
        wsConfFd.seekg(0, std::ios::beg);

        u_int32_t cnt = 0;
        fc::raw::unpack(wsConfFd, cnt);

        std::cout << "wsNode Local cnt: " << cnt << std::endl;
        wsNode node;
        std::list<wsNode> retList;
        while(cnt > 0){
            cnt--;
            fc::raw::unpack(wsConfFd, node);
            std::string filePath = dirPath + "/" + node.hashString + ".bin";     
            // std::cout << "wsNode: "<< node.blockHeight << "   "<< node.hashString << "   "<< node.totalSize  << std::endl;
            if(!bfs::exists(filePath) || !bfs::is_regular_file(filePath) || bfs::file_size(filePath) != node.totalSize)
                continue;
                   
            retList.push_back(node);            
        }
        wsConfFd.close();

        return retList;
    } catch( ... ) {  
      std::cout << "ERROR: getLocalWorldStateInfo fail" << std::endl;
      return std::list<wsNode>();
   }
}

void worldstateManager::saveWsInfo(wsNode& node)
{
    std::string confFile = dirPath + "/ws.conf";
    std::fstream wsConfFd = std::fstream(confFile.c_str(), (std::ios::in | std::ios::binary));
    wsConfFd.seekg(0, std::ios::end);

    int fileLen = wsConfFd.tellg(); 
    u_int32_t numberOfws = 0;
    std::vector<char> buffer;
    if(fileLen > 0 && fileLen > sizeof(numberOfws)) {
        std::cout << "saveWsInfo2221" << std::endl;
        buffer.resize(fileLen - sizeof(numberOfws));
        wsConfFd.seekg(0, std::ios::beg);
        fc::raw::unpack(wsConfFd, numberOfws);
        wsConfFd.read(buffer.data(),fileLen - sizeof(numberOfws));
    }
    wsConfFd.close();

    std::cout << "ws conf numberOfws: " << numberOfws << std::endl;

    wsConfFd = std::fstream(confFile.c_str(), (std::ios::out | std::ios::binary));        
    numberOfws++;
    wsConfFd.seekp(0, std::ios::beg);
    fc::raw::pack(wsConfFd, numberOfws);//update count of ws 

    if (buffer.size() > 0)
        wsConfFd.write(buffer.data(), buffer.size());
    fc::raw::pack(wsConfFd, node);   
    wsConfFd.close();
}

worldstateFileReader* worldstateManager::getReader(std::string hash)
{
    auto listNode = getLocalInfo();
    for(auto &it : listNode){
        if(it.hashString == hash){
            std::string filePath = dirPath + "/" + it.hashString + ".bin";
            if(!bfs::exists(filePath) || !bfs::is_regular_file(filePath))
                return nullptr;            
            return new worldstateFileReader(it, dirPath);
        }
    }

    return nullptr;
}

worldstateFileWriter* worldstateManager::getWriter(std::string hash, uint32_t blockHeight)
{
    auto listNode = getLocalInfo();
    for(auto &it : listNode){
        if(it.hashString == hash){
            return nullptr;
        }
    }
    return new worldstateFileWriter(hash, blockHeight, dirPath, *this);
}

}}