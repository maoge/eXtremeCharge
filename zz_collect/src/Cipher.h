#pragma once
#include <string>
#include <string.h>

#define BLOCK_SIZE 16
 
namespace ZZ_TOOLS
{

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

typedef struct
{
    uint32 erk[64];     /* encryption round keys */
    uint32 drk[64];     /* decryption round keys */
    int nr;             /* number of rounds */
} aes_context;

int base64_encode(const unsigned char *src, int src_bytes, char *out);
int base64_decode(const char *in, int in_bytes, unsigned char *out);

class AES_TOOL
{
public:
    static int  aes_set_key( aes_context *ctx, uint8 *key, int nbits );
    static void aes_encrypt( aes_context *ctx, uint8 input[16], uint8 output[16] );
    static void aes_decrypt( aes_context *ctx, uint8 input[16], uint8 output[16] );

    static std::string encrypt(const std::string &plaintext, const std::string &skey);
    static std::string decrypt(const std::string &basestr, const std::string &skey);

    static std::string encrypt_cbc(const std::string &plaintext, const std::string &skey, const std::string &siv);
    static std::string decrypt_cbc(const std::string &basestr, const std::string &skey, const std::string &siv);
};

class RSA_TOOL
{
public:
    // static void gen_rsa_key(std::string & out_pub_key, std::string & out_pri_key);

    static std::string rsa_pri_encrypt(const std::string &clear_text, std::string &pri_key);
    static std::string rsa_pub_decrypt(const std::string & cipher_text, const std::string & pub_key);

    static std::string rsa_pub_encrypt(const std::string &clear_text, const std::string &pub_key);
    static std::string rsa_pri_decrypt(const std::string &cipher_text, const std::string &pri_key);
};

}  // end of ZZ_TOOLS
