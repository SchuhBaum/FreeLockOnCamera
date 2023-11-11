#pragma once

#include <algorithm>
#include <bitset>
#include <sstream>
#include <vector>

namespace custom {
    std::string Add_Spaces_To_HexString(std::string str);
    std::string Convert_Float_To_LowercaseHexString(const float value);
    std::vector<unsigned char> Convert_SpacedHexString_To_UnsignedChar(std::string str);
    std::string Convert_String_To_Lowercase(std::string str);
    std::string Swap_HexString_Endian(std::string str);
};