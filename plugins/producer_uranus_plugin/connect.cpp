#include <ultrainio/producer_uranus_plugin/connect.hpp>

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <time.h>

#include <ultrainio/producer_uranus_plugin/define.hpp>
#include <ultrainio/producer_uranus_plugin/log.hpp>

namespace ultrainio {

    //using namespace boost::asio;
    //using namespace std;

    std::string Connect::s_local_host("");

    bool Connect::isOpen() {
        return i_socket.is_open();
    }

    void Connect::sendBrdCast(const char *buf, size_t size) {
        //TimeoutAdjust adjust;

        //i_socket.set_option(boost::asio::ip::multicast::enable_loopback(false));

        i_socket.set_option(boost::asio::socket_base::broadcast(true));
        //i_socket.set_option(adjust);

        boost::asio::ip::udp::endpoint broadcast_endpoint(boost::asio::ip::address_v4::broadcast(), usPort);

        //ip::udp::endpoint broadcast_endpoint(ip::address_v4::from_string("192.1.1.2"), usPort);
        i_socket.send_to(boost::asio::buffer(buf, size), broadcast_endpoint);
    }

    void Connect::sendUniCast(const char *buf, size_t size, const std::string &host) {
        //TimeoutAdjust adjust;
        i_socket.set_option(boost::asio::socket_base::broadcast(false));
        //i_socket.set_option(adjust);
        boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::address_v4::from_string(host), usPort);
        i_socket.send_to(boost::asio::buffer(buf, size), endpoint);
    }

    std::string Connect::get_local_host() {
        return boost::asio::ip::host_name();
    }

    std::size_t Connect::rcvMsg(char *buf, std::size_t size) {
        boost::asio::ip::udp::endpoint sender_endpoint;
        char ucBuffer[BUFSIZE] = {0};
        std::size_t dataLenth = 0;
        // Receive data.
        TimeoutAdjust adjust;
        struct sockaddr_in clientaddr;

        i_socket.set_option(boost::asio::socket_base::broadcast(true));
        i_socket.set_option(adjust);

        try {
            socklen_t clientlen = sizeof(clientaddr);
            //dataLenth = i_socket.receive_from(boost::asio::buffer(ucBuffer,BUFSIZE), sender_endpoint);
            dataLenth = recvfrom(i_socket.native_handle(), ucBuffer, BUFSIZE, 0, (struct sockaddr *) &clientaddr,
                                 &clientlen);

            //LOG_INFO<<"peer_ip"<<sender_endpoint.address().to_string().c_str()<<endl;
            //LOG_INFO<<"remote ip"<<i_socket.remote_endpoint().address().to_string().c_str()<<endl;
            //if (i_socket.remote_endpoint().address().to_string() == i_socket.local_endpoint().address().to_string())
            //    LOG_INFO<<"same ip"<<endl;
        } catch (const boost::system::system_error &e) {
            if (e.code() != boost::asio::error::timed_out)
                throw;
            return 0;
        }

        if ((BUFSIZE <= dataLenth) || (dataLenth == 0) || (dataLenth > size)) {
            return 0;
        }
        std::memcpy(buf, ucBuffer, dataLenth);
        return dataLenth;
    }

}  // namespace ultrainio

