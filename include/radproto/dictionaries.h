#pragma once

#include <string>
#include <map>
#include <cstdint> //uint8_t, uint32_t
#include <optional>
#include <unordered_map>

namespace RadProto
{
  class BasicDictionary
  {
  public:
    BasicDictionary() = default;

    std::string name(uint32_t code) const;

    uint32_t code(const std::string& name) const;

    void add(uint32_t code, const std::string& name);

    void append(const BasicDictionary& basicDict);

  private:
    std::map<uint32_t, std::string> right_dict_;
    std::map<std::string, uint32_t> reverse_dict_;
  };

  class DependentDictionary
  {
  public:
    DependentDictionary() = default;

    std::string name(const std::string& dependencyName, uint32_t code) const;

    uint32_t code(const std::string& dependencyName, const std::string& name) const;

    void add(uint32_t code, const std::string& name, const std::string& dependencyName);

    void append(const DependentDictionary& dependentDict);

  private:
    std::map<std::pair<std::string, uint32_t>, std::string> right_dict_;
    std::map<std::pair<std::string, std::string>, uint32_t> reverse_dict_;
  };

  class Dictionaries
  {
  public:
    struct AttributeKey
    {
      uint8_t code = 0;
      uint32_t vendor_id = 0;

      AttributeKey() {}

      AttributeKey(uint8_t code_val, uint32_t vendor_id_val = 0)
        : code(code_val), vendor_id(vendor_id_val)
      {}

      bool operator==(const AttributeKey& right) const
      {
        return code == right.code && vendor_id == right.vendor_id;
      }
    };

  public:
    Dictionaries(const std::string& filePath);
    void append(const Dictionaries& fillingDictionaries);
    const BasicDictionary& attributes() const { return m_attributes; }
    const BasicDictionary& vendorNames() const { return m_vendorNames; }
    const DependentDictionary& attributeValues() const { return m_attributeValues; }
    const DependentDictionary& vendorAttributes() const { return m_vendorAttributes; }
    const DependentDictionary& vendorAttributeValues() const;

    std::string attributeName(uint32_t code) const;
    uint32_t attributeCode(const std::string& name) const;

    std::string attributeTypeName(uint8_t code, uint32_t vendor_id = 0) const;

    std::string vendorName(uint32_t code) const;
    uint32_t vendorCode(const std::string& name) const;

    std::string vendorAttributeName(const std::string& vendorName, uint32_t code) const;
    uint32_t vendorAttributeCode(const std::string& vendorName, const std::string& name) const;

    std::string attributeValueName(const std::string& attributeName, uint32_t code) const;
    uint32_t attributeValueCode(const std::string& attributeName, const std::string& name) const;

    std::string vendorAttributeValueName(const std::string& valueName, uint32_t code) const;
    uint32_t vendorAttributeValueCode(const std::string& valueName, const std::string& name) const;

    std::optional<std::string> get_attribute_type(uint8_t code, uint32_t vendor_id = 0) const;

    void resolve();

  private:
    struct AttributeKeyHash
    {
      std::size_t operator()(const AttributeKey& attribute_key) const;
    };

    struct UnresolvedAttributeKey
    {
      uint8_t code = 0;
      std::string vendor_name;

      UnresolvedAttributeKey() {}

      UnresolvedAttributeKey(uint8_t code_val, std::string vendor_name_val)
        : code(code_val), vendor_name(std::move(vendor_name_val))
      {}

      bool operator==(const UnresolvedAttributeKey& right) const
      {
        return code == right.code && vendor_name == right.vendor_name;
      }
    };

    struct UnresolvedAttributeKeyHash
    {
      std::size_t operator()(const UnresolvedAttributeKey& attribute_key) const;
    };

  private:
    BasicDictionary m_attributes;
    BasicDictionary m_vendorNames;
    DependentDictionary m_attributeValues;
    DependentDictionary m_vendorAttributes;
    DependentDictionary vendor_attribute_values_;

    std::unordered_map<AttributeKey, std::string, AttributeKeyHash> m_attributeTypes;
    std::unordered_map<UnresolvedAttributeKey, std::string, UnresolvedAttributeKeyHash> m_unresolvedAttributeTypes;
  };
}

namespace RadProto
{
  inline const DependentDictionary&
  Dictionaries::vendorAttributeValues() const
  {
    return vendor_attribute_values_;
  }
}
