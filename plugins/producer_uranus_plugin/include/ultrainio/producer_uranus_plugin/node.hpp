#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <boost/chrono/include.hpp>

#include "define.hpp"
#include "connect.hpp"
#include "pktmanage.hpp"
#include "security.hpp"

namespace ultrainio {

    class UranusNode : public std::enable_shared_from_this<UranusNode> {
    public:
        static const uint64_t MAX_HASH_LEN_VALUE;
        static boost::chrono::system_clock::time_point GENESIS;
        static uint8_t URANUS_PUBLIC_KEY[VRF_PUBLIC_KEY_LEN];
        static uint8_t URANUS_PRIVATE_KEY[VRF_PRIVATE_KEY_LEN];

        static bool verifyRole(uint32_t block_id, uint16_t phase, const std::string& role_proof, const std::string& pk);

        static uint64_t proof_to_priority(const uint8_t role_proof[VRF_PROOF_LEN]);

        explicit UranusNode(const boost::asio::io_service &ioservice) : _phase(PHASE_INIT)
                , _block_id(0), _role_proof(), _role(NONE), connection(ioservice), pktmng(this) {
        };

        uint32_t get_block_id() const;

        const uint8_t* get_role_proof() const;

        uranus_role get_role() const;

        bool isTimeout();

        bool ready_to_join();

        bool startup();

        void run();

        uranus_role generate_own_role(uint16_t phase);

        processresult Listen();

        void reset();

        bool sendMsg(MsgInfo &stMsgInfo);

        bool insertMsg(MsgInfo &stMsgInfo);

    private:
        static const boost::chrono::milliseconds MS_PER_PHASE;
        static const boost::chrono::seconds SECONDS_PER_ROUND;
        consensus_phase _phase;
        uint32_t _block_id;
        uint8_t _role_proof[VRF_PROOF_LEN];
        uranus_role _role;
        boost::chrono::steady_clock::time_point _start;
        Connect connection;
        PktManager pktmng;
    };
}
