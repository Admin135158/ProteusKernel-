#include <iostream>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <string>

std::string sha256(const std::string &data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)data.c_str(), data.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << std::hex << std::setw(2) << std::setfill('0')
           << (int)hash[i];
    return ss.str();
}

int main() {
    std::cout << "[GOTEM] SHA-256 Engine Active" << std::endl;

    int n = 0;
    while (true) {
        std::string msg = "ProteusKernel_" + std::to_string(n++);
        std::cout << "[GOTEM] " << sha256(msg) << std::endl;
    }
}
