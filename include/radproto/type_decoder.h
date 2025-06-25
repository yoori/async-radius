#pragma once

#include "attribute.h"

namespace RadProto
{
  class TypeDecoder
  {
  public:
    AttributePtr decode(
      unsigned int attribute_id,
      const std::string& type_name,
      const uint8_t* data,
      size_t size,
      const std::string& secret,
      const std::array<uint8_t, 16>& auth) const;

    static const TypeDecoder& instance();

  private:
    using BaseTypeDecoder = std::function<
      RadProto::AttributePtr(
        unsigned int attribute_id,
        const uint8_t* data,
        size_t size,
        const std::string& secret,
        const std::array<uint8_t, 16>& auth)>;

  private:
    TypeDecoder();

  private:
    std::unordered_map<std::string, BaseTypeDecoder> base_type_decoders_;
  };
}
