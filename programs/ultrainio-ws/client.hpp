// client_sender//www.cnblogs.com/flyinghearts
#pragma once
#include <iostream>
#include <cstdio>
#include <cstring>
#include <boost/shared_ptr.hpp>
#include <fc/exception/exception.hpp>
#include <list>
#include <string>
#include <fc/reflect/reflect.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>
#include <iostream>
#include <fc/io/raw.hpp>
#include <sstream>
#include <fc/crypto/sha256.hpp>
#include <boost/asio.hpp>
#include <fc/io/datastream.hpp>
#include "wsFileManager.hpp"
#include "protocal.hpp"
#include <unistd.h>

namespace ultrainio { namespace ws {

void sender(boost::asio::io_service& io, const char* ip_address, unsigned port, wsNode& node, wsFileManager& manager)
{
  typedef boost::asio::ip::tcp TCP;
  TCP::socket socket(io);
  socket.connect(TCP::endpoint(boost::asio::ip::address_v4::from_string(ip_address), port));

    auto reader = manager.getReader(node.hashString, lenOfSlice);

    clock_t cost_time = clock();

    message_info info={0};
    int msg_len = fc::raw::pack_size(info);
    int data_size = fc::raw::pack_size(node);
    int send_size = msg_len + data_size;

    info.type = 0;
    info.size = data_size;
    info.sliceId = -1;

    std::vector<char> send_buffer;
    send_buffer.resize(send_size);
    fc::datastream<char*> ds(send_buffer.data(), send_size);
    fc::raw::pack(ds, info);
    fc::raw::pack(ds, node);
    socket.send(boost::asio::buffer(send_buffer.data(), send_size), 0);

    int cnt = 0;    
    while (true) {
        auto data = reader->getWsData(cnt);
        if(data.size() <= 0 ){
            std::cout << "sender data.size() <= 0:  " << cnt <<std::endl;
            if(cnt % 2 == 0){
                cnt = 1;
                continue;
            } else {
                break;
            }            
        }

        info.type = 1;
        info.size = data.size();
        info.sliceId = cnt;

        send_buffer.resize(msg_len);
        fc::datastream<char*> ds(send_buffer.data(), msg_len);
        fc::raw::pack(ds, info);

        send_buffer.insert(send_buffer.end(), data.begin(), data.end());

        int ret = socket.send(boost::asio::buffer(send_buffer.data(), send_buffer.size()), 0);
        if(cnt % 100 == 0 || cnt %100 == 1) std::cout << "send: " << cnt<< "th: size  "<< ret <<std::endl;

        cnt += 2;
    }
  
  reader->destory();
  cost_time = clock() - cost_time;
  if (cost_time == 0) cost_time = 1;
  double speed = node.totalSize * (CLOCKS_PER_SEC / 1024.0 / 1024.0) / cost_time;
  std::cout << "cost time: " << cost_time / (double) CLOCKS_PER_SEC  << " s " 
    << "  transferred_bytes: " << node.totalSize << " bytes\n"
    << "speed: " <<  speed << " MB/s\n\n"; 
}
}}