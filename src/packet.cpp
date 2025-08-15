#include "packet.h"
#include "error.h"
#include "attribute_types.h"
#include <openssl/md5.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>

using Packet = radius_lite::Packet;

namespace
{
    radius_lite::Attribute* makeAttribute(
        uint8_t type,
        const uint8_t* data,
        size_t size,
        const std::string& secret,
        const std::array<uint8_t, 16>& auth)
    {
      //std::cerr << "makeAttribute: type = " << static_cast<unsigned int>(type) << std::endl;
        if (type == 1 || type == 11 || type == 18 || type == 22 || type == 34 || type == 35 || type == 60 || type == 63)
            return new radius_lite::String(type, data, size);
        else if (type == 2)
            return new radius_lite::Encrypted(type, data, size, secret, auth);
        else if (type == 3)
            return new radius_lite::ChapPassword(type, data, size);
        else if (type == 4 || type == 8 || type == 9 || type == 14)
            return new radius_lite::IpAddress(type, data, size);
        else if (type == 5 || type == 6 || type == 7 || type == 10 || type == 12 || type == 13 || type == 15 || type == 16 || type == 27 || type == 28 || type == 29 || type == 37 || type == 38 || type == 61 || type == 62)
            return new radius_lite::Integer<uint64_t>(type, data, size);
        else
            return new radius_lite::Bytes(type, data, size);

        throw radius_lite::Exception(radius_lite::Error::invalidAttributeType);
    }
}

Packet::Packet(
  const uint8_t* buffer,
  size_t size,
  const std::string& secret)
  : m_recalcAuth(false)
{
  /*
  std::cout << "===================" << std::endl;
  std::cout << std::hex << std::setfill('0') << std::setw(2);

  for (unsigned int i = 0; i < size; ++i)
  {
    if (i % 16 == 0)
    {
      std::cout << std::endl;
    }
    std::cout << static_cast<unsigned int>(buffer[i]) << " ";
  }

  std::cout << std::dec;
  std::cout << "===================" << std::endl;
  */

  if (size < 20)
      throw Exception(Error::numberOfBytesIsLessThan20);

  size_t length = buffer[2] * 256 + buffer[3];

  if (size < length)
      throw Exception(Error::requestLengthIsShort);

  m_type = buffer[0];

  m_id = buffer[1];

  for (std::size_t i = 0; i < m_auth.size(); ++i)
  {
    m_auth[i] = buffer[i + 4];
  }

  size_t attributeIndex = 20;
  while (attributeIndex < length)
  {
    const uint8_t attributeType = buffer[attributeIndex];
    const uint8_t attributeLength = buffer[attributeIndex + 1];

    if (attributeType == VENDOR_SPECIFIC)
    {
      m_vendorSpecific.emplace_back(VendorSpecific(&buffer[attributeIndex + 2]));
    }
    else
    {
        m_attributes.push_back(
          makeAttribute(
            attributeType,
            &buffer[attributeIndex + 2],
            attributeLength - 2,
            secret,
            m_auth));
    }

    attributeIndex += attributeLength;
  }

  bool eapMessage = false;
  bool messageAuthenticator = false;

  for (const auto& a : m_attributes)
  {
      if (a->type() == EAP_MESSAGE)
          eapMessage = true;

      if (a->type() == MESSAGE_AUTHENTICATOR)
          messageAuthenticator = true;
  }

  if (eapMessage && !messageAuthenticator)
      throw Exception(Error::eapMessageAttributeError);
}

Packet::Packet(uint8_t type, uint8_t id, const std::vector<Attribute*>& attributes,
    const std::vector<VendorSpecific>& vendorSpecific)
    : m_type(type),
      m_id(id),
      m_recalcAuth(true),
      m_auth{}, // fill auth with zeros
      m_attributes(attributes),
      m_vendorSpecific(vendorSpecific)
{
}

Packet::Packet(uint8_t type, uint8_t id, const std::array<uint8_t, 16>& auth, const std::vector<Attribute*>& attributes,
    const std::vector<VendorSpecific>& vendorSpecific,
    bool recalc_auth)
    : m_type(type),
      m_id(id),
      m_recalcAuth(m_type == 2 || recalc_auth),
      m_auth(auth),
      m_attributes(attributes),
      m_vendorSpecific(vendorSpecific)
{
}

Packet::Packet(const Packet& other)
    : m_type(other.m_type),
      m_id(other.m_id),
      m_recalcAuth(other.m_recalcAuth),
      m_auth(other.m_auth),
      m_vendorSpecific(other.m_vendorSpecific)

{
  for (const auto& a :  other.m_attributes)
  {
    if (a)
    {
      m_attributes.push_back(a->clone());
    }
  }
}

Packet::~Packet()
{
    for (const auto& ap : m_attributes)
        delete ap;
}

const std::vector<uint8_t> Packet::makeSendBuffer(const std::string& secret) const
{
    std::vector<uint8_t> sendBuffer(20);

    sendBuffer[0] = m_type;
    sendBuffer[1] = m_id;

    for (size_t i = 0; i < m_auth.size(); ++i)
    {
        sendBuffer[i + 4] = m_auth[i];
    }

    for (const auto& attribute : m_attributes)
    {
        const auto aData = attribute->toVector(secret, m_auth);
        sendBuffer.insert(sendBuffer.end(), aData.begin(), aData.end());
    }

    for (const auto& vendorAttribute : m_vendorSpecific)
    {
        const auto aData = vendorAttribute.toVector();
        sendBuffer.insert(sendBuffer.end(), aData.begin(), aData.end());
    }

    sendBuffer[2] = sendBuffer.size() / 256 % 256;
    sendBuffer[3] = sendBuffer.size() % 256;

    if (m_recalcAuth)
    {
        sendBuffer.resize(sendBuffer.size() + secret.length());

        for (size_t i = 0; i < secret.length(); ++i)
        {
            sendBuffer[i + sendBuffer.size() - secret.length()] = secret[i];
        }

        std::array<uint8_t, 16> md;
        MD5(sendBuffer.data(), sendBuffer.size(), md.data());

        sendBuffer.resize(sendBuffer.size() - secret.length());

        for (size_t i = 0; i < md.size(); ++i)
            sendBuffer[i + 4] = md[i];
    }
    return sendBuffer;
}
