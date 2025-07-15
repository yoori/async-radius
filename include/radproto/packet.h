#pragma once

#include "attribute.h"
#include "vendor_attribute.h"
#include "dictionaries.h"

#include <array>
#include <vector>
#include <string>
#include <cstdint> //uint8_t, uint32_t

namespace RadProto
{
  class Packet
  {
  public:
    friend class PacketReader;

  public:
    Packet(
      const uint8_t* buffer,
      size_t size,
      const std::string& secret);

    // request packet
    Packet(
      uint8_t type,
      uint8_t id,
      const std::vector<Attribute*>& attributes,
      const std::vector<VendorSpecific>& vendorSpecific);

    // response packet,
    // auth : authenticator of request should be passed here
    Packet(
      uint8_t type,
      uint8_t id,
      const std::array<uint8_t, 16>& auth,
      const std::vector<Attribute*>& attributes,
      const std::vector<VendorSpecific>& vendorSpecific,
      bool recalc_auth = false
      );

    Packet(const Packet& other);
    ~Packet();
    uint8_t type() const { return m_type; }
    uint8_t id() const { return m_id; };
    const std::array<uint8_t, 16>& auth() const { return m_auth; }
    const std::vector<Attribute*>& attributes() const { return m_attributes; }
    const std::vector<VendorSpecific>& vendorSpecific() const { return m_vendorSpecific; }
    const std::vector<uint8_t> makeSendBuffer(const std::string& secret) const;

  private:
    uint8_t m_type;
    uint8_t m_id;
    bool m_recalcAuth;
    std::array<uint8_t, 16> m_auth;
    std::vector<Attribute*> m_attributes;
    std::vector<VendorSpecific> m_vendorSpecific;
  };
}
