#include <fc/io/raw.hpp>
#include <fc/io/json.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/optional.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/filesystem.hpp>

#include <ultrainio/chain/worldstate.hpp>
#include <ultrainio/chain/chain_worldstate.hpp>
#include <ultrainio/chain/block_header_state.hpp>
#include <ultrainio/chain/block_state.hpp>
#include <ultrainio/chain/authority.hpp>

#include <chainbase/chainbase.hpp>

#include <boost/program_options.hpp>
#include <fstream>

// TODO: find a better way to instantiate chainbase objects
#undef OBJECT_CTOR
#define OBJECT_CTOR(...) public:
#define shared_string vector<char>
#define shared_authority authority
namespace ultrainio { namespace chain { namespace config {
       template<>
              constexpr uint64_t billable_size_v<authority> = 1;
}}}
#include <ultrainio/chain/abi_serializer.hpp>
#include <ultrainio/chain/account_object.hpp>
#include <ultrainio/chain/global_property_object.hpp>
#include <ultrainio/chain/block_summary_object.hpp>
#include <ultrainio/chain/transaction_object.hpp>
#include <ultrainio/chain/generated_transaction_object.hpp>
#include <ultrainio/chain/contract_table_objects.hpp>
#include <ultrainio/chain/authorization_manager.hpp>
#include <ultrainio/chain/permission_link_object.hpp>
#include <ultrainio/chain/resource_limits.hpp>
#include <ultrainio/chain/resource_limits_private.hpp>

using namespace fc;
using namespace std;
using namespace ultrainio::chain;
using namespace chainbase;
using namespace ultrainio::chain::resource_limits;

namespace po = boost::program_options;

int main(int argc, const char **argv) {

    try {
        po::options_description desc("Convert ws 2 json");
        desc.add_options()
            ("help,h", "Print this help message and exit")
            ("in", po::value<string>(), "Pathname of the input binary snapshot")
            ("out", po::value<string>(), "Pathname of the output JSON file")
            ;
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        string in_file;
        if(vm.count("in")) {
            in_file = vm.at("in").as<string>();
        }

        if( vm.count("help") || !in_file.size() ) {
            std::cout << desc << std::endl;
            return 1;
        }

        if(!fc::exists(in_file)) {
            cout << "Input file does not exists: " << in_file << endl;
            return 1;
        }

        std::ifstream ifs(in_file, (std::ios::in | std::ios::binary));
        auto worldstate = std::make_shared<istream_worldstate_reader>(ifs);

        std::streambuf *buf;
        std::ofstream ofs;

        if(vm.count("out")) {
            ofs.open(vm.at("out").as<string>());
            buf = ofs.rdbuf();
        } else {
            buf = std::cout.rdbuf();
        }

        std::ostream out(buf);

        boost::filesystem::path temp = boost::filesystem::unique_path();
        chainbase::database db(temp, database::read_write, 1024);
        std::cerr << temp.native() << " \n";

        map<account_name, abi_def> contract_abi;
        const fc::microseconds max_serialization_time{100000};

        worldstate->read_section<chain_worldstate_header>([&db,&out]( auto &section ){
          chain_worldstate_header header;
          section.read_row(header, db);
        });

        worldstate->read_section<block_state>([&db,&out]( auto &section ){
          block_header_state head_header_state;
          section.read_row(head_header_state, db);
        });

        out << "\naccounts\n";
        //account_object
        worldstate->read_section<account_object>([&db,&contract_abi,&out]( auto& section ) {
                bool more = !section.empty();
                while(more) {
                  account_object row;
                  more = section.read_row(row, db);

                  fc::variant v;
                  fc::to_variant(row, v);
                  fc::mutable_variant_object mvo(v);

                  abi_def abi;
                  if( abi_serializer::to_abi(row.abi, abi) ) {
                  contract_abi[row.name] = abi;
                  fc::variant vabi;
                  fc::to_variant(abi, vabi);
                  mvo["abi"] = vabi;
                  }

                  out << fc::json::to_string(mvo)<<"\n";
                }
        });

        //account_sequence_object
        worldstate->read_section<account_sequence_object>([&db,&contract_abi,&out]( auto& section ) {
                bool more = !section.empty();
                while(more) {
                  account_sequence_object row;
                  more = section.read_row(row, db);
                }
        });

        //global_property_object
        worldstate->read_section<global_property_object>([&db,&contract_abi,&out]( auto& section ) {
                bool more = !section.empty();
                while(more) {
                  global_property_object row;
                  more = section.read_row(row, db);
                }
        });

        //dynamic_global_property_object
        worldstate->read_section<dynamic_global_property_object>([&db,&contract_abi,&out]( auto& section ) {
                bool more = !section.empty();
                while(more) {
                  dynamic_global_property_object row;
                  more = section.read_row(row, db);
                }
        });

        //block_summary_object
        worldstate->read_section<block_summary_object>([&db,&contract_abi,&out]( auto& section ) {
                bool more = !section.empty();
                while(more) {
                  block_summary_object row;
                  more = section.read_row(row, db);
                }
        });

        //transaction_object
        worldstate->read_section<transaction_object>([&db,&contract_abi,&out]( auto& section ) {
                bool more = !section.empty();
                while(more) {
                  transaction_object row;
                  more = section.read_row(row, db);
                }
        });

        //generated_transaction_object
        worldstate->read_section<generated_transaction_object>([&db,&contract_abi,&out]( auto& section ) {
                bool more = !section.empty();
                while(more) {
                  generated_transaction_object row;
                  more = section.read_row(row, db);
                }
        });

        out << "\ntables\n";
        worldstate->read_section("contract_tables", [&db,&contract_abi,&out,&max_serialization_time]( auto& section ) {
                bool more = !section.empty();
                while (more) {
                  table_id_object::id_type t_id;
                  table_id_object table;
                  section.read_row(table, db);
                  t_id = table.id;
                  const auto& abi = contract_abi[table.code];
                  abi_serializer abis(abi,max_serialization_time);
                  string table_type = abis.get_table_type(table.table);
                  vector<char> data;

                  out<<"\ntid:"<<fc::json::to_string(table)<<"\n";
                  out<<"rows:\n";

                  unsigned_int size;

                  more = section.read_row(size, db);
                  for (size_t idx = 0; idx < size.value; idx++) {
                      key_value_object row;
                      row.t_id = t_id;
                      more = section.read_row(row, db);

                      out<< row.primary_key << ":";
                      if(!table_type.size()) {
                          out << "{\"hex_data\":\"" << fc::to_hex(row.value.data(), row.value.size()) << "\n";
                      } else {
                          data.resize( row.value.size() );
                          memcpy(data.data(), row.value.data(), row.value.size());
                          out << "{\"data\":"
                              << fc::json::to_string( abis.binary_to_variant(table_type, data, max_serialization_time))
                              <<"\n";
                      }

                  }

                  more = section.read_row(size, db);
                  for (size_t idx = 0; idx < size.value; idx++) {
                      index64_object row;
                      row.t_id = t_id;
                      more = section.read_row(row, db);
                  }

                  more = section.read_row(size, db);
                  for (size_t idx = 0; idx < size.value; idx++) {
                      index128_object row;
                      row.t_id = t_id;
                      more = section.read_row(row, db);
                  }

                  more = section.read_row(size, db);
                  for (size_t idx = 0; idx < size.value; idx++) {
                      index_double_object row;
                      row.t_id = t_id;
                      more = section.read_row(row, db);
                  }

                  more = section.read_row(size, db);
                  for (size_t idx = 0; idx < size.value; idx++) {
                      index_long_double_object row;
                      row.t_id = t_id;
                      more = section.read_row(row, db);
                  }

                  more = section.read_row(size, db);
                  for (size_t idx = 0; idx < size.value; idx++) {
                      index256_object row;
                      row.t_id = t_id;
                      more = section.read_row(row, db);
                  }
                }
        });

        //permission
        worldstate->read_section<permission_object>([&db,&out]( auto& section ) {
                bool more = !section.empty();
                while(more) {
                     permission_object row;
                     more = section.read_row(row, db);
                }
        });

        worldstate->read_section<permission_link_object>([&db,&out]( auto& section ) {
                bool more = !section.empty();
                while(more) {
                     permission_link_object row;
                     more = section.read_row(row, db);
                }
        });

        //resource
        worldstate->read_section<resource_limits_object>([&db,&out]( auto& section ) {
                bool more = !section.empty();
                while(more) {
                 resource_limits_object row;
                 more = section.read_row(row, db);
                 }
        });

        worldstate->read_section<resource_usage_object>([&db,&out]( auto& section ) {
                bool more = !section.empty();
                while(more) {
                 resource_usage_object row;
                 more = section.read_row(row, db);
                 }
        });

        worldstate->read_section<resource_limits_state_object>([&db,&out]( auto& section ) {
                bool more = !section.empty();
                while(more) {
                 resource_limits_state_object row;
                 more = section.read_row(row, db);
                 }
        });

        worldstate->read_section<resource_limits_config_object>([&db,&out]( auto& section ) {
                bool more = !section.empty();
                while(more) {
                 resource_limits_config_object row;
                 more = section.read_row(row, db);
                 }
        });

        ifs.close();
        ofs.close();
        boost::filesystem::remove_all( temp );
        return 0;
    }
    FC_CAPTURE_AND_LOG(());
        return 1;
}
