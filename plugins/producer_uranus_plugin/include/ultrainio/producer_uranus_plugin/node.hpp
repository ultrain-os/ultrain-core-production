#ifndef ULTRAINNODE_H_
#define ULTRAINNODE_H_

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <boost/chrono/include.hpp>
#include "define.hpp"
#include "connect.hpp"
#include "pktmanage.hpp"
#include "security.hpp"

extern uint8_t uranus_public_key[32];
extern uint8_t uranus_private_key[64];

namespace ultrain {

    class UranusNode : public std::enable_shared_from_this<UranusNode> {
    public:

        explicit UranusNode(const boost::asio::io_service &ioservice) : _phase(PHASE_INIT), _role(NONE),
                                                                        connection(ioservice) {};

        bool isTimeout();

        void startup();

        processresult Listen();

        void reset();

        bool sendMsg(MsgInfo &stMsgInfo);

        bool insertMsg(MsgInfo &stMsgInfo);

    private:
        uint16_t _phase;
        uranusState _role;
        boost::chrono::steady_clock::time_point start;
        Connect connection;
        PktManager pktmng;
    };
}
#endif



