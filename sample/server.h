#pragma once

#include "socket.h"
#include "packet.h"
#include "dictionaries.h"
#include <boost/asio.hpp>
#include <optional>
#include <cstdint> //uint8_t, uint32_t

class Server
{
public:
  Server(
    boost::asio::io_service& io_service,
    const std::string& secret,
    uint16_t port,
    const std::string& filePath);

private:
  radius_lite::Packet make_response(const radius_lite::Packet& request);

  void handle_receive(
    const boost::system::error_code& error,
    const std::optional<radius_lite::Packet>& packet,
    const boost::asio::ip::udp::endpoint& source);

  void handle_send(const boost::system::error_code& ec);

  //void startReceive();

private:
  radius_lite::Socket m_radius;
  radius_lite::Dictionaries m_dictionaries;
  std::string secret_;
};
