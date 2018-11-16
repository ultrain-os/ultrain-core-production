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
#include <iostream>
#include <cstdio>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "wsFileManager.hpp"
#include "protocal.hpp"

namespace ultrainio { namespace ws {
    //namespace asio {
      using namespace boost::asio;
      using  namespace std;
      using boost::system::error_code;
    //}
class Session : public boost::enable_shared_from_this<Session> {
public:
  typedef ip::tcp TCP;
  typedef error_code Error; 
  typedef boost::shared_ptr<Session> Pointer;
//   typedef File_info::Size_type Size_type;
  
  static void print_asio_error(const Error& error) { std::cerr << error.message() << "\n";}
  
  static Pointer create(io_service& io, wsFileManager& m) { return Pointer(new Session(io, m));}
  
  TCP::socket& socket() { return socket_; }
  
  ~Session() 
  {
    clock_ = clock() - clock_;
    if (clock_ == 0) clock_ = 1;

    writePtr->isValid();
    writePtr->destory();

    double speed = node.totalSize * (CLOCKS_PER_SEC / 1024.0 / 1024.0) / clock_ ;
    std::cout << "cost time: " << clock_ / (double) CLOCKS_PER_SEC << " s  " 
       << "bytes_writen: " << node.totalSize << " bytes\n"
       << "speed: " <<  speed << " MB/s\n\n"; 
  }
  
  void start()
  {
    clock_ = clock();
    std::cout << "client: " << socket_.remote_endpoint().address() << "\n";
    receive_header_content();
  }
  
private:
  Session(io_service& io, wsFileManager& m) : socket_(io), manager(m), writePtr(nullptr) { }

  void receive_header_content(){
        int header_len = fc::raw::pack_size(header);
        headerData.resize(header_len);
        // socket_.async_receive(buffer(headerData, header_len), 
        //     boost::bind(&Session::handle_header, shared_from_this(), boost::asio::placeholders::error))
        
        async_read(socket_, buffer(headerData, header_len),
            boost::bind(&Session::handle_header, shared_from_this(), boost::asio::placeholders::error));  
  }

  
  void handle_header(const Error& error) 
  {
    if (error) return print_asio_error(error);

    fc::datastream<char*> ds(headerData.data(), headerData.size());
    fc::raw::unpack(ds, header);

    cout << "header: " << header.type  << " size: " << header.size << " sliceId:" << header.sliceId << endl; 
    if(header.type == 0){//wsNode info
        receiveData.resize(header.size);
        async_read(socket_, buffer(receiveData, header.size),
            boost::bind(&Session::handle_wsNode, shared_from_this(), boost::asio::placeholders::error)); 
    } else {
        receiveData.resize(header.size);
        async_read(socket_, buffer(receiveData, header.size),
            boost::bind(&Session::handle_file, shared_from_this(), boost::asio::placeholders::error)); 
    }
  }
  
  void handle_wsNode(const Error& error)
  {
    if (error) return print_asio_error(error);

    fc::datastream<char*> ds(receiveData.data(), receiveData.size());
    fc::raw::unpack(ds, node);
    cout << "node: " << node.blockHeight << "   "<< node.hashString << "   "<< node.totalSize  << endl;

    if(writePtr)
        std::cerr << "writePtr not null"<< endl;

    writePtr = manager.getWriter(node.hashString, node.blockHeight, node.totalSize, lenOfSlice);

    if(!writePtr){
        std::cerr << "writePtr is null"<< endl;
        return;
    }
    receive_header_content();
  }
  

  void handle_file(const Error& error)
  {
    if (error) return print_asio_error(error);
    if(!writePtr){
        std::cerr << "尚未接收到文件头"<< endl;
        return;
    }
    
    writePtr->writeWsData(header.sliceId, receiveData, receiveData.size());
    receive_header_content();
  }
  
private:  
    clock_t clock_;
    TCP::socket socket_;
    wsFileManager& manager;
    wsNode node;
    message_info header;
    std::vector<char> headerData;
    std::vector<char> receiveData;
    wsFileWriter* writePtr;
};

class Tcp_server
{
public:
  typedef ip::tcp TCP;
  typedef error_code Error;
  
  Tcp_server(io_service& io, unsigned port, wsFileManager& m) : 
      acceptor_(io, TCP::endpoint(TCP::v4(), port))
      ,manager(m)
  {
    start_accept();
  }

  static void print_asio_error(const Error& error) { std::cerr << error.message() << "\n";}

private:
  void start_accept()
  {
    Session::Pointer session = Session::create(acceptor_.get_io_service(), manager);
    acceptor_.async_accept(session->socket(),
      boost::bind(&Tcp_server::handle_accept, this, session, boost::asio::placeholders::error));
  }
  
  void handle_accept(Session::Pointer session, const Error& error)
  {
    if (error) return print_asio_error(error);
    session->start();
    start_accept();
  }
  
  TCP::acceptor acceptor_;
  wsFileManager& manager;
};
}}