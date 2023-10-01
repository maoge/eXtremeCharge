#pragma once

#include <string>

namespace ZZTools
{

class AesUtils
{
public:
    static std::string encrypt(const std::string& str_to_encrypt);
    static std::string encrypt(const std::string& str_to_encrypt, const std::string& key);

    static std::string decrypt(const std::string& str_to_decrypt);
    static std::string decrypt(const std::string& str_to_decrypt, const std::string& key);
    
};

}
