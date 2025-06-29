#include <iostream>

#include "packet_reader.h"
#include "type_decoder.h"

namespace RadProto
{
  PacketReader::PacketReader(
    const Packet& packet,
    const Dictionaries& dictionaries,
    std::string secret)
    : packet_(packet),
      dictionaries_(dictionaries),
      secret_(std::move(secret))
  {}

  ConstAttributePtr
  PacketReader::get_attribute_by_name(const std::string& name) const
  {
    for (const auto& attribute : packet_.attributes())
    {
      auto attribute_id = attribute->type();
      //std::cout << "A: " << dictionaries_.attributeName(attribute_id) << std::endl;

      if (dictionaries_.attributeName(attribute_id) == name)
      {
        auto attribute_type = dictionaries_.get_attribute_type(attribute_id);
        //std::cout << "ATYPE: " << (attribute_type.has_value() ? *attribute_type : std::string("unknown")) << std::endl;
        if (attribute_type.has_value())
        {
          auto plain_value = attribute->data(secret_, packet_.auth());
          return TypeDecoder::instance().decode(
            attribute_id,
            *attribute_type,
            plain_value.data(),
            plain_value.size(),
            secret_,
            packet_.auth());
        }
      }
    }

    return ConstAttributePtr();
  }

  ConstAttributePtr
  PacketReader::get_attribute_by_name(const std::string& name, const std::string& vendor_name) const
  {
    if (!vendor_name.empty())
    {
      auto vendor_id = dictionaries_.vendorNames().code(vendor_name);
      auto vendor_attr_id = dictionaries_.vendorAttributes().code(std::string(vendor_name), name);

      for (const auto& attribute : packet_.vendorSpecific())
      {
        if (attribute.vendorId() == vendor_id &&
          attribute.vendorType() == vendor_attr_id)
        {
          auto attribute_type = dictionaries_.get_attribute_type(vendor_attr_id, vendor_id);
          //std::cout << "ATYPE: " << (attribute_type.has_value() ? *attribute_type : std::string("unknown")) << std::endl;
          if (attribute_type.has_value())
          {
            const auto plain_value = attribute.data();
            return TypeDecoder::instance().decode(
              vendor_attr_id,
              *attribute_type,
              plain_value.data(),
              plain_value.size(),
              secret_,
              packet_.auth());
          }
        }
      }

      return ConstAttributePtr();
    }
    else
    {
      return get_attribute_by_name(name);
    }
  }

  ConstAttributePtr
  PacketReader::get_attribute(const Dictionaries::AttributeKey& attribute_key) const
  {
    if (attribute_key.vendor_id != 0)
    {
      for (const auto& attribute : packet_.vendorSpecific())
      {
        if (attribute.vendorId() == attribute_key.vendor_id &&
          attribute.vendorType() == attribute_key.code)
        {
          auto attribute_type = dictionaries_.get_attribute_type(
            attribute_key.code,
            attribute_key.vendor_id);

          if (attribute_type.has_value())
          {
            const auto plain_value = attribute.data();
            return TypeDecoder::instance().decode(
              attribute_key.code,
              *attribute_type,
              plain_value.data(),
              plain_value.size(),
              secret_,
              packet_.auth());
          }
        }
      }
    }
    else
    {
      for (const auto& attribute : packet_.attributes())
      {
        auto attribute_id = attribute->type();

        if (attribute_id == attribute_key.code)
        {
          auto attribute_type = dictionaries_.get_attribute_type(attribute_id);

          if (attribute_type.has_value())
          {
            auto plain_value = attribute->data(secret_, packet_.auth());
            return TypeDecoder::instance().decode(
              attribute_id,
              *attribute_type,
              plain_value.data(),
              plain_value.size(),
              secret_,
              packet_.auth());
          }
        }
      }
    }

    return ConstAttributePtr();
  }
}
