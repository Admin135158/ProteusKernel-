#include <iostream>
#include <string>

std::string toDNA(unsigned char b) {
    const char* map = "ACGT";
    std::string out;
    out += map[(b >> 6) & 3];
    out += map[(b >> 4) & 3];
    out += map[(b >> 2) & 3];
    out += map[b & 3];
    return out;
}

int main() {
    std::cout << "[DNA-BINARY] Encoding stream..." << std::endl;

    int n = 0;
    while (true) {
        unsigned char b = (unsigned char)(n++ & 0xFF);
        std::cout << "[DNA] " << toDNA(b) << std::endl;
    }
}
