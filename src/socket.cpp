#include <iostream>
#include "socket.h"
#include "error.h"
#include "packet_codes.h"

using boost::asio::ip::udp;
using boost::system::error_code;

namespace pls = std::placeholders;

std::string packetTypeToString(int type)
{
    switch (type)
    {
        case RadProto::ACCESS_REQUEST: return "ACCESS_REQUEST";
        case RadProto::ACCESS_ACCEPT: return "ACCESS_ACCEPT";
        case RadProto::ACCESS_REJECT: return "ACCESS_REJECT";
        case RadProto::ACCOUNTING_REQUEST: return "ACCOUNTING_REQUEST";
        case RadProto::ACCOUNTING_RESPONSE: return "ACCOUNTING_RESPONSE";
        case RadProto::ACCESS_CHALLENGE: return "ACCESS_CHALLENGE";
        case RadProto::STATUS_SERVER: return "STATUS_SERVER";
        case RadProto::STATUS_CLIENT: return "STATUS_CLIENT";
    }
    return "uncnown";
}

namespace RadProto
{
  Socket::Socket(
    boost::asio::io_service& io_service,
    const std::string& secret,
    uint16_t port,
    const std::function<void(const boost::system::error_code&, const std::optional<Packet>&, const udp::endpoint&)>& callback)
    : io_service_(io_service),
      socket_(io_service, udp::endpoint(udp::v4(), port)),
      secret_(secret)
  {
    std::cout << "Socket: port = " << port << std::endl;
    start_receive_loop_(callback);
  }

  void Socket::asyncSend(
    const Packet& response,
    const udp::endpoint& destination,
    const std::function<void(const boost::system::error_code&)>& callback)
  {
    const std::vector<uint8_t> vResponse = response.makeSendBuffer(secret_);

    io_service_.post(
      [this, destination, callback, vResponse = std::move(vResponse)]
      {
        socket_.async_send_to(
          boost::asio::buffer(vResponse),
          destination,
          [this, callback](const error_code& ec, std::size_t /*bytesTransferred*/)
          {
            handle_send_(ec, callback);
          }
        );
      }
    );
  }

  void Socket::start_receive_loop_(const PacketProcessFun& callback)
  {
    std::cout << "Socket: start_receive_loop_" << std::endl;
    io_service_.post(
      [this, callback]
      {
        order_receive_(callback);
      }
    );
  }

  void
  Socket::order_receive_(const PacketProcessFun& callback)
  {
    std::cout << "Socket: order_receive_" << std::endl;
    socket_.async_receive_from(
      boost::asio::buffer(recv_buffer_),
      remote_endpoint_,
      [this, callback](const error_code& error, std::size_t bytes)
      {
        handle_receive_(error, bytes, callback);
        order_receive_(callback);
      });
  }

  void Socket::handle_receive_(
    const error_code& error,
    std::size_t bytes,
    const std::function<void(const error_code&, const std::optional<Packet>&, const udp::endpoint&)>& callback)
  {
    if (error)
    {
      callback(error, std::nullopt, remote_endpoint_);
    }

    if (bytes < 20)
    {
      callback(Error::numberOfBytesIsLessThan20, std::nullopt, remote_endpoint_);
    }

    try
    {
      callback(
        error,
        std::make_optional<Packet>(recv_buffer_.data(), bytes, secret_),
        remote_endpoint_);
    }
    catch (const Exception& exception)
    {
      std::cerr << "exception: " << exception.what() << std::endl;
      callback(exception.getErrorCode(), std::nullopt, remote_endpoint_);
    }
  }

  void Socket::handle_send_(const error_code& ec, const std::function<void(const error_code&)>& callback)
  {
    callback(ec);
  }

  void Socket::close(error_code& ec)
  {
    socket_.shutdown(udp::socket::shutdown_both, ec);
    socket_.close(ec);
  }
}
