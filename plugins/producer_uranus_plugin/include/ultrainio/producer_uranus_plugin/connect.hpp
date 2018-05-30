#pragma once

#include <sys/time.h>

#include <iostream>
#include <string>

#include <boost/asio.hpp>

namespace ultrainio {

    class Connect : public std::enable_shared_from_this<Connect> {
    public:
        static std::string s_local_host;

        Connect(const boost::asio::io_service &service, unsigned short usPortNum = 13066)
                : usPort(usPortNum), i_socket(const_cast<boost::asio::io_service &>(service),
                                              boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), usPortNum)) {
            s_local_host = get_local_host();
        };

        ~Connect() { i_socket.close(); };

        // Sends broadcast
        void sendBrdCast(const char *buf, size_t size);

        void sendUniCast(const char *buf, size_t size, const std::string &host);

        //receive message
        std::size_t rcvMsg(char *buf, std::size_t size);

        std::string get_local_host();

        //Determine whether the i_socket is open.
        bool isOpen();

    private:
        boost::asio::ip::udp::socket i_socket;
        unsigned short usPort;

    };

    class TimeoutAdjust {
    public:
        TimeoutAdjust() {
            tv.tv_sec = 0;
            tv.tv_usec = 200;
        };

        template<class Protocol>
        int level(const Protocol &p) const { return SOL_SOCKET; }

        template<class Protocol>
        int name(const Protocol &p) const { return SO_RCVTIMEO; }

        template<class Protocol>
        const void *data(const Protocol &p) const { return &tv; }

        template<class Protocol>
        size_t size(const Protocol &p) const { return sizeof(timeval); }

    private:
        struct timeval tv;
        unsigned int m_dwTimeout;
    };

}
