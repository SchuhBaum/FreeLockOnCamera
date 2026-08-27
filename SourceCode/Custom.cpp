#include "Custom.h"

std::string Custom::Add_Spaces_To_HexString(const std::string str) {
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

std::string Custom::Aob_At_Address(uintptr_t address, size_t size)
{
    const unsigned char *bytes = reinterpret_cast<const unsigned char *>(address);

    std::ostringstream result_stream;
    result_stream << std::hex << std::setfill('0');
    for (size_t i = 0; i < size; ++i) {
        result_stream << std::setw(2) << static_cast<unsigned int>(bytes[i]);
        if (i + 1 < size) { result_stream << ' '; }
    }

    return result_stream.str();
}

std::string Custom::Aob_From_Number(float number) {
    std::string aob =
        Add_Spaces_To_HexString(
            Swap_HexString_Endian(
                Convert_Float_To_LowercaseHexString(number)));
    return aob;
}

std::string Custom::Convert_Float_To_LowercaseHexString(const float value) {
    std::uint32_t t;
    std::memcpy(&t, &value, sizeof(value));
    std::stringstream result_stream;
    
    result_stream << std::hex << std::bitset<8*sizeof(uint32_t)>(t).to_ulong();
    return Custom::Convert_String_To_Lowercase(result_stream.str());
}

std::vector<unsigned char> Custom::Convert_SpacedHexString_To_UnsignedChar(std::string str) {
    std::istringstream hex_char_stream(str);
    std::vector<unsigned char> bytes;
    bytes.reserve(str.length());
    unsigned int i;

    while (hex_char_stream >> std::hex >> i) {
        bytes.push_back(i);
    }
    return bytes;    
}

std::string Custom::Convert_String_To_Lowercase(std::string str) {
    auto to_lowercase_int = [](unsigned char c) { 
        return std::tolower(c);
    };
    
    std::transform(str.begin(), str.end(), str.begin(), to_lowercase_int);
    return str;
}

// assumes str.size() % 2 == 0;
std::string Custom::Swap_HexString_Endian(const std::string str) {
    size_t str_size = str.size();
    std::stringstream result_stream;
    
    for (int index = 0; index < str_size - 1; index += 2) {
        result_stream << str[str_size - 2 - index];
        result_stream << str[str_size - 1 - index];
    }
    return result_stream.str();
}
