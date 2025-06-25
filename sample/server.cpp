#include <functional>
#include <iostream>

#include "server.h"
#include "packet_codes.h"
#include "packet_reader.h"

using boost::system::error_code;

Server::Server(boost::asio::io_service& io_service, const std::string& secret, uint16_t port, const std::string& filePath)
    : m_radius(io_service, secret, port),
      m_dictionaries(filePath),
      secret_(secret)
{
  m_dictionaries.resolve(); // TODO: make this in Dictionaries c-tor, but use other class for included dictionaries
  std::cout << "To start receive" << std::endl;
  startReceive();
}

void Server::startReceive()
{
  m_radius.asyncReceive(
    [this](const auto& error, const auto& packet, const boost::asio::ip::udp::endpoint& source)
    {
      handleReceive(error, packet, source);
    }
  );
}

std::string ipv4_address_to_string(uint32_t ipv4)                                                                                                        
{                                                                                                                                                        
  return std::to_string(ipv4 & 0xFF) + "." +                                                                                                             
    std::to_string((ipv4 >> 8) & 0xFF) + "." +                                                                                                           
    std::to_string((ipv4 >> 16) & 0xFF) + "." +                                                                                                          
    std::to_string((ipv4 >> 24) & 0xFF)                                                                                                                  
    ;                                                                                                                                                    
}

std::string
print_uint_attr(
  RadProto::PacketReader& packet_reader,
  const std::string& attr_name,
  const std::string& vendor_name = std::string())
{
  auto attr = packet_reader.get_attribute_by_name(attr_name, vendor_name);
  if (!attr)
  {
    return attr_name + ": no attribute in dictionary";
  }

  auto attr_s = attr->as_uint();
  return attr_name + ": " + (attr_s.has_value() ? std::to_string(*attr_s) : "none");
}

std::string byteToHex(uint8_t byte)
{
  static const std::string digits = "0123456789ABCDEF";
  return {digits[byte / 16], digits[byte % 16]};
}

std::string
print_ipv4_attr(
  RadProto::PacketReader& packet_reader,
  const std::string& attr_name,
  const std::string& vendor_name = std::string())
{
  auto attr = packet_reader.get_attribute_by_name(attr_name, vendor_name);
  if (!attr)
  {
    return attr_name + ": no attribute in dictionary";
  }

  auto attr_s = attr->as_uint();
  return attr_name + ": " + (attr_s.has_value() ?
    ipv4_address_to_string(*attr_s) + "(" + std::to_string(*attr_s) + ")" :
    "none");
}

std::string
print_string_attr(
  RadProto::PacketReader& packet_reader,
  const std::string& attr_name,
  const std::string& vendor_name = std::string())
{
  auto attr = packet_reader.get_attribute_by_name(attr_name, vendor_name);
  if (!attr)
  {
    return attr_name + ": no attribute in dictionary";
  }

  auto attr_s = attr->as_string();
  return attr_name + ": " + (attr_s.has_value() ? *attr_s : "none");
}

std::string
print_octets_attr(
  RadProto::PacketReader& packet_reader,
  const std::string& attr_name,
  const std::string& vendor_name = std::string())
{
  auto attr = packet_reader.get_attribute_by_name(attr_name, vendor_name);
  if (!attr)
  {
    return attr_name + ": no attribute in dictionary";
  }

  auto attr_s = attr->as_octets();
  std::string res = attr_name + ":";

  for (const auto& b : attr_s)
  {
    res += " " + byteToHex(b);
  }

  return res;
}

RadProto::Packet Server::makeResponse(const RadProto::Packet& request)
{
  RadProto::PacketReader packet_reader(request, m_dictionaries, secret_);
  std::cout << print_string_attr(packet_reader, "Calling-Station-Id") << std::endl;
  std::cout << print_string_attr(packet_reader, "Called-Station-Id") << std::endl;
  std::cout << print_ipv4_attr(packet_reader, "Framed-IP-Address") << std::endl;
  std::cout << print_ipv4_attr(packet_reader, "NAS-IP-Address") << std::endl;
  std::cout << print_uint_attr(packet_reader, "Acct-Status-Type") << std::endl;
  std::cout << print_string_attr(packet_reader, "IMSI", "3GPP") << std::endl;
  std::cout << print_string_attr(packet_reader, "IMEISV", "3GPP") << std::endl;
  std::cout << print_uint_attr(packet_reader, "RAT-Type", "3GPP") << std::endl;
  std::cout << print_string_attr(packet_reader, "SGSN-MCC-MNC", "3GPP") << std::endl;

  // MS-TimeZone struct => TZ (2 bytes)
  {
    auto attr = packet_reader.get_attribute_by_name("MS-TimeZone", "3GPP");
    if (attr)
    {
      auto attr_s = attr->as_octets();
      if (attr_s.size() > 0)
      {
        std::cout << "MS-TimeZone: " << static_cast<unsigned int>(attr_s[0]) << std::endl;
      }
    }
  }
  //std::cout << print_octets_attr(packet_reader, "MS-TimeZone", "3GPP") << std::endl;
  std::cout << print_ipv4_attr(packet_reader, "SGSN-Address", "3GPP") << std::endl;
  std::cout << print_ipv4_attr(packet_reader, "CG-Address", "3GPP") << std::endl;
  std::cout << print_uint_attr(packet_reader, "Charging-ID", "3GPP") << std::endl;
  std::cout << print_string_attr(packet_reader, "GPRS-Negotiated-QoS-profile", "3GPP") << std::endl;
  std::cout << print_octets_attr(packet_reader, "User-Location-Info", "3GPP") << std::endl;
  std::cout << print_string_attr(packet_reader, "NSAPI", "3GPP") << std::endl;
  std::cout << print_string_attr(packet_reader, "Selection-Mode", "3GPP") << std::endl;
  std::cout << print_string_attr(packet_reader, "Charging-Characteristics", "3GPP") << std::endl;

  /*
  for (const auto& vendor_v : request.vendorSpecific())
  {
    auto vendor_name = m_dictionaries.vendorNames().name(vendor_v.vendorId());
    auto vendor_attr_name = m_dictionaries.vendorAttributes().name(vendor_name, vendor_v.vendorType());

    std::cout << "vendorSpecific: vendorId = " << vendor_v.vendorId() <<
      ", vendorType = " << static_cast<int>(vendor_v.vendorType()) <<
      ", vendor_name = " << vendor_name <<
      ", vendor_attr_name = " << vendor_attr_name << std::endl;
  }
  */

  std::vector<RadProto::Attribute*> attributes;
  attributes.push_back(new RadProto::String(m_dictionaries.attributeCode("User-Name"), "test"));
  attributes.push_back(new RadProto::Integer<uint32_t>(m_dictionaries.attributeCode("NAS-Port"), 20));
  std::array<uint8_t, 4> address {127, 104, 22, 17};
  attributes.push_back(new RadProto::IpAddress(m_dictionaries.attributeCode("NAS-IP-Address"), address));
  std::vector<uint8_t> bytes {'1', '2', '3', 'a', 'b', 'c'};
  attributes.push_back(new RadProto::Bytes(m_dictionaries.attributeCode("Callback-Number"), bytes));
  std::vector<uint8_t> chapPassword {'1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
  attributes.push_back(new RadProto::ChapPassword(m_dictionaries.attributeCode("CHAP-Password"), 1, chapPassword));

  std::vector<RadProto::VendorSpecific> vendorSpecific;
  /*
  std::vector<uint8_t> vendorValue {0, 0, 0, 3};
  vendorSpecific.push_back(RadProto::VendorSpecific(m_dictionaries.vendorCode("Dlink"), m_dictionaries.vendorAttributeCode("Dlink", "Dlink-User-Level"), vendorValue));
  */

  if (request.type() == RadProto::ACCESS_REQUEST)
  {
    return RadProto::Packet(RadProto::ACCESS_ACCEPT, request.id(), request.auth(), attributes, vendorSpecific);
  }

  return RadProto::Packet(RadProto::ACCESS_REJECT, request.id(), request.auth(), attributes, vendorSpecific);
}

void Server::handleSend(const error_code& ec)
{
    if (ec)
        std::cout << "Error asyncSend: " << ec.message() << "\n";

    startReceive();
}

void Server::handleReceive(const error_code& error, const std::optional<RadProto::Packet>& packet, const boost::asio::ip::udp::endpoint& source)
{
    if (error)
    {
        std::cout << "Error asyncReceive: " << error.message() << "\n";
        return;
    }

    if (packet == std::nullopt)
    {
        std::cout << "Error asyncReceive: the request packet is missing\n";
        return;
    }
    else
    {
        m_radius.asyncSend(makeResponse(*packet), source, [this](const auto& ec){ handleSend(ec); });
    }
}
