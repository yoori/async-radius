#pragma once

#include "attribute_types.h"
#include "attribute.h"
#include "vendor_attribute.h"

radius_lite::Attribute*
findAttribute(const std::vector<radius_lite::Attribute*>& attributes, radius_lite::Attribute_Types type);

