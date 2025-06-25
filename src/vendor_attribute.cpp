#include "packet.h"
#include "vendor_attribute.h"
#include "utils.h"
#include "error.h"
#include "attribute_types.h"
#include <iostream>

using VendorSpecific = RadProto::VendorSpecific;

namespace RadProto
{
  VendorSpecific::VendorSpecific(const uint8_t* data)
  {
    if (data[0] != 0)
        throw RadProto::Exception(RadProto::Error::invalidVendorSpecificAttributeId);

    m_vendorId = data[0] * (1 << 24)
               + data[1] * (1 << 16)
               + data[2] * (1 << 8)
               + data[3];

    m_vendorType = data[4];

    size_t vendorLength = data[5];
    m_value.resize(vendorLength - 2);

    for (size_t i = 0; i < vendorLength - 2; ++i)
        m_value[i] = data[i + 6];
  }

  VendorSpecific::VendorSpecific(uint32_t vendorId, uint8_t vendorType, const std::vector<uint8_t>& vendorValue)
    : m_vendorId(vendorId),
      m_vendorType(vendorType),
      m_value(vendorValue)
  {
  }

  const ByteArray& VendorSpecific::data() const
  {
    return m_value;
  }

  std::vector<uint8_t> VendorSpecific::toVector() const
  {
    std::vector<uint8_t> attribute(m_value.size() + 8);
    attribute[0] = VENDOR_SPECIFIC;
    attribute[1] = m_value.size() + 8;
    attribute[2] = m_vendorId / (1 << 24);
    attribute[3] = (m_vendorId / (1 << 16)) % 256;
    attribute[4] = (m_vendorId / (1 << 8)) % 256;
    attribute[5] = m_vendorId % 256;
    attribute[6] = vendorType();
    attribute[7] = m_value.size() + 2;
    for (size_t i = 0; i < m_value.size(); ++i)
        attribute[i + 8] = m_value[i];
    return attribute;
  }

  std::string VendorSpecific::toString() const
  {
    std::string value;

    for (const auto& b : m_value)
    {
      value += byteToHex(b);
    }

    return value;
  }
}
