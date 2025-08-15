#pragma once

#include <string>
#include <vector>
#include <array>
#include <cstdint> //uint8_t, uint32_t
#include <memory>
#include <optional>

#include "error.h"
#include "types.h"

namespace radius_lite
{
  class Attribute
  {
  public:
    Attribute(uint8_t type);
    virtual ~Attribute() = default;
    uint8_t type() const { return m_type; }
    virtual std::string toString() const = 0;
    virtual ByteArray data(const std::string& secret, const Auth& auth) const = 0;

    // rename to serialize
    virtual std::vector<uint8_t> toVector(const std::string& secret, const Auth& auth) const;
    virtual Attribute* clone() const = 0;

    virtual std::optional<int64_t> as_int() const;
    virtual std::optional<uint64_t> as_uint() const;
    virtual std::optional<std::string> as_string() const;
    virtual ByteArray as_octets() const = 0; // each type can be represented as octets

  private:
    uint8_t m_type;
  };

  // String
  class String: public Attribute
  {
  public:
    String(uint8_t type, const uint8_t* data, size_t size);
    String(uint8_t type, const std::string& string);
    std::string toString() const override { return m_value; }
    std::vector<uint8_t> data(const std::string& secret, const std::array<uint8_t, 16>& auth) const override;
    //std::vector<uint8_t> toVector(const std::string& secret, const std::array<uint8_t, 16>& auth) const override;
    String* clone() const override;

    std::optional<std::string> as_string() const override;
    ByteArray as_octets() const override;

  private:
    std::string m_value;
  };

  // Integer
  template<typename IntType>
  class Integer: public Attribute
  {
  public:
    Integer(uint8_t type, const uint8_t* data, size_t size);
    Integer(uint8_t type, uint32_t value);
    std::string toString() const override;
    ByteArray data(const std::string& secret, const std::array<uint8_t, 16>& auth) const override;
    //std::vector<uint8_t> toVector(const std::string& secret, const std::array<uint8_t, 16>& auth) const override;
    Integer* clone() const override;

    std::optional<int64_t> as_int() const override;
    std::optional<uint64_t> as_uint() const override;
    std::optional<std::string> as_string() const override;
    ByteArray as_octets() const override;

  private:
    IntType m_value;
  };

  // IpAddress
  class IpAddress : public Attribute
  {
  public:
    IpAddress(uint8_t type, const uint8_t* data, size_t size);
    IpAddress(uint8_t type, const std::array<uint8_t, 4>& address);
    std::string toString() const override;
    ByteArray data(const std::string& secret, const std::array<uint8_t, 16>& auth) const override;
    IpAddress* clone() const override;

    std::optional<std::string> as_string() const override;
    std::optional<uint64_t> as_uint() const override;
    ByteArray as_octets() const override;

  private:
    std::array<uint8_t, 4> m_value;
  };

  // Encrypted
  class Encrypted : public Attribute
  {
  public:
    Encrypted(uint8_t type, const uint8_t* data, size_t size, const std::string& secret, const std::array<uint8_t, 16>& auth);
    Encrypted(uint8_t type, const std::string& password);
    std::string toString() const override { return m_value; }
    ByteArray data(const std::string& secret, const std::array<uint8_t, 16>& auth) const override;
    //std::vector<uint8_t> toVector(const std::string& secret, const std::array<uint8_t, 16>& auth) const override;
    Encrypted* clone() const override;

    ByteArray as_octets() const override;

  private:
    std::string m_value;
  };

  class Bytes: public Attribute
  {
  public:
    Bytes(uint8_t type, const uint8_t* data, size_t size);
    Bytes(uint8_t type, const std::vector<uint8_t>& bytes);
    std::string toString() const override;
    ByteArray data(const std::string& secret, const std::array<uint8_t, 16>& auth) const override;
    //std::vector<uint8_t> toVector(const std::string& secret, const std::array<uint8_t, 16>& auth) const override;
    Bytes* clone() const override;

    ByteArray as_octets() const override;

  private:
    ByteArray m_value;
  };

  class ChapPassword: public Attribute
  {
  public:
    ChapPassword(uint8_t type, const uint8_t* data, size_t size);
    ChapPassword(uint8_t type, uint8_t chapId, const std::vector<uint8_t>& chapValue);
    std::string toString() const override;
    uint8_t chapId() const { return m_chapId; }
    std::vector<uint8_t> chapValue() const { return m_value; }
    ByteArray data(const std::string& secret, const std::array<uint8_t, 16>& auth) const override;
    //std::vector<uint8_t> toVector(const std::string& secret, const std::array<uint8_t, 16>& auth) const override;
    ChapPassword* clone() const override;

    ByteArray as_octets() const override;

  private:
    uint8_t m_chapId;
    ByteArray m_value;
  };

  using ConstAttributePtr = std::shared_ptr<const Attribute>;
  using AttributePtr = std::shared_ptr<Attribute>;
}

namespace radius_lite
{
  // Attribute inlines
  inline std::optional<int64_t>
  Attribute::as_int() const
  {
    return std::nullopt;
  }

  inline std::optional<uint64_t>
  Attribute::as_uint() const
  {
    return std::nullopt;
  }

  inline std::optional<std::string>
  Attribute::as_string() const
  {
    return std::nullopt;
  }

  inline ByteArray
  Attribute::toVector(const std::string& secret, const Auth& auth) const
  {
    auto serialize_data = data(secret, auth);
    ByteArray result;
    result.reserve(2 + serialize_data.size());
    result.resize(2);
    result[0] = type();
    result[1] = 2 + serialize_data.size();
    result.insert(result.end(), serialize_data.begin(), serialize_data.end());
    return result;
  }

  // Integer inlines
  template<typename IntType>
  Integer<IntType>::Integer(uint8_t type, const uint8_t* data, size_t size)
    : Attribute(type),
      m_value(0)
  {
    if (size > sizeof(IntType))
    {
      throw radius_lite::Exception(radius_lite::Error::invalidAttributeSize);
    }

    if (size == 8 || size == 4 || size == 2 || size == 1)
    {
      for (size_t i = 0; i < size; ++i)
      {
        m_value = (m_value << 8) | data[i];
      }
    }
    else
    {
      throw radius_lite::Exception(radius_lite::Error::invalidAttributeSize);
    }
  }

  template<typename IntType>
  Integer<IntType>::Integer(uint8_t type, uint32_t value)
    : Attribute(type),
      m_value(value)
  {}

  template<typename IntType>
  std::string Integer<IntType>::toString() const
  {
    return std::to_string(m_value);
  }

  template<typename IntType>
  std::vector<uint8_t> Integer<IntType>::data(const std::string& /*secret*/, const std::array<uint8_t, 16>& /*auth*/) const
  {
    std::vector<uint8_t> attribute(2 + sizeof(IntType));
    attribute[0] = type();
    attribute[1] = 2 + sizeof(IntType);
    for (size_t i = 0; i < sizeof(IntType); ++i)
    {
      attribute[2 + sizeof(IntType) - i - 1] = (m_value >> (i * 8)) & 0xFF;
    }
    return attribute;
  }

  template<typename IntType>
  Integer<IntType>* Integer<IntType>::clone() const
  {
    return new Integer<IntType>(*this);
  }

  template<typename IntType>
  std::optional<int64_t>
  Integer<IntType>::as_int() const
  {
    // TODO: control types compatibility.
    return m_value;
  }

  template<typename IntType>
  std::optional<uint64_t>
  Integer<IntType>::as_uint() const
  {
    // TODO: control types compatibility.
    return m_value;
  }

  template<typename IntType>
  std::optional<std::string>
  Integer<IntType>::as_string() const
  {
    return std::to_string(m_value);
  }

  template<typename IntType>
  ByteArray
  Integer<IntType>::as_octets() const
  {
    std::vector<uint8_t> attribute(sizeof(IntType));
    for (size_t i = 0; i < sizeof(IntType); ++i)
    {
      attribute[sizeof(IntType) - i - 1] = (m_value >> (i * 8)) & 0xFF;
    }
    return attribute;
  }

  // IpAddress inlines
  inline std::optional<std::string>
  IpAddress::as_string() const
  {
    return std::to_string(m_value[0]) + "." +
      std::to_string(m_value[1]) + "." +
      std::to_string(m_value[2]) + "." +
      std::to_string(m_value[3]);
  }

  inline ByteArray
  IpAddress::as_octets() const
  {
    return ByteArray(m_value.begin(), m_value.end());
  }

  // Encrypted inlines
  inline ByteArray
  Encrypted::as_octets() const
  {
    return ByteArray(
      reinterpret_cast<const uint8_t*>(m_value.data()),
      reinterpret_cast<const uint8_t*>(m_value.data()) + m_value.size());
  }

  // String inlines
  inline std::optional<std::string>
  String::as_string() const
  {
    return m_value;
  }

  inline ByteArray
  String::as_octets() const
  {
    return ByteArray(
      reinterpret_cast<const uint8_t*>(m_value.data()),
      reinterpret_cast<const uint8_t*>(m_value.data()) + m_value.size());
  }

  // Bytes inlines
  inline ByteArray
  Bytes::as_octets() const
  {
    return m_value;
  }
  
  // ChapPassword inlines
  inline ByteArray
  ChapPassword::as_octets() const
  {
    return m_value;
  }
}
