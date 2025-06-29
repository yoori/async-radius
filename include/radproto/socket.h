#pragma once

#include "packet.h"
#include <boost/asio.hpp>
#include <cstdint> //uint8_t, uint32_t
#include <array>
#include <functional>
#include <optional>

namespace RadProto
{
  class Socket
  {
  public:
    using PacketProcessFun = std::function<void(
      const boost::system::error_code&,
      const std::optional<Packet>&,
      const boost::asio::ip::udp::endpoint&)>;

  public:
    Socket(
      boost::asio::io_service& io_service,
      const std::string& secret,
      uint16_t port,
      const PacketProcessFun& callback);

    void asyncSend(
      const Packet& response,
      const boost::asio::ip::udp::endpoint& destination,
      const std::function<void(const boost::system::error_code&)>& callback);

    void close(boost::system::error_code& ec);

  private:
    void start_receive_loop_(const PacketProcessFun& callback);

    void handle_receive_(
      const boost::system::error_code& error,
      std::size_t bytes,
      const PacketProcessFun& callback);

    void handle_send_(
      const boost::system::error_code& ec,
      const std::function<void(const boost::system::error_code&)>& callback);

    void order_receive_(const PacketProcessFun& callback);

  private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
    std::array<uint8_t, 4096> recv_buffer_;
    std::string secret_;
  };
}
