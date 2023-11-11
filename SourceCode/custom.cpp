#include "custom.h"

std::string custom::Add_Spaces_In_HexString(const std::string str) {
    size_t str_size = str.size();
    std::stringstream result_stream;
    
    for (int index = 0; index < str_size; ++index) {
        result_stream << str[index];
        if ((index + 1) % 2 != 0) continue;
        if (index == str_size - 1) continue;
        result_stream << " ";
    }
    return result_stream.str();
}

std::string custom::Convert_Float_To_LowercaseHexString(const float value) {
    std::uint32_t t;
    std::memcpy(&t, &value, sizeof(value));
    std::stringstream result_stream;
    
    result_stream << std::hex << std::bitset<8*sizeof(uint32_t)>(t).to_ulong();
    return custom::Convert_String_To_Lowercase(result_stream.str());
}

std::string custom::Convert_String_To_Lowercase(std::string str) {
    auto to_lowercase_int = [](unsigned char c) { 
        return std::tolower(c);
    };
    
    std::transform(str.begin(), str.end(), str.begin(), to_lowercase_int);
    return str;
}

// assumes str.size() % 2 == 0;
std::string custom::Swap_HexString_Endian(const std::string str) {
    size_t str_size = str.size();
    std::stringstream result_stream;
    
    for (int index = 0; index < str_size - 1; index += 2) {
        result_stream << str[str_size - 2 - index];
        result_stream << str[str_size - 1 - index];
    }
    return result_stream.str();
}