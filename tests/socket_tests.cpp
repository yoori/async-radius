#define BOOST_TEST_MODULE radius_lite_dictionaries_tests

#include <array>
#include <vector>
#include <set>
#include <string>
#include <cstdint> //uint8_t, uint32_t
#include <stdexcept>
#include <boost/asio.hpp>

#include "attribute_types.h"
#include "utils.h"
#include <radius_lite/socket.h>
#include <radius_lite/error.h>
#include <radius_lite/attribute.h>
#include <radius_lite/vendor_attribute.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <boost/test/unit_test.hpp>
#pragma GCC diagnostic pop

using boost::system::error_code;

namespace
{
  bool callbackReceiveCalled = false;
  bool callbackSendCalled = false;

  void checkSend(const error_code& ec)
  {
    callbackSendCalled = true;
    BOOST_REQUIRE(!ec);
  }

  void checkReceive(const error_code& ec, const std::optional<radius_lite::Packet>& p, boost::asio::ip::udp::endpoint /*source*/)
  {
    callbackReceiveCalled = true;
    BOOST_REQUIRE(!ec);

    BOOST_REQUIRE(p != std::nullopt);

    radius_lite::Packet d = p.value();
    BOOST_CHECK_EQUAL(d.type(), 1);

    BOOST_CHECK_EQUAL(d.id(), 208);

    std::array<uint8_t, 16> auth {0x1a, 0x40, 0x43, 0xc6, 0x41, 0x0a, 0x08, 0x31, 0x12, 0x16, 0x80, 0x2c, 0x3e, 0x83, 0x12, 0x45};

    BOOST_TEST(d.auth() == auth, boost::test_tools::per_element());

    const auto& attrs = d.attributes();

    auto* attr0 = findAttribute(attrs, radius_lite::USER_NAME);

    BOOST_REQUIRE(attr0 != nullptr);
    BOOST_CHECK_EQUAL(attr0->type(), radius_lite::USER_NAME);
    BOOST_CHECK_EQUAL(attr0->toString(), "test");

    auto* attr1 = findAttribute(attrs, radius_lite::USER_PASSWORD);

    BOOST_REQUIRE(attr1 != nullptr);
    BOOST_CHECK_EQUAL(attr1->type(), radius_lite::USER_PASSWORD);
    BOOST_CHECK_EQUAL(attr1->toString(), "123456");

    auto* attr2 = findAttribute(attrs, radius_lite::NAS_IP_ADDRESS);

    BOOST_REQUIRE(attr2 != nullptr);
    BOOST_CHECK_EQUAL(attr2->type(), radius_lite::NAS_IP_ADDRESS);
    BOOST_CHECK_EQUAL(attr2->toString(), "127.0.0.1");

    auto* attr3 = findAttribute(attrs, radius_lite::NAS_PORT);

    BOOST_REQUIRE(attr3 != nullptr);
    BOOST_CHECK_EQUAL(attr3->type(), radius_lite::NAS_PORT);
    BOOST_CHECK_EQUAL(attr3->toString(), "1");

    auto* attr4 = findAttribute(attrs, radius_lite::MESSAGE_AUTHENTICATOR);

    BOOST_REQUIRE(attr4 != nullptr);
    BOOST_CHECK_EQUAL(attr4->type(), radius_lite::MESSAGE_AUTHENTICATOR);
    BOOST_CHECK_EQUAL(attr4->toString(), "F3E000E77DEB51EB815D52373D06B71B");

    auto* attr5 = findAttribute(attrs, radius_lite::FRAMED_PROTOCOL);

    BOOST_REQUIRE(attr5 != nullptr);
    BOOST_CHECK_EQUAL(attr5->type(), radius_lite::FRAMED_PROTOCOL);
    BOOST_CHECK_EQUAL(attr5->toString(), "1");

    std::vector<radius_lite::VendorSpecific> vendor = d.vendorSpecific();

    BOOST_REQUIRE_EQUAL(vendor.size(), 1);
    BOOST_CHECK_EQUAL(vendor[0].vendorId(), 171);
    BOOST_CHECK_EQUAL(vendor[0].vendorType(), 1);
    BOOST_CHECK_EQUAL(vendor[0].toString(), "00000003");

    std::set<uint8_t> types {
      radius_lite::USER_NAME,
      radius_lite::USER_PASSWORD,
      radius_lite::NAS_IP_ADDRESS,
      radius_lite::NAS_PORT,
      radius_lite::MESSAGE_AUTHENTICATOR,
      radius_lite::FRAMED_PROTOCOL};

    BOOST_REQUIRE_EQUAL(attrs.size(), 6);
    BOOST_CHECK(types.count(attrs[0]->type()) == 1);
    BOOST_CHECK(types.count(attrs[1]->type()) == 1);
    BOOST_CHECK(types.count(attrs[2]->type()) == 1);
    BOOST_CHECK(types.count(attrs[3]->type()) == 1);
    BOOST_CHECK(types.count(attrs[4]->type()) == 1);
    BOOST_CHECK(types.count(attrs[5]->type()) == 1);
  }
}

BOOST_AUTO_TEST_SUITE(socket_tests)

BOOST_AUTO_TEST_CASE(TestConstructor)
{
  boost::asio::io_service io_service;
  BOOST_CHECK_NO_THROW(radius_lite::Socket s(
    io_service,
    "secret",
    3000,
    [this](const auto&, const auto&, const boost::asio::ip::udp::endpoint&){}));
}

BOOST_AUTO_TEST_CASE(TestAsyncSend)
{
  std::array<uint8_t, 16> auth {
    0x1a, 0x40, 0x43, 0xc6, 0x41, 0x0a, 0x08, 0x31, 0x12, 0x16, 0x80, 0x2c, 0x3e, 0x83, 0x12, 0x45};

  const std::vector<radius_lite::Attribute*> attributes {
    new radius_lite::String(1, "test"),
    new radius_lite::Encrypted(2, "123456"),
    new radius_lite::IpAddress(4, {127, 0, 0, 1}),
    new radius_lite::Integer<uint32_t>(5, 1),
    new radius_lite::Bytes(80, {
      0xf3, 0xe0, 0x00, 0xe7, 0x7d, 0xeb, 0x51, 0xeb, 0x81, 0x5d, 0x52, 0x37, 0x3d, 0x06, 0xb7, 0x1b}),
    new radius_lite::Integer<uint32_t>(7, 1)};

  std::vector<radius_lite::VendorSpecific> vendorSpecific;
  vendorSpecific.emplace_back(radius_lite::VendorSpecific(171, 1, {0, 0, 0, 3}));

  radius_lite::Packet p(1, 208, auth, attributes, vendorSpecific);

  boost::asio::io_service io_service;

  boost::asio::ip::udp::endpoint destination(boost::asio::ip::address_v4::from_string("127.0.0.1"), 3000);

  radius_lite::Socket s(io_service, "secret", 3000, checkReceive);

  s.asyncSend(p, destination, checkSend);

  io_service.run();

  BOOST_CHECK_MESSAGE(callbackSendCalled, "Function asyncSend hasn't called checkSend.");
  BOOST_CHECK_MESSAGE(callbackReceiveCalled, "Function asyncReceive hasn't called checkReceive.");
}

BOOST_AUTO_TEST_SUITE_END()
