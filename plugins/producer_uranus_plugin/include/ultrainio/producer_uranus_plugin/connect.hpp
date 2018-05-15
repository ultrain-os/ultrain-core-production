#ifndef CONNECT_H_
#define CONNECT_H_

#include <sys/time.h>

#include <iostream>
#include <string>

#include <boost/asio.hpp>

namespace ultrain {

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

#if 0
    class TimeoutAdjust
    {
    public:
      TimeoutAdjust()
      {
        m_dwTimeout.tv_sec = 3;
        m_dwTimeout.tv_usec = 3;
      };

      template<class Protocol>
      int level(const Protocol& p) const {return SOL_SOCKET;}

      template<class Protocol>
      int name(const Protocol& p) const {return SO_RCVTIMEO;}

      template<class Protocol>
      const void* data(const Protocol& p) const {return &m_dwTimeout;}

      template<class Protocol>
      size_t size(const Protocol& p) const {return sizeof(struct timeval);}
    //private:
      struct timeval m_dwTimeout;
    };
#endif

    class TimeoutAdjust {
    public:
        TimeoutAdjust() {
            tv.tv_sec = 1;
            tv.tv_usec = 0;
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
#endif

