/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/state_merkle_plugin/state_merkle_plugin.hpp>
#include <queue>
#include <ultrainio/state_history_plugin/state_history_serialization.hpp>
#include <ultrainio/state_merkle_plugin/merkle_file_manager.hpp>

namespace ultrainio {

static appbase::abstract_plugin& _state_merkle_plugin = app().register_plugin<state_merkle_plugin>();

using bytes = std::vector<char>;
struct states {
    int64_t                     revision = 0;
    std::vector<bytes>     modify_values;
    std::vector<bytes>     removed_ids;
    std::vector<bytes>     new_values;
};


struct state_merkle_plugin_impl: std::enable_shared_from_this<state_merkle_plugin_impl> {
    chain_plugin*                             chain_plug = nullptr;
    fc::optional<scoped_connection>           accepted_block_connection;
    std::deque<states>                        kv_state_queue;
    std::thread                               consume_thread;
    fc::optional<merkle_file_manager>         m_manager;
    bool                                      is_ready{false};
    std::atomic_bool                          done{false};
    state_merkle_plugin_impl(){}
    ~state_merkle_plugin_impl(){
        done = true;
        consume_thread.join();
    }
    template<typename Queue, typename Index, typename F> void queue(Queue& queue, const int64_t blk,const Index& index,F& pack_row){
        auto&   db = chain_plug->chain().db();
        queue.emplace_back();
        auto& state = queue.back();
        state.revision = blk;

        if (index.stack().empty())
                return;
        auto& undo = index.stack().back();
        if (undo.old_values.empty() && undo.new_ids.empty() && undo.removed_values.empty())
            return;
        for (auto& old : undo.old_values){
            auto& row = index.get(old.first);
            state.modify_values.push_back(pack_row(row));
        }

        for (auto& old : undo.removed_values){
            state.removed_ids.push_back( pack_row(old.second));//flag false for delete
        }

        for (auto id : undo.new_ids) {
            auto& row = index.get(id);
            state.new_values.push_back(pack_row(row));
        }
    }

    void on_accepted_block(const block_state_ptr& block_state) {
        if(!is_ready){
            if( chain_plug->chain().is_replaying() )
                return;
            else if(block_state->block_num % merkle_interval != 1)
                return;
            is_ready=true;
            m_manager.emplace(chain_plug->get_chain_id().str(),block_state->block_num,app().data_dir() / config::default_wsroot_dir_name);
        }
        auto&           db = chain_plug->chain().db();
        std::map<uint64_t, const table_id_object*> removed_table_id;
        const auto&     table_id_index = db.get_index<table_id_multi_index>();
        for (auto& rem : table_id_index.stack().back().removed_values)
            removed_table_id[rem.first._id] = &rem.second;

        auto get_table_id = [&](uint64_t tid) -> const table_id_object& {
           auto obj = table_id_index.find(tid);
            if (obj)
                return *obj;
            auto it = removed_table_id.find(tid);
            ULTRAIN_ASSERT(it != removed_table_id.end(), chain::plugin_exception, "can not found table id ${tid}",
                    ("tid", tid));
             return *it->second;
        };

        auto pack_contract_row = [&](auto& row) {
            return fc::raw::pack(make_history_context_wrapper(db, get_table_id(row.t_id._id), row));
        };
        queue(kv_state_queue, block_state->block_num, db.get_index<key_value_index>(),pack_contract_row);
    }

    void merkle_computer(states& state){
        digest_type::encoder enc;
        incremental_merkle mroot;
        for(auto& item:state.modify_values)
            enc.reset(),fc::raw::pack( enc, item),mroot.append(enc.result());
        for(auto& item:state.removed_ids)
            enc.reset(),fc::raw::pack( enc, item),mroot.append(enc.result());
        for(auto& item:state.new_values)
            enc.reset(),fc::raw::pack( enc, item),mroot.append(enc.result());

        m_manager->write_data(state.revision,mroot);
    }
    void init(){
        consume_thread =std::thread([this]{consume_item();});
    }

    void consume_item(){
        while(true){
            while(kv_state_queue.size() < 2){
                if (done){
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds{ultrainio::chain::config::block_interval_ms});//sleep 1 block
            }
            merkle_computer(kv_state_queue.front());
            kv_state_queue.pop_front();
        }
    }
};

state_merkle_plugin::state_merkle_plugin()
    : my(std::make_shared<state_merkle_plugin_impl>()) {}

state_merkle_plugin::~state_merkle_plugin() {}

void state_merkle_plugin::set_program_options(options_description& cli, options_description& cfg) {}

void state_merkle_plugin::plugin_initialize(const variables_map& options) {
    try{
        my->chain_plug = app().find_plugin<chain_plugin>();
        ULTRAIN_ASSERT(my->chain_plug, chain::missing_chain_plugin_exception, "");
        auto& chain = my->chain_plug->chain();
        my->accepted_block_connection.emplace(
                chain.accepted_block.connect([&](const block_state_ptr& p) { my->on_accepted_block(p); }));
        my->init();
    }FC_LOG_AND_RETHROW()
}

void state_merkle_plugin::plugin_startup() {}

void state_merkle_plugin::plugin_shutdown() {
    my->accepted_block_connection.reset();
}
} // namespace ultrainio
