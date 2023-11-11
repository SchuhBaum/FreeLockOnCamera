#pragma once

#include <bitset>
#include <algorithm>
#include <sstream>

namespace custom {
    std::string Add_Spaces_In_HexString(std::string str);
    std::string Convert_Float_To_LowercaseHexString(const float value);
    std::string Convert_String_To_Lowercase(std::string str);
    std::string Swap_HexString_Endian(std::string str);
};