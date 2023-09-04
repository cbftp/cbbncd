#include "crypto.h"

#include <climits>
#include <ctime>
#include <cstring>
#include <openssl/sha.h>
#include <openssl/evp.h>

#define SALT_LENGTH 8
#define SALT_STRING_LENGTH 16

namespace {

const EVP_CIPHER* cipher() {
  return EVP_aes_256_cbc();
}

const EVP_MD* digest() {
  return EVP_sha256();
}

int blockSize() {
  return EVP_CIPHER_block_size(cipher());
}

}

void Crypto::encrypt(const Core::BinaryData& indata, const Core::BinaryData& pass, Core::BinaryData& outdata) {
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  const EVP_CIPHER* cipherp = cipher();
  int ivlen = EVP_CIPHER_iv_length(cipherp);
  int keylen = EVP_CIPHER_key_length(cipherp);
  unsigned char* key = (unsigned char*) malloc(keylen);
  unsigned char* iv = (unsigned char*) malloc(ivlen);
  outdata.resize(SALT_STRING_LENGTH + indata.size() + blockSize());
  memcpy(&outdata[0], "Salted__", SALT_STRING_LENGTH - SALT_LENGTH);
  srand(time(NULL));
  for (int i = SALT_LENGTH; i < SALT_STRING_LENGTH; ++i) {
    outdata[i] = (unsigned char)(rand() % UCHAR_MAX);
  }
  EVP_BytesToKey(cipherp, digest(), &outdata[SALT_STRING_LENGTH - SALT_LENGTH], &pass[0],
                 pass.size(), 1, key, iv);
  int resultlen;
  int finallen;
  EVP_EncryptInit_ex(ctx, cipherp, NULL, key, iv);
  EVP_EncryptUpdate(ctx, &outdata[SALT_STRING_LENGTH], &resultlen, &indata[0], indata.size());
  EVP_EncryptFinal_ex(ctx, &outdata[SALT_STRING_LENGTH + resultlen], &finallen);
  outdata.resize(SALT_STRING_LENGTH + resultlen + finallen);
  EVP_CIPHER_CTX_free(ctx);
  free(key);
  free(iv);
}

void Crypto::decrypt(const Core::BinaryData& indata, const Core::BinaryData& pass, Core::BinaryData& outdata) {
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  const EVP_CIPHER* cipherp = cipher();
  int ivlen = EVP_CIPHER_iv_length(cipherp);
  int keylen = EVP_CIPHER_key_length(cipherp);
  unsigned char* key = (unsigned char*) malloc(keylen);
  unsigned char* iv = (unsigned char*) malloc(ivlen);
  EVP_BytesToKey(cipherp, digest(), &indata[SALT_STRING_LENGTH - SALT_LENGTH], &pass[0],
                 pass.size(), 1, key, iv);
  outdata.resize(indata.size() + blockSize());
  int writelen;
  int finalwritelen;
  EVP_DecryptInit_ex(ctx, cipherp, NULL, key, iv);
  EVP_DecryptUpdate(ctx, &outdata[0], &writelen, &indata[SALT_STRING_LENGTH],
                    indata.size() - SALT_STRING_LENGTH);
  EVP_DecryptFinal_ex(ctx, &outdata[writelen], &finalwritelen);
  outdata.resize(writelen + finalwritelen);
  EVP_CIPHER_CTX_free(ctx);
  free(key);
  free(iv);
}

void Crypto::sha256(const Core::BinaryData& indata, Core::BinaryData& outdata) {
  outdata.resize(SHA256_DIGEST_LENGTH);
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, &indata[0], indata.size());
  SHA256_Final(&outdata[0], &ctx);
}

void Crypto::base64Encode(const Core::BinaryData& indata, Core::BinaryData& outdata) {
  outdata.resize((indata.size() / 3 + ((indata.size() % 3) ? 1 : 0)) * 4 + 1);
  int bytes = EVP_EncodeBlock(&outdata[0], &indata[0], indata.size());
  outdata.resize(bytes);
}

void Crypto::base64Decode(const Core::BinaryData& indata, Core::BinaryData& outdata) {
  int pos = indata.size();
  int padding = 0;
  while (--pos >= 0 && indata[pos] == '=') {
    ++padding;
  }
  unsigned int outsize = indata.size() / 4 * 3;
  outdata.resize(outsize);
  EVP_DecodeBlock(&outdata[0], &indata[0], indata.size());
  outdata.resize(outsize - padding);
}
