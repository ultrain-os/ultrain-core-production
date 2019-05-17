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
#define shared_vector vector
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

// If using custom worldstate object, need re-define the template here
namespace ultrainio { namespace chain {
    namespace detail {
        template<>
        struct worldstate_row_traits<permission_object> {
            using value_type = permission_object;
            using worldstate_type = worldstate_permission_object;
        };
    }
}}

#define LAMBDA_PARAME (auto& row,auto& mvo,auto& section,auto& more)
#define READ_SECTION(objtype) read_section<objtype>(out,[]LAMBDA_PARAME{});
#define READ_SECTION_LAMBDA(objtype,lbd) read_section<objtype>(out,lbd);

namespace po = boost::program_options;

typedef decltype(std::make_shared<istream_worldstate_reader>(std::ifstream{}) ) WS;
const fc::microseconds max_serialization_time{100000};
map<account_name, abi_def> contract_abi;
WS worldstate;
boost::filesystem::path temp = boost::filesystem::unique_path();
chainbase::database db(temp, database::read_write, 1024);
std::ofstream table_info_out;
std::ofstream  account_info_out;

struct contract_table_info {
    table_name     table;
    uint64_t        len = 0;
    uint64_t        cnt = 0;
};

struct account_info {
    account_name name;
    uint64_t     recv_sequence = 0;
    uint64_t     auth_sequence = 0;
    uint64_t     code_sequence = 0;
    uint64_t     abi_sequence  = 0;
    uint64_t     ram_usage = 0;
    int64_t     ram_quota = 0;
    std::map<table_name, contract_table_info>  contract_info_map;
    uint64_t cnt_of_contact_table = 0;
    uint64_t cnt_of_contact_record = 0;
    uint64_t len_of_contact_in_ws = 0;
};

struct table_info {
    std::string table_name_text;
    uint64_t size_of_ws_file = 0;
    uint64_t cnt_of_record = 0;
};

FC_REFLECT(account_info, (name)(recv_sequence)(auth_sequence)(code_sequence)(abi_sequence)(ram_usage)(ram_quota)\
            (cnt_of_contact_table)(cnt_of_contact_record)(len_of_contact_in_ws))
FC_REFLECT(table_info, (table_name_text)(size_of_ws_file)(cnt_of_record))

std::map<account_name, account_info> account_info_map;
std::vector<table_info> table_info_vector;
bool is_enable_json = false;

//process section with lambda , contract table etc.
template<typename ObjectType,typename ProcessTable>
void read_section(std::ostream& out,ProcessTable&& pt) {
    auto it = [&](auto &section){
            string type_name= boost::core::demangle(typeid(ObjectType).name());
            string name = type_name.substr(type_name.find_last_of(":",type_name.size()-1)+1,type_name.size()-1);
            out << name << "\n";
            auto start_pos = worldstate->current_pos();
            int total_record = 0;
            bool more = !section.empty();
            while(more) {
              total_record++;
              auto row = typename ultrainio::chain::detail::worldstate_row_traits<ObjectType>::worldstate_type();
              more = section.read_row(row, db);

              fc::variant v;
              fc::to_variant(row, v);
              fc::mutable_variant_object mvo(v);

              pt(row,mvo,section,more);

                if (!std::is_same<ObjectType, table_id_object>::value) {
                    out << fc::json::to_string(mvo)<<"\n";
                }
            }

            table_info t_info;
            t_info.table_name_text = name;
            t_info.size_of_ws_file = worldstate->current_pos() - start_pos;
            t_info.cnt_of_record = total_record;
            if (table_info_out.is_open() && !std::is_same<ObjectType, block_state>::value && !std::is_same<ObjectType, chain_worldstate_header>::value) {
                table_info_vector.push_back(t_info);
            }
        };
        if (std::is_same<ObjectType, table_id_object>::value)
            worldstate->read_section("contract_tables",it);
        else if (std::is_same<ObjectType, block_header_state>::value)
            worldstate->read_section<block_state>(it);
        else
            worldstate->read_section<ObjectType>(it);
}

void output_info()
{
    if (!table_info_out.is_open())
        return;

    //print account info
    if (!is_enable_json){
        account_info_out << std::left << std::setw(13) << "name";
        account_info_out << std::setw(15) << "ram_usage"<< " ";
        account_info_out << std::setw(15) << "ram_limit"<< " ";
        account_info_out << std::setw(15) << "recv_sequence"<< " ";
        account_info_out << std::setw(15) << "abi_sequence"<< " ";
        account_info_out << std::setw(15) << "auth_sequence"<< " ";
        account_info_out << std::setw(15) << "code_sequence"<< " ";
        account_info_out << std::setw(15) << "cnt_of_contact_table"<< " ";
        account_info_out << std::setw(15) << "cnt_of_contact_record"<< " ";
        account_info_out << std::setw(15) << "len_of_contact_in_ws"<< " ";
        account_info_out << "\n";
    }

    std::vector<account_info> acc_v(account_info_map.size());
    for(auto& it : account_info_map){
        int len = 0;
        int cnt = 0;
        for (auto& t : it.second.contract_info_map){
            len += t.second.len;
            cnt += t.second.cnt;
        }
        it.second.cnt_of_contact_table = it.second.contract_info_map.size();
        it.second.cnt_of_contact_record = cnt;
        it.second.len_of_contact_in_ws = len;

        if (!is_enable_json) {
            account_info_out << std::left << std::setw(13) << it.second.name << " ";
            account_info_out << std::setw(15) << it.second.ram_usage << " ";
            if(it.second.ram_quota == -1)
                account_info_out << std::setw(15) << "unlimited" << " ";
            else
                account_info_out << std::setw(15) << it.second.ram_quota << " ";
            account_info_out << std::setw(15) << it.second.recv_sequence << " ";
            account_info_out << std::setw(15) << it.second.abi_sequence << " ";
            account_info_out << std::setw(15) << it.second.auth_sequence << " ";
            account_info_out << std::setw(15) << it.second.code_sequence << " ";
            account_info_out << std::setw(15) << it.second.cnt_of_contact_table << " ";
            account_info_out << std::setw(15) << it.second.cnt_of_contact_record << " ";
            account_info_out << std::setw(15) << it.second.len_of_contact_in_ws << " ";
            account_info_out << "\n";
        } else {
            acc_v.push_back(it.second);
        }
    }

    if (is_enable_json) {
        fc::variant v;
        fc::to_variant(table_info_vector, v);
        account_info_out << fc::json::to_string(acc_v);
    }

    //print  table_info
    if (!is_enable_json) {
        table_info_out << std::left << std::setw(50) << "Native_table_name ";
        table_info_out <<std::setw(20)<< "size_of_ws_file" << " " << std::setw(20) << "cnt_of_record" << "\n";
        for(auto& it : table_info_vector) {
            table_info_out << std::left << std::setw(50) << it.table_name_text << " ";
            table_info_out <<std::setw(20)<< it.size_of_ws_file << std::setw(20) << it.cnt_of_record << "\n";
        }
    } else {
        fc::variant v;
        fc::to_variant(table_info_vector, v);
        table_info_out << fc::json::to_string(v);
    }
}

void decode_ws(std::ostream& out){
    READ_SECTION(chain_worldstate_header)

    READ_SECTION(block_header_state)

    auto acc_obj=[&]LAMBDA_PARAME{
        abi_def abi;
        if( abi_serializer::to_abi(row.abi, abi) ) {
            contract_abi[row.name] = abi;
            fc::variant vabi;
            fc::to_variant(abi, vabi);
            mvo["abi"] = vabi;
        }
        ULTRAIN_ASSERT(account_info_map.find(row.name) == account_info_map.end(), worldstate_exception, "Account already exist: ${a}", ("a", row.name));

        account_info info;
        info.name = row.name;
        account_info_map[row.name] = info;
    };
    READ_SECTION_LAMBDA(account_object, acc_obj)

    auto tb = [&]LAMBDA_PARAME{
        table_id_object::id_type t_id = row.id;
        const auto& abi = contract_abi[row.code];
        abi_serializer abis(abi,max_serialization_time);
        string table_type = abis.get_table_type(row.table);
        vector<char> data;
        out<<"\ntid:"<<fc::json::to_string(row)<<"\n";
        out<<"rows:\n";

        ULTRAIN_ASSERT(account_info_map.find(row.code) != account_info_map.end(), worldstate_exception, "Account don't exist: ${a}", ("a", row.code));
        auto& info = account_info_map[row.code];
        auto start_pos = worldstate->current_pos();
        int total_record = 0;

        unsigned_int size;
        more = section.read_row(size, db);
        for (size_t idx = 0; idx < size.value; idx++) {
            key_value_object row;
            row.t_id = t_id;
            more = section.read_row(row, db);

            total_record++;

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
            out<< row.primary_key << ":";
            fc::variant v;
            fc::to_variant(row, v);
            fc::mutable_variant_object mvo(v);
            out << "index64 "<<fc::json::to_string(mvo)<<"\n";
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
            fc::variant v;
            fc::to_variant(row, v);
            fc::mutable_variant_object mvo(v);
            out << fc::json::to_string(mvo)<<"\n";
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
            fc::variant v;
            fc::to_variant(row, v);
            fc::mutable_variant_object mvo(v);
            out << "index256 "<<fc::json::to_string(mvo)<<"\n";
        }

        auto data_len = worldstate->current_pos() - start_pos;
        if (info.contract_info_map.find(row.table) != info.contract_info_map.end()){
            auto& table_info = info.contract_info_map.find(row.table)->second;
            table_info.len += data_len;
            table_info.cnt += total_record;
        } else {
            contract_table_info table_info;
            table_info.table = row.table;
            table_info.len = data_len;
            table_info.cnt = total_record;
            info.contract_info_map[row.table] = table_info;
        }
    };
    READ_SECTION_LAMBDA(table_id_object,tb)
    READ_SECTION_LAMBDA(account_sequence_object, [&]LAMBDA_PARAME{
        ULTRAIN_ASSERT(account_info_map.find(row.name) != account_info_map.end(), worldstate_exception, "Account don't exist: ${a}", ("a", row.name));
        auto& info = account_info_map[row.name];
        info.recv_sequence = row.recv_sequence;
        info.auth_sequence = row.auth_sequence;
        info.code_sequence = row.code_sequence;
        info.abi_sequence = row.abi_sequence;
    });
    READ_SECTION(global_property_object)
    READ_SECTION(dynamic_global_property_object)
    READ_SECTION(block_summary_object)
    READ_SECTION(transaction_object)
    READ_SECTION(generated_transaction_object)
    READ_SECTION(permission_object)
    READ_SECTION(permission_link_object)
    READ_SECTION_LAMBDA(resource_limits_object, [&]LAMBDA_PARAME{
        ULTRAIN_ASSERT(account_info_map.find(row.owner) != account_info_map.end(), worldstate_exception, "Account don't exist: ${a}", ("a", row.owner));
        auto& info = account_info_map[row.owner];
        if (info.ram_quota <= 0 || row.pending == true)
            info.ram_quota = row.ram_bytes;
    });
    READ_SECTION_LAMBDA(resource_usage_object, [&]LAMBDA_PARAME{
        ULTRAIN_ASSERT(account_info_map.find(row.owner) != account_info_map.end(), worldstate_exception, "Account don't exist: ${a}", ("a", row.owner));
        auto& info = account_info_map[row.owner];
        info.ram_usage = row.ram_usage;
    });
    READ_SECTION(resource_limits_state_object)
    READ_SECTION(resource_limits_config_object)

    output_info();
}

int main(int argc, const char **argv) {
    try {
        po::options_description desc("Convert ws 2 json");
        desc.add_options()
            ("help,h", "Print this help message and exit")
            ("in,i", po::value<string>(), "Pathname of the input binary snapshot")
            ("out,o", po::value<string>(), "Pathname of the output JSON file")
            ("table,t","out put contact table and account info")
            ("json,j","out put contact table and account info as json format")
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
        worldstate = std::make_shared<istream_worldstate_reader>(ifs);

        std::streambuf *buf;
        std::ofstream ofs;
        string out_path;
        if(vm.count("out")) {
            out_path=vm.at("out").as<string>();
            if(!fc::is_directory(out_path))
                fc::create_directories(out_path);
            ofs.open(out_path+"/ws.json");
            buf = ofs.rdbuf();
        } else {
            buf = std::cout.rdbuf();
        }

        std::ostream out(buf);

        if(vm.count("table")) {
            account_info_out = std::ofstream(out_path+"/account_info.txt", ios::out);
            table_info_out = std::ofstream(out_path+"/table_info.txt", ios::out);
            if(vm.count("json"))
                is_enable_json = true;
        }

        cout << temp.native() << " \n";

        decode_ws(out);
        ifs.close();
        ofs.close();
        table_info_out.close();
        account_info_out.close();
        boost::filesystem::remove_all( temp );
        return 0;
    }
    FC_CAPTURE_AND_LOG(());
    return 1;
}
