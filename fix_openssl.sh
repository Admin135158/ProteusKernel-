#!/bin/bash
sed -i 's/SHA256_Init(&sha256)/EVP_DigestInit_ex(ctx, EVP_sha256(), NULL)/' src/pk_gotem.cpp
sed -i 's/SHA256_Update(&sha256, (const unsigned char\*)/EVP_DigestUpdate(ctx, /' src/pk_gotem.cpp
sed -i 's/SHA256_Final(hash, &sha256)/EVP_DigestFinal_ex(ctx, hash, NULL)/' src/pk_gotem.cpp
sed -i 's/ SHA256_CTX sha256;/ EVP_MD_CTX *ctx = EVP_MD_CTX_new();/' src/pk_gotem.cpp
