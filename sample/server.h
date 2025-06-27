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
  RadProto::Packet make_response(const RadProto::Packet& request);

  void handle_receive(
    const boost::system::error_code& error,
    const std::optional<RadProto::Packet>& packet,
    const boost::asio::ip::udp::endpoint& source);

  void handle_send(const boost::system::error_code& ec);

  //void startReceive();

private:
  RadProto::Socket m_radius;
  RadProto::Dictionaries m_dictionaries;
  std::string secret_;
};
