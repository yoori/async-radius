#pragma once

#include <string>
#include <vector>
#include <cstdint> //uint8_t, uint32_t

#include "types.h"

namespace radius_lite
{
  class VendorSpecific
  {
  public:
    VendorSpecific(const uint8_t* data);

    VendorSpecific(uint32_t vendorId, uint8_t vendorType, const std::vector<uint8_t>& vendorValue);

    std::string toString() const;

    uint8_t vendorType() const { return m_vendorType; }

    uint32_t vendorId() const { return m_vendorId; }

    std::vector<uint8_t> toVector() const;

    const ByteArray& data() const;

  private:
    uint32_t m_vendorId;
    uint8_t m_vendorType;
    std::vector<uint8_t> m_value;
  };
}
