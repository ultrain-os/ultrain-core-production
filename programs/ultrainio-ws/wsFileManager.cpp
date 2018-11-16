/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include "wsFileManager.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>
#include <iostream>
#include <fc/io/raw.hpp>
#include <sstream>
#include <fc/crypto/sha256.hpp>

namespace bfs = boost::filesystem;

namespace ultrainio { namespace ws {

wsFileReader::wsFileReader(wsNode& node, std::string dir, uint32_t lenPerSlice)
:ws(node)
,dirPath(dir)
,lenPerSlice(lenPerSlice)
{
    std::string filePath = dirPath + "/" + node.hashString + ".bin";
    fd.open(filePath.c_str(), (std::ios::in | std::ios::binary));
}

wsFileReader::~wsFileReader()
{
    if(fd.is_open())
        fd.close();
}

void wsFileReader::destory()
{
    delete this;
}

std::vector<char>  wsFileReader::getWsData(uint32_t sliceId)
{
    if(sliceId*lenPerSlice >= ws.totalSize)
        return std::vector<char>();

    fd.seekg(sliceId*lenPerSlice, std::ios::beg);
    std::vector<char>  retData;
    retData.resize(lenPerSlice);
    fd.read(retData.data(), lenPerSlice);
    retData.resize(fd.gcount());
    return retData;
}

wsFileWriter::wsFileWriter(std::string hash, uint32_t blockHeight, uint32_t fileSize, uint32_t lenPerSlice, std::string dir, wsFileManager& m)
:valid(-1)
,fileName()
,manager(m),
isWrite(false)
,writeSize(0)
,lenPerSlice(lenPerSlice)
{
    ws.blockHeight = blockHeight;
    ws.hashString = hash;
    ws.totalSize = fileSize;
    fileName = dir + "/" + hash + ".bin";

    fd.open(fileName.c_str(), std::ios::out | std::ios::binary);    
    isWrite = true;
}

wsFileWriter::~wsFileWriter()
{
    if(fd.is_open())
        fd.close();
}

void wsFileWriter::destory()
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

void wsFileWriter::writeWsData(uint32_t sliceId, std::vector<char>& data, uint32_t dataLen)
{
    if(dataLen % lenPerSlice){//Error, write data was not whole slice
        std::cerr << "ERROR!!!!!!!Need assert!!!!!" << std::endl;
        return;
    }

    open_write();
    fd.seekp(0, std::ios::end);
    int count = fd.tellp();

    if (count > 0) {
        char placeholder[1024] = {0};
        while(count < sliceId*lenPerSlice){
            int cnt = sliceId*lenPerSlice - count > sizeof(placeholder) ? sizeof(placeholder) : sliceId*lenPerSlice - count;
            fd.write(placeholder, cnt);
            count += cnt;
        }
    }

    fd.seekp(sliceId*lenPerSlice, std::ios::beg);
    fd.write(data.data(), dataLen);
    writeSize += dataLen;
    return;
}

bool wsFileWriter::isValid()
{
    if(writeSize != ws.totalSize)
        return false;

    open_read();
    fc::sha256::encoder enc;
    char buffer[1024];
    fd.seekg(0, std::ios::beg);

    for(uint32_t i = 0; i < writeSize; i += 1024){
        memset(buffer, 0, sizeof(buffer));
        fd.read(buffer, sizeof(buffer));

        int cnt = fd.gcount();
        enc.write(buffer, cnt);
    }

    auto result = enc.result();
    std::cout << "isValid ? : "<< ws.hashString == result.str() << " "<< ws.hashString << "== " << result.str() << std::endl;
    if (result.str() == ws.hashString ) {
        valid = 1;
        return true;
    }
    
    valid = 0;
    return false;
}

void wsFileWriter::writeFinished(){

}

void wsFileWriter::open_write()
{
    if(isWrite)
        return;

    if(fd.is_open())
        fd.close();

    fd.open(fileName.c_str(), std::ios::out | std::ios::binary);    
    isWrite = true;
}
    
void wsFileWriter::open_read()
{
    if(!isWrite)
        return;

    if(fd.is_open())
        fd.close();

    fd.open(fileName.c_str(), std::ios::in | std::ios::binary);    
    isWrite = false;
}

wsFileManager::wsFileManager(std::string dir)
:dirPath(dir)
{
    if (!bfs::is_directory(dirPath)){
        bfs::create_directories(dirPath);
    }
}

wsFileManager::~wsFileManager()
{

}

std::list<wsNode> wsFileManager::getLocalInfo()
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

void wsFileManager::saveWsInfo(wsNode& node)
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

wsFileReader* wsFileManager::getReader(std::string hash, uint32_t lenPerSlice)
{
    auto listNode = getLocalInfo();
    for(auto &it : listNode){
        if(it.hashString == hash){
            std::string filePath = dirPath + "/" + it.hashString + ".bin";
            if(!bfs::exists(filePath) || !bfs::is_regular_file(filePath))
                return nullptr;            
            return new wsFileReader(it, dirPath, lenPerSlice);
        }
    }

    return nullptr;
}

wsFileWriter* wsFileManager::getWriter(std::string hash, uint32_t blockHeight, uint32_t fileSize, uint32_t lenPerSlice)
{
    auto listNode = getLocalInfo();
    for(auto &it : listNode){
        if(it.hashString == hash){
            return nullptr;
        }
    }
    return new wsFileWriter(hash, blockHeight, fileSize, lenPerSlice, dirPath,  *this);
}

}}