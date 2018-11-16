/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 *  @brief launch testnet nodes
 **/
#include <string>
#include <vector>
#include <math.h>
#include <sstream>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/program_options.hpp>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#include <boost/process/child.hpp>
#pragma GCC diagnostic pop
#include <boost/process/system.hpp>
#include <boost/process/io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <fc/crypto/private_key.hpp>
#include <fc/crypto/public_key.hpp>
#include <fc/io/json.hpp>
#include <fc/network/ip.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/log/logger_config.hpp>
#include <ifaddrs.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <net/if.h>
#include <ultrainio/chain/genesis_state.hpp>
#include "wsFileManager.hpp"
#include "client.hpp"
#include "server.hpp"

using namespace std;
namespace bfs = boost::filesystem;
namespace bp = boost::process;
namespace bpo = boost::program_options;
using boost::asio::ip::tcp;
using boost::asio::ip::host_name;
using bpo::options_description;
using bpo::variables_map;
using public_key_type = fc::crypto::public_key;
using private_key_type = fc::crypto::private_key;

void print_vec(const std::vector<char>& vec)
{
    for (auto x: vec) {
         std::cout << ' ' << x;
    }
    std::cout << '\n';
}

// void createWs(std::string content, ultrainio::ws::wsFileManager& wsFileManager)
// {
//       std::vector<char> addData;
//       addData.insert(addData.begin(), content.begin(), content.end());
//       print_vec(addData);

//       fc::sha256::encoder enc;
//       enc.write(addData.data(), addData.size());
//       auto hash = enc.result();

//       auto wsWrite = wsFileManager.getWriter(hash.str(), 123);
//       if(!wsWrite){
//         std::cout << "Failed" << std::endl;
//       }
//       wsWrite->writeWsData(addData, addData.size());

//       wsWrite->isValid();
//       wsWrite->destory();
// }

void createWsFile(ultrainio::ws::wsFileManager& wsFileManager)
{
      std::string filePath =  "./test.log";
      std::ifstream fd;
      fd.open(filePath.c_str(), (std::ios::in | std::ios::binary));
      fd.seekg(0, std::ios::end);
      int totalSize = fd.tellg();
      
      fc::sha256::encoder enc;
      char buffer[1024];
      fd.seekg(0, std::ios::beg);

      for(uint32_t i = 0; i < totalSize; i += 1024){
        memset(buffer, 0, sizeof(buffer));
        fd.read(buffer, sizeof(buffer));

        int cnt = fd.gcount();
        enc.write(buffer, cnt);
      }

      auto hash = enc.result();

      auto wsWrite = wsFileManager.getWriter(hash.str(), 123, totalSize, 2048);
      if(!wsWrite){
        std::cout << "Failed" << std::endl;
      }     

      fd.seekg(0, std::ios::beg);

      std::vector<char> data;
      data.resize(2048);
      for(uint32_t i = 0; i < totalSize; i += 2048){
        fd.read(data.data(), data.size());

        int cnt = fd.gcount();
        wsWrite->writeWsData(i/2048, data, cnt > 0 ? cnt : 0);

      }      

      wsWrite->isValid();
      wsWrite->destory();
}

int main (int argc, char *argv[]) {

  if(argc < 1 || strlen(argv[0]) <= 0){
    cout << "exit" << endl;
  }


  try {
    std::string mode(argv[1]);
    cout << "mode: " << mode << endl;
    auto manager = ultrainio::ws::wsFileManager(".");
    auto wsList = manager.getLocalInfo();
    cout << "ws List size: " << wsList.size() << endl;

    int cnt = 0;
    for(auto& node : wsList){
      std::cout << "wsNode[" << cnt++ <<"]: "<< node.blockHeight << "   "<< node.hashString << "   "<< node.totalSize  << std::endl;
    }

    if (mode == "client"){
      cout << "client, ip: " << argv[2] << endl;
      if(wsList.size() == 0) {
        createWsFile(manager);
        wsList = manager.getLocalInfo();
      }

      boost::asio::io_service io;
      ultrainio::ws::sender(io, argv[2], 7777, wsList.front(), manager);
    } else if(mode == "server"){
        cout << "server" << endl;
        boost::asio::io_service io;
        ultrainio::ws::Tcp_server receiver(io, 7777, manager);  
        io.run();
    }
  } catch (...) {
    throw;
  }

  return 0;
}
