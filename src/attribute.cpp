#include "packet.h"
#include "attribute.h"
#include "utils.h"
#include "error.h"
#include <openssl/md5.h>
#include <algorithm>
#include <iostream>

namespace radius_lite
{
  Attribute::Attribute(uint8_t type)
    : m_type(type)
  {}

  // String impl
  String::String(uint8_t type, const uint8_t* data, size_t size)
    : Attribute(type),
      m_value(reinterpret_cast<const char*>(data), size)
  {}

  String::String(uint8_t type, const std::string& string)
    : Attribute(type),
      m_value(string)
  {}

  ByteArray
  String::data(const std::string& /*secret*/, const std::array<uint8_t, 16>& /*auth*/) const
  {
    return ByteArray(m_value.begin(), m_value.end());
  }

  String* String::clone() const
  {
    return new String(*this);
  }

  // IpAddress impl
  IpAddress::IpAddress(uint8_t type, const uint8_t* data, size_t size)
    : Attribute(type)
  {
    if (size != 4)
    {
      throw radius_lite::Exception(radius_lite::Error::invalidAttributeSize);
    }

    for (size_t i = 0; i < size; ++i)
    {
      m_value[i] = data[i];
    }
  }

  IpAddress::IpAddress(uint8_t type, const std::array<uint8_t, 4>& address)
    : Attribute(type),
      m_value(address)
  {}

  std::string
  IpAddress::toString() const
  {
    return std::to_string(m_value[0]) + "." + std::to_string(m_value[1]) + "." +
      std::to_string(m_value[2]) + "." + std::to_string(m_value[3]);
  }

  std::optional<uint64_t>
  IpAddress::as_uint() const
  {
    return (
      (static_cast<uint32_t>(m_value[3]) << 24) |
      (static_cast<uint32_t>(m_value[2]) << 16) |
      (static_cast<uint32_t>(m_value[1]) << 8) |
      static_cast<uint32_t>(m_value[0]));
  }

  ByteArray
  IpAddress::data(const std::string& /*secret*/, const std::array<uint8_t, 16>& /*auth*/) const
  {
    std::vector<uint8_t> result(4);
    std::copy(m_value.begin(), m_value.end(), result.begin());
    return result;
  }

  IpAddress* IpAddress::clone() const
  {
    return new IpAddress(*this);
  }

  // Encrypted impl
  Encrypted::Encrypted(
    uint8_t type,
    const uint8_t* data,
    size_t size,
    const std::string& secret,
    const std::array<uint8_t, 16>& auth)
    : Attribute(type)
  {
    if (size > 128)
    {
      throw radius_lite::Exception(radius_lite::Error::invalidAttributeSize);
    }

    std::vector<uint8_t> mdBuffer(auth.size() + secret.length());

    std::copy(secret.begin(), secret.end(), mdBuffer.begin());
    std::copy(auth.begin(), auth.end(), mdBuffer.begin() + secret.length());

    std::vector<uint8_t> plaintext(size);

    for (size_t i = 0; i < size / 16; ++i)
    {
      std::array<uint8_t, 16> md;

      MD5(mdBuffer.data(), mdBuffer.size(), md.data());
      for (size_t j = 0; j < 16; ++j)
      {
        plaintext[i * 16 + j] = data[i * 16 + j] ^ md[j];
      }

      for (size_t j = 0; j < 16; ++j)
      {
        mdBuffer[j + secret.length()] = data[i * 16 + j];
      }
    }

    m_value.assign(plaintext.begin(), std::find(plaintext.begin(), plaintext.end(), 0));
  }

  Encrypted::Encrypted(uint8_t type, const std::string& password)
    : Attribute(type),
      m_value(password)
  {}

  ByteArray
  Encrypted::data(const std::string& secret, const std::array<uint8_t, 16>& auth) const
  {
    std::string plaintext = m_value;

    if (plaintext.length() % 16 != 0)
    {
      plaintext.append(16 - m_value.length() % 16, '\0');
    }

    ByteArray mdBuffer(auth.size() + secret.length());

    for (size_t i = 0; i < secret.length(); ++i)
    {
      mdBuffer[i] = secret[i];
    }

    for (size_t i = 0; i < auth.size(); ++i)
    {
      mdBuffer[i + secret.length()] = auth[i];
    }

    ByteArray res(plaintext.length());
    auto it = std::next(res.begin());

    for (size_t i = 0; i < plaintext.length() / 16; ++i)
    {
      std::array<uint8_t, 16> md;
      MD5(mdBuffer.data(), mdBuffer.size(), md.data());

      for (size_t j = 0; j < md.size(); ++j)
      {
        *it++ = plaintext[i * 16 + j] ^ md[j];
      }

      for (size_t j = 0; j < 16; ++j)
      {
        mdBuffer[j + secret.length()] = res[i * 16 + j];
      }
    }

    return res;
  }

  Encrypted* Encrypted::clone() const
  {
    return new Encrypted(*this);
  }

  Bytes::Bytes(uint8_t type, const uint8_t* data, size_t size)
    : Attribute(type),
      m_value(size)
  {
    std::copy(data, data + size, m_value.begin());
  }

  Bytes::Bytes(uint8_t type, const std::vector<uint8_t>& bytes)
    : Attribute(type),
      m_value(bytes)
  {}

  std::string Bytes::toString() const
  {
    std::string value;

    for (const auto& b : m_value)
    {
      value += byteToHex(b);
    }

    return value;
  }

  ByteArray
  Bytes::data(const std::string& /*secret*/, const std::array<uint8_t, 16>& /*auth*/) const
  {
    return m_value;
  }

  Bytes* Bytes::clone() const
  {
    return new Bytes(*this);
  }

  // ChapPassword impl
  ChapPassword::ChapPassword(uint8_t type, const uint8_t* data, size_t size)
    : Attribute(type),
      m_value(size - 1)
  {
    if (size != 17)
    {
      throw radius_lite::Exception(radius_lite::Error::invalidAttributeSize);
    }

    m_chapId = data[0];

    for (size_t i = 0; i < size - 1; ++i)
    {
      m_value[i] = data[i + 1];
    }
  }

  ChapPassword::ChapPassword(uint8_t type, uint8_t chapId, const std::vector<uint8_t>& chapValue)
    : Attribute(type),
      m_chapId(chapId),
      m_value(chapValue)
  {
  }

  std::string ChapPassword::toString() const
  {
    std::string value;

    for (const auto& b : m_value)
    {
      value += byteToHex(b);
    }

    return std::to_string(m_chapId) + " " + value;
  }

  ByteArray
  ChapPassword::data(const std::string& /*secret*/, const std::array<uint8_t, 16>& /*auth*/) const
  {
    ByteArray result(m_value.size() + 1);
    result[0] = m_chapId;
    std::copy(m_value.begin(), m_value.end(), result.begin() + 1);
    return result;
  }

  ChapPassword* ChapPassword::clone() const
  {
    return new ChapPassword(*this);
  }
}
