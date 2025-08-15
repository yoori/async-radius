#include "utils.h"

radius_lite::Attribute*
findAttribute(const std::vector<radius_lite::Attribute*>& attributes, radius_lite::Attribute_Types type)
{
  for (const auto& b : attributes)
  {
    if (b->type() == type)
    {
      return b;
    }
  }
  return nullptr;
}
