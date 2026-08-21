#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>

std::string compute_hash(const std::string& data) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) return "";
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1 ||
        EVP_DigestUpdate(ctx, data.c_str(), data.length()) != 1 ||
        EVP_DigestFinal_ex(ctx, hash, &len) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }
    EVP_MD_CTX_free(ctx);
    std::ostringstream ss;
    for (unsigned int i = 0; i < len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

int main() {
    std::string input;
    std::cout << "Enter string to hash: ";
    std::getline(std::cin, input);
    std::cout << "SHA-256: " << compute_hash(input) << std::endl;
    return 0;
}
