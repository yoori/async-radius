#pragma once

#include "attribute.h"
#include "dictionaries.h"
#include "packet.h"

namespace RadProto
{
  class PacketReader
  {
  public:
    PacketReader(
      const Packet& packet,
      const Dictionaries& dictionaries,
      std::string secret);

    ConstAttributePtr
    get_attribute(const Dictionaries::AttributeKey& attribute_key) const;

    ConstAttributePtr
    get_attribute_by_name(const std::string& name) const;

    ConstAttributePtr
    get_attribute_by_name(const std::string& name, const std::string& vendor_name) const;

  private:
    const Packet& packet_;
    const Dictionaries& dictionaries_;
    const std::string secret_;
  };
}
