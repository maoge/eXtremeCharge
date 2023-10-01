#pragma once

#include <string>

namespace ZZTools
{

class AesEncryptor
{
public:
    AesEncryptor(unsigned char* key);
    ~AesEncryptor(void);

    std::string EncryptString(std::string strInfor);
    std::string DecryptString(std::string strMessage);

private:
    void Byte2Hex(const unsigned char* src, int len, char* dest);
    void Hex2Byte(const char* src, int len, unsigned char* dest);
    int  Char2Int(char c);

private:
    unsigned char* m_pKey;

};

}
