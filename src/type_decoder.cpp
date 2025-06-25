#include "attribute.h"
#include "type_decoder.h"

namespace RadProto
{
  template<typename IntType>
  std::function<RadProto::AttributePtr(
    unsigned int attribute_id,
    const uint8_t* data,
    size_t size,
    const std::string& secret,
    const std::array<uint8_t, 16>& auth)>
  createIntegerDecodeFun()
  {
    auto fun = [] (
      unsigned int attribute_id,
      const uint8_t* data,
      size_t size,
      const std::string&,
      const std::array<uint8_t, 16>&)
    {
      return std::make_shared<RadProto::Integer<IntType>>(attribute_id, data, size);
    };
    return fun;
  }

  TypeDecoder::TypeDecoder()
  {
    base_type_decoders_.emplace(
      "uint16",
      createIntegerDecodeFun<uint16_t>()
      );
    base_type_decoders_.emplace(
      "integer",
      createIntegerDecodeFun<int32_t>()
      );
    base_type_decoders_.emplace(
      "uint32",
      createIntegerDecodeFun<uint32_t>()
      );
    base_type_decoders_.emplace(
      "uint64",
      createIntegerDecodeFun<uint64_t>()
      );
    base_type_decoders_.emplace(
      "byte",
      createIntegerDecodeFun<uint8_t>()
      );
    base_type_decoders_.emplace(
      "string",
      [] (
        unsigned int attribute_id,
        const uint8_t* data,
        size_t size,
        const std::string&,
        const std::array<uint8_t, 16>&)
      {
        return std::make_shared<RadProto::String>(attribute_id, data, size);
      }
    );
    base_type_decoders_.emplace(
      "ipaddr",
      [] (
        unsigned int attribute_id,
        const uint8_t* data,
        size_t size,
        const std::string&,
        const std::array<uint8_t, 16>&)
      {
        return std::make_shared<RadProto::IpAddress>(attribute_id, data, size);
      }
    );
  }

  RadProto::AttributePtr TypeDecoder::decode(
    unsigned int attribute_id,
    const std::string& type_name,
    const uint8_t* data,
    size_t size,
    const std::string& secret,
    const std::array<uint8_t, 16>& auth) const
  {
    auto decoder_it = base_type_decoders_.find(type_name);
    if (decoder_it != base_type_decoders_.end())
    {
      return decoder_it->second(attribute_id, data, size, secret, auth);
    }

    return std::make_shared<RadProto::Bytes>(attribute_id, data, size);
  }

  const TypeDecoder&
  TypeDecoder::instance()
  {
    static TypeDecoder type_decoder;
    return type_decoder;
  }
}
