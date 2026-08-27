#pragma once

#include "Include/ModUtils.h"

#include <algorithm>
#include <bitset>
#include <sstream>
#include <vector>

using namespace ModUtils;

namespace Custom {
    std::string Add_Spaces_To_HexString(std::string str);
    std::string Aob_At_Address(uintptr_t address, size_t size);
    std::string Aob_From_Number(float number);
    std::string Convert_Float_To_LowercaseHexString(const float value);
    std::vector<unsigned char> Convert_SpacedHexString_To_UnsignedChar(std::string str);
    std::string Convert_String_To_Lowercase(std::string str);
    std::string String_From_Bytes_At(uintptr_t address, size_t size);
    std::string Swap_HexString_Endian(std::string str);

    template<typename T>
    std::string Aob_From_Number(T number) {
        std::string aob =
            Add_Spaces_To_HexString(
                Swap_HexString_Endian(
                    NumberToHexString(number)));
        return aob;
    }
};
