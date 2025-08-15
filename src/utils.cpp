#include "utils.h"
#include "attribute_types.h"
#include <cstdint> //uint8_t, uint32_t

std::string radius_lite::byteToHex(uint8_t byte)
{
    static const std::string digits = "0123456789ABCDEF";
    return {digits[byte / 16], digits[byte % 16]};
}
