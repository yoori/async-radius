#ifndef SERVER_H
#define SERVER_H

#include "packet.h"
#include "dictionaries.h"
#include <boost/asio.hpp>
#include <cstdint> //uint8_t, uint32_t
#include <array>
#include <functional>

class Server
{
    public:
        Server(boost::asio::io_service& io_service, const std::string& secret);
        void asyncReceive(const std::function<void(const boost::system::error_code&, const Packet&)>& callback);
        void asyncSend(const std::function<void(const boost::system::error_code&)>& callback, Packet& response);

    private:
        void handleReceive(const boost::system::error_code& error, std::size_t bytes, std::function<void(const boost::system::error_code&, const Packet&)> callback);
        void handleSend(const boost::system::error_code& ec, std::function<void(const boost::system::error_code&)> callback);

        boost::asio::ip::udp::socket m_socket;
        boost::asio::ip::udp::endpoint m_remoteEndpoint;
        std::array<uint8_t, 4096> m_recvBuffer;
        std::string m_secret;
        Dictionaries m_dictionaries;
};

#endif
