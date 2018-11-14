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

namespace ultrainio { namespace worldstate {
    struct wsNode{
        uint32_t blockHeight; //快照对应的快高
        std::string  hashString;//快照文件的hash 值，和block chain保存的一致
        uint64_t  totalSize;   //快照文件的大小
    };

    class worldstateManager;
    class worldstateFileReader
    {
        public:
            void destory();
            std::vector<char> getWsData(uint64_t len = 1024, uint64_t startPos=0);
        private:
            worldstateFileReader(wsNode& node, std::string dir);
            ~worldstateFileReader();
        private:
            wsNode ws;
            std::ifstream fd;
            std::string dirPath;
            friend class worldstateManager;
    };

    class worldstateFileWriter
    {
        public:
            void destory();
            void writeWsData(std::vector<char>& data, uint64_t len);            
            bool isValid();
            void writeFinished();
        
        private:
            worldstateFileWriter(std::string hashString, uint32_t blockHeight, std::string dir, worldstateManager& m);
            ~worldstateFileWriter();
            void open_write();
            void open_read();
        private:
            wsNode ws;
            std::fstream fd;
            int valid;
            std::string fileName;
            worldstateManager& manager;
            bool isWrite;
            friend class worldstateManager;
    };

    class worldstateManager
    {
        public:
            worldstateManager(std::string dir);
            ~worldstateManager();        
        public:            
            std::list<wsNode> getLocalInfo();
            worldstateFileReader* getReader(std::string hashString);
            worldstateFileWriter* getWriter(std::string hashString, uint32_t blockHeight);
            void saveWsInfo(wsNode& node);
        private:
            std::string dirPath;
    };
}}

FC_REFLECT(ultrainio::worldstate::wsNode, (blockHeight)(hashString)(totalSize))
