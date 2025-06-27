#include <vector>
#include <utility>
#include <fstream>
#include <iostream>

#include <boost/tokenizer.hpp>
#include <boost/functional/hash.hpp>

#include "dictionaries.h"
#include "error.h"

namespace RadProto
{
  std::size_t RadProto::Dictionaries::AttributeKeyHash::operator()(const AttributeKey& attribute_key) const
  {
    std::size_t seed = 0;
    boost::hash_combine(seed, attribute_key.code);
    boost::hash_combine(seed, attribute_key.vendor_id);
    return seed;
  };

  std::size_t RadProto::Dictionaries::UnresolvedAttributeKeyHash::operator()(
    const UnresolvedAttributeKey& attribute_key) const
  {
    std::size_t seed = 0;
    boost::hash_combine(seed, attribute_key.code);
    boost::hash_combine(seed, attribute_key.vendor_name);
    return seed;
  };

  // BasicDictionary impl
  std::string BasicDictionary::name(uint32_t code) const
  {
      return right_dict_.at(code);
  }

  uint32_t BasicDictionary::code(const std::string& name) const
  {
    return reverse_dict_.at(name);
  }

  void BasicDictionary::add(uint32_t code, const std::string& name)
  {
    for (const auto& entry: right_dict_)
    {
      if (entry.second == name && entry.first != code)
      {
        throw Exception(
          Error::suchAttributeNameAlreadyExists,
          "[BasicDictionary::add]. Attribute name " + name + " already exists with code " + std::to_string(entry.first));
      }
    }

    right_dict_.insert_or_assign(code, name);
    reverse_dict_.emplace(name, code);
  }

  void BasicDictionary::append(const BasicDictionary& basicDict)
  {
    for (const auto& entry: basicDict.right_dict_)
    {
      for (const auto& item: right_dict_)
      {
        if (entry.second == item.second && entry.first != item.first)
        {
          throw Exception(
            Error::suchAttributeNameAlreadyExists,
            "[BasicDictionary::append]. Attribute name " + entry.second + " already exists with code " +
            std::to_string(item.first));
        }
      }

      right_dict_.insert_or_assign(entry.first, entry.second);
    }

    for (const auto& entry: basicDict.reverse_dict_)
    {
      reverse_dict_.emplace(entry.first, entry.second);
    }
  }

  // DependentDictionary impl
  std::string DependentDictionary::name(const std::string& dependencyName, uint32_t code) const
  {
    return right_dict_.at(std::make_pair(dependencyName, code));
  }

  uint32_t DependentDictionary::code(const std::string& dependencyName, const std::string& name) const
  {
    return reverse_dict_.at(std::make_pair(dependencyName, name));
  }

  void DependentDictionary::add(uint32_t code, const std::string& name, const std::string& dependencyName)
  {
    for (const auto& entry: right_dict_)
    {
      if (entry.second == name && entry.first.first == dependencyName && entry.first.second != code)
      {
        throw Exception(
          Error::suchAttributeNameAlreadyExists,
          "[DependentDictionary::add]. Value name " + name + " of attribute " + dependencyName +
          " already exists with code " + std::to_string(entry.first.second));
      }
    }

    right_dict_.insert_or_assign(std::make_pair(dependencyName, code), name);
    reverse_dict_.emplace(std::make_pair(dependencyName, name), code);
  }

  void DependentDictionary::append(const DependentDictionary& dependentDict)
  {
    for (const auto& entry: dependentDict.right_dict_)
    {
      for (const auto& item: right_dict_)
      {
        if (item.second == entry.second && item.first.first == entry.first.first &&
          item.first.second != entry.first.second)
        {
          throw Exception(Error::suchAttributeNameAlreadyExists,
            "[DependentDictionary::append]. Value name " + entry.second + " of attribute " +
            entry.first.first +
            "(code = " + std::to_string(entry.first.second) + ") already exists with code " +
            std::to_string(item.first.second));
        }
      }

      right_dict_.insert_or_assign(std::make_pair(entry.first.first, entry.first.second), entry.second);
    }

    for (const auto& entry: dependentDict.reverse_dict_)
    {
      reverse_dict_.insert_or_assign(entry.first, entry.second);
    }
  }

  // Dictionaries impl
  Dictionaries::Dictionaries(const std::string& filePath)
  {
    std::ifstream stream(filePath);
    if (!stream)
    {
      throw std::runtime_error("Cannot open dictionary file " + filePath);
    }

    using tokenizer = boost::tokenizer<boost::char_separator<char>>;
    boost::char_separator<char> sep(" \t");

    std::string line;
    std::string vendorName;

    while (std::getline(stream, line))
    {
      tokenizer tok(line, sep);

      std::vector<std::string> tokens;
      std::copy(tok.begin(), tok.end(), std::back_inserter(tokens));

      if (!tokens.empty())
      {
        if (tokens[0] == "ATTRIBUTE")
        {
          const auto& attrName = tokens[1];
          const auto& attrId = tokens[2];

          if (attrId.find('.') == std::string::npos) // skip attrbutes with OID
          {
            const auto code = std::stoul(tokens[2]);
            std::string attrTypeName = tokens[3];

            auto size_part_pos = attrTypeName.find('[');
            if (size_part_pos != std::string::npos)
            {
              attrTypeName = attrTypeName.substr(0, size_part_pos);
            }

            if (!vendorName.empty())
            {
              m_vendorAttributes.add(code, attrName, vendorName);
            }
            else
            {
              m_attributes.add(code, attrName);
            }

            bool resolved = true;
            uint32_t vendor_id = 0;
            if (!vendorName.empty())
            {
              try
              {
                vendor_id = m_vendorNames.code(vendorName);
              }
              catch(const std::exception&)
              {
                resolved = false;
              }
            }

            if (resolved)
            {
              m_attributeTypes.emplace(AttributeKey(code, vendor_id), attrTypeName);
            }
            else
            {
              m_unresolvedAttributeTypes.emplace(
                UnresolvedAttributeKey(code, vendorName), attrTypeName);
            }
          }
        }
        else if (tokens[0] == "VALUE")
        {
          const auto& attrNameVal = tokens[1];
          const auto& valueName = tokens[2];
          const auto valueCode = std::stoul(tokens[3]);
          if (!vendorName.empty())
          {
            vendor_attribute_values_.add(valueCode, valueName, attrNameVal);
          }
          else
          {
            m_attributeValues.add(valueCode, valueName, attrNameVal);
          }
        }
        else if (tokens[0] == "VENDOR")
        {
          m_vendorNames.add(std::stoul(tokens[2]), tokens[1]);
        }
        else if (tokens[0] == "BEGIN-VENDOR")
        {
          vendorName = tokens[1];
        }
        else if (tokens[0] == "END-VENDOR")
        {
          vendorName.clear();
        }
        else if (tokens[0] == "$INCLUDE")
        {
          if (tokens[1].substr(0, 1) == "/")
          {
            append(Dictionaries(tokens[1]));
          }
          else
          {
            append(Dictionaries(filePath.substr(0, filePath.rfind('/') + 1) + tokens[1]));
          }
        }
      }
    }
  }

  void Dictionaries::append(const Dictionaries& fillingDictionaries)
  {
    m_attributes.append(fillingDictionaries.m_attributes);
    m_vendorNames.append(fillingDictionaries.m_vendorNames);
    m_attributeValues.append(fillingDictionaries.m_attributeValues);
    m_vendorAttributes.append(fillingDictionaries.m_vendorAttributes);
    vendor_attribute_values_.append(fillingDictionaries.vendor_attribute_values_);
    m_attributeTypes.insert(
      fillingDictionaries.m_attributeTypes.begin(),
      fillingDictionaries.m_attributeTypes.end());
    m_unresolvedAttributeTypes.insert(
      fillingDictionaries.m_unresolvedAttributeTypes.begin(),
      fillingDictionaries.m_unresolvedAttributeTypes.end());
  }

  std::string Dictionaries::attributeName(uint32_t code) const
  {
    return attributes().name(code);
  }

  uint32_t Dictionaries::attributeCode(const std::string& name) const
  {
    return attributes().code(name);
  }

  std::string Dictionaries::attributeTypeName(uint8_t code, uint32_t vendor_id) const
  {
    auto it = m_attributeTypes.find(AttributeKey(code, vendor_id));
    return it != m_attributeTypes.end() ? it->second : std::string();
  }

  std::string Dictionaries::vendorName(uint32_t code) const
  {
    return vendorNames().name(code);
  }

  uint32_t Dictionaries::vendorCode(const std::string& name) const
  {
    return vendorNames().code(name);
  }

  std::string Dictionaries::vendorAttributeName(const std::string& vendorName, uint32_t code) const
  {
      return vendorAttributes().name(vendorName, code);
  }

  uint32_t Dictionaries::vendorAttributeCode(const std::string& vendorName,const std::string& name) const
  {
    return vendorAttributes().code(vendorName, name);
  }

  std::string Dictionaries::attributeValueName(const std::string& attributeName, uint32_t code) const
  {
    return attributeValues().name(attributeName, code);
  }

  uint32_t Dictionaries::attributeValueCode(const std::string& attributeName, const std::string& name) const
  {
    return attributeValues().code(attributeName, name);
  }

  std::string Dictionaries::vendorAttributeValueName(const std::string& valueName, uint32_t code) const
  {
    return vendorAttributeValues().name(valueName, code);
  }

  uint32_t Dictionaries::vendorAttributeValueCode(const std::string& valueName, const std::string& name) const
  {
    return vendorAttributeValues().code(valueName, name);
  }

  std::optional<std::string>
  Dictionaries::get_attribute_type(uint8_t code, uint32_t vendor_id) const
  {
    auto it = m_attributeTypes.find(AttributeKey(code, vendor_id));
    if (it != m_attributeTypes.end())
    {
      return it->second;
    }

    return std::nullopt;
  }

  void
  Dictionaries::resolve()
  {
    for (const auto& [unresolved_attr_key, attribute_type] : m_unresolvedAttributeTypes)
    {
      m_attributeTypes.emplace(
        AttributeKey(
          unresolved_attr_key.code,
          m_vendorNames.code(unresolved_attr_key.vendor_name)),
        attribute_type);
    }

    m_unresolvedAttributeTypes.clear();
  }
}
