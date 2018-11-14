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
        uint64_t  totalSize;   //快照文件的大小
    };

    class wsManager;
    class wsFileReader
    {
        public:
            void destory();
            std::vector<char> getWsData(uint64_t len = 1024, uint64_t startPos=0);
        private:
            wsFileReader(wsNode& node, std::string dir);
            ~wsFileReader();
        private:
            wsNode ws;
            std::ifstream fd;
            std::string dirPath;
            friend class wsManager;
    };

    class wsFileWriter
    {
        public:
            void destory();
            void writeWsData(std::vector<char>& data, uint64_t len);            
            bool isValid();
            void writeFinished();
        
        private:
            wsFileWriter(std::string hashString, uint32_t blockHeight, std::string dir, wsManager& m);
            ~wsFileWriter();
            void open_write();
            void open_read();
        private:
            wsNode ws;
            std::fstream fd;
            int valid;
            std::string fileName;
            wsManager& manager;
            bool isWrite;
            friend class wsManager;
    };

    class wsManager
    {
        public:
            wsManager(std::string dir);
            ~wsManager();        
        public:            
            std::list<wsNode> getLocalInfo();
            wsFileReader* getReader(std::string hashString);
            wsFileWriter* getWriter(std::string hashString, uint32_t blockHeight);
            void saveWsInfo(wsNode& node);
        private:
            std::string dirPath;
    };
}}

FC_REFLECT(ultrainio::ws::wsNode, (blockHeight)(hashString)(totalSize))
