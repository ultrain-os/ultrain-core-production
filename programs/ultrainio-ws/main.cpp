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

void createWs(std::string content, ultrainio::ws::wsManager& wsManager)
{
      std::vector<char> addData;
      addData.insert(addData.begin(), content.begin(), content.end());
      print_vec(addData);

      fc::sha256::encoder enc;
      enc.write(addData.data(), addData.size());
      auto hash = enc.result();

      auto wsWrite = wsManager.getWriter(hash.str(), 123);
      if(!wsWrite){
        std::cout << "Failed" << std::endl;
      }
      wsWrite->writeWsData(addData, addData.size());

      wsWrite->isValid();
      wsWrite->destory();
}

int main (int argc, char *argv[]) {

  try {
    cout << "test" << endl;

    auto wsManager = ultrainio::ws::wsManager(".");
    auto wsList = wsManager.getLocalInfo();
    cout << "ws List size: " << wsList.size() << endl;

    int cnt = 0;
    for(auto& node : wsList){
      std::cout << "wsNode[" << cnt++ <<"]: "<< node.blockHeight << "   "<< node.hashString << "   "<< node.totalSize  << std::endl;
    }

    std::string str = "TestAdd" + std::to_string(wsList.size());
    createWs(str, wsManager);
  } catch (...) {
    throw;
  }
  return 0;
}
