#include <ultrainio.sched/ultrainio.sched.hpp>

namespace ultrainio {
    scheduler::scheduler( account_name self ) : contract(self),
        _nodes(_self, _self), _subchains(_self,_self),
        _miners_que(_self,_self), _pending_subchain(_self,_self) {}

    /// @abi action
    void scheduler::regminer(account_name miner_account_name,
                                   const std::string& miner_pk,
                                   uint32_t miner_deposit,
                                   const std::string& miner_ip) {
        require_auth(miner_account_name);
        ultrainio_assert(miner_deposit >= deposit_threshold, "The deposit is less than threshold.");

        //todo, check the fomat of ip string.
        //todo, check whether this miner have enough asset for mortgaging.
        //check if this account has registered
        auto itor = _nodes.find(miner_account_name);
        ultrainio_assert(itor == _nodes.end(), "this account is runing a node now.");
        auto itor2 = _miners_que.find(miner_account_name);
        ultrainio_assert(itor2 == _miners_que.end(), "this account is in the waiting queue.");
        /*
        bool has_a_pending_subchain = _pending_subchain.exists();
        if (has_a_pending_subchain) {
            auto _pending = _pending_subchain.get();  //todo, maybe chack if it's valid
            for(const auto& tmp_node : _pending.committee_members) {
                if(tmp_node.my_account == miner_account_name) {
                    ultrainio_assert(false, "this account is in the waiting queue of the pending subchain.");
               }
            }
        }
        //todo, can this account also run a node in main chain?
        //todo, transfer miner's asset to freeze the deposit

        //check if a subchain could be started after this miner joins.

        if(has_a_pending_subchain) {
            auto _pending = _pending_subchain.get();
            _pending.total_deposit += miner_deposit;
            node_base temp_node;
            temp_node.my_account = miner_account_name;
            temp_node.my_deposit = miner_deposit;
            temp_node.my_pk      = miner_pk; 
            temp_node.node_ip    = miner_ip;
            _pending.committee_members.push_back(temp_node);
            _pending_subchain.set(_pending, miner_account_name); //todo, check who should pay for this. maybe _pending.root_name.
            if(_pending.total_deposit >= _pending.min_active_stake && _pending.committee_members.size() > _pending.min_committee_member_num) {
                start_new_chain();
                return;
            }
        } */

        //The new node doesn't meet the threshold of the pending subchain, add it into node queue
        _miners_que.emplace(miner_account_name, [&]( auto& node ) {
            node.my_account = miner_account_name;
            node.my_pk      = miner_pk;
            node.my_deposit = miner_deposit;
            node.node_ip    = miner_ip;
        });
    }

    /// @abi action
    void scheduler::regsubchain(uint64_t chain_name,
                                       account_name root_user_name,
                                       const std::string& root_user_pk,
                                       uint32_t sub_chain_deposit,
                                       uint32_t node_num) {
        require_auth(root_user_name);
        ultrainio_assert(!_pending_subchain.exists(), "There's already a pending subchain, please try later.");
        auto itor = _subchains.find(root_user_name);
        ultrainio_assert(itor == _subchains.end(), "This account has hosted a subchian.");

        //todo, if there are enough nodes in the waiting queue, start the new chain immediately.

        //else, store it in pending chain.
        subchain_base new_subchain;
        new_subchain.root_name                 = root_user_name;
        new_subchain.root_pk                   = root_user_pk;
        new_subchain.min_active_stake          = sub_chain_deposit;
        new_subchain.min_committee_member_num  = node_num;
        new_subchain.total_deposit             = 0;
        new_subchain.head_block_id             = block_id_type();
        new_subchain.head_block_num            = 0;
        _pending_subchain.set(new_subchain, root_user_name);
    }

    /// @abi action
    void scheduler::acceptheader (uint64_t chain_name,
                                  const block_header& header) {
        require_auth(current_sender());
        //todo, check chain_name is a valid name of subchain
        account_name block_proposer = header.proposer;
        //todo, check in node table temporarily, will update to check subchain's committee list.
        auto itor = _miners_que.find(block_proposer);
        ultrainio_assert(itor != _miners_que.end(), "block proposer is not in committee list of this subchian.");
        //compare previous with pre block's id.

        //use the _pending_subchain temporarily
        ultrainio_assert(_pending_subchain.exists(), "please start a subchain first");
        auto _pending = _pending_subchain.get();
        auto block_number = header.block_num();
        if(_pending.head_block_num == 0 ||
           (_pending.head_block_id == header.previous && block_number == _pending.head_block_num + 1)) {
              _pending.head_block_id = header.id(); 
              _pending.head_block_num = block_number;
              _pending_subchain.set(_pending, block_proposer);
        }
        else if (_pending.head_block_id == header.id() && block_number == _pending.head_block_num) {
            //this block has been submited by other relayer
            return;
        }
        else if (block_number == _pending.head_block_num) {
            //todo, fork chain block, need to check malicious hehavior togther with signatures.
        }
        else if (block_number > _pending.head_block_num) {
            ultrainio_assert(false, "block number is greater than current block.");
        }
        else if (block_number < _pending.head_block_num) {
            ultrainio_assert(false, "block number is less than current block.");
        }
        else {
            ultrainio_assert(false, "block header verification failed.");
        }
    }

    void scheduler::join_subchain(const std::string& miner_pk,
                                  account_name miner_account_name,
                                  uint32_t miner_deposit,
                                  const std::string& ip,
                                  uint64_t chain_name) {
        require_auth(miner_account_name);
        ultrainio_assert(miner_deposit >= deposit_threshold, "The deposit doesn't reach the threshold.");
        //todo, check the fomat of ip string.
        //todo, check whether this miner have enough asset for mortgaging.
        //check if this account has registered
        auto itor = _nodes.find(miner_account_name);
        ultrainio_assert(itor == _nodes.end(), "this account is runing a node now.");
        auto itor2 = _miners_que.find(miner_account_name);
        ultrainio_assert(itor2 == _miners_que.end(), "this account is in the waiting queue.");
        bool has_a_pending_subchain = _pending_subchain.exists();
        if (has_a_pending_subchain) {
            auto _pending = _pending_subchain.get();
            for(const auto& tmp_node : _pending.committee_members) {
                if(tmp_node.my_account == miner_account_name) {
                    ultrainio_assert(false, "this account is in the waiting queue of the pending subchain.");
               }
            }
        }
        //todo, can this account also run a node in main chain?
        //check the deposit
        auto itor_subchain = _subchains.find(chain_name);
        //todo, transfer miner's asset to freeze the deposit
        //todo, add new miner into the specified subchain.
        //add into nodes table
          //_nodes.emplace
    }

    void scheduler::register_relayer(const std::string& miner_pk,
                                     account_name relayer_account_name,
                                     uint32_t relayer_deposit,
                                     const std::string& ip,
                                     uint64_t chain_name) {}

    void scheduler::start_new_chain() {
        ultrainio_assert(_pending_subchain.exists(), "start_new_chain: pending subchain is not existed.");
        //move pending subchain into subchains table
        auto _pending = _pending_subchain.get();
        _subchains.emplace(_pending.root_name, [&]( auto& new_subchain ) {
            new_subchain.root_name                 = _pending.root_name;
            new_subchain.root_pk                   = _pending.root_pk;
            new_subchain.min_active_stake          = _pending.min_active_stake;
            new_subchain.min_committee_member_num  = _pending.min_committee_member_num;
            new_subchain.total_deposit             = _pending.total_deposit;
            new_subchain.head_block_id             = _pending.head_block_id;
            new_subchain.head_block_num            = _pending.head_block_num;
//1            new_subchain.chain_hash                = ; //new generated or set?
            new_subchain.genesis_info              = "";
            new_subchain.relayer_stake_threshold   = 0;
        });

        //add all committee members into nodes table
        for (const auto& _miner : _pending.committee_members) {
            _nodes.emplace(_miner.my_account, [&]( auto& active_miner ) {
                active_miner.my_account        = _miner.my_account;
                active_miner.my_pk             = _miner.my_pk;
                active_miner.my_deposit        = _miner.my_deposit;
                active_miner.node_ip           = _miner.node_ip;
                active_miner.chain_name        = _pending.root_name;
                active_miner.dest_chain_name   = _pending.root_name; //same with chain_name.
                active_miner.quit_before_block = 0;
            });
        }
        //clear the _pending
        _pending_subchain.remove();
        //todo, select relayers.
    }
} //namespace ultrainio

ULTRAINIO_ABI( ultrainio::scheduler, (regminer)(regsubchain)(acceptheader) )
