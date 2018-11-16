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

namespace ultrainio { namespace ws {
    struct wsNode{
        uint32_t blockHeight; //快照对应的快高
        std::string  hashString;//快照文件的hash 值，和block chain保存的一致
        uint32_t  totalSize;   //快照文件的大小
    };

    class wsFileManager;
    class wsFileReader
    {
        public:
            void destory();
            std::vector<char> getWsData(uint32_t sliceId);
        private:
            wsFileReader(wsNode& node, std::string dir, uint32_t lenPerSlice);
            ~wsFileReader();
        private:
            wsNode ws;
            std::ifstream fd;
            std::string dirPath;
            uint32_t lenPerSlice;
            friend class wsFileManager;
    };

    class wsFileWriter
    {
        public:
            void destory();
            void writeWsData(uint32_t sliceId, std::vector<char>& data, uint32_t dataLen);
            bool isValid();
            void writeFinished();
        
        private:
            wsFileWriter(std::string hashString, uint32_t blockHeight, uint32_t fileSize, uint32_t lenPerSlice, std::string dir, wsFileManager& m);
            ~wsFileWriter();
            void open_write();
            void open_read();
        private:
            wsNode ws;
            std::fstream fd;
            int valid;
            std::string fileName;
            wsFileManager& manager;
            bool isWrite;
            uint32_t writeSize;
            uint32_t lenPerSlice;
            friend class wsFileManager;
    };

    class wsFileManager
    {
        public:
            wsFileManager(std::string dir);
            ~wsFileManager();        
        public:            
            std::list<wsNode> getLocalInfo();
            wsFileReader* getReader(std::string hashString, uint32_t lenPerSlice);
            wsFileWriter* getWriter(std::string hashString, uint32_t blockHeight, uint32_t fileSize, uint32_t lenPerSlice);
            void saveWsInfo(wsNode& node);
        private:
            std::string dirPath;
    };
}}

FC_REFLECT(ultrainio::ws::wsNode, (blockHeight)(hashString)(totalSize))
