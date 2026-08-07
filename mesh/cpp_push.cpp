#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 9163
#define CHUNK 1024

class CppPush {
public:
    void push(const std::string& file, const std::string& ip) {
        std::ifstream f(file, std::ios::binary);
        if (!f.is_open()) { std::cerr << "Cannot open " << file << std::endl; return; }
        
        std::vector<char> data((std::istreambuf_iterator<char>(f)),
                                std::istreambuf_iterator<char>());
        f.close();
        
        std::string encoded;
        for (char c : data) {
            char buf[3];
            sprintf(buf, "%02x", (unsigned char)c);
            encoded += buf;
        }
        
        std::cout << "[PUSH] " << file << " → " << ip << std::endl;
        std::cout << "       Size: " << data.size() << " bytes | Hex: " 
                  << encoded.size() << " chars" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./cpp_push [file] [ip]" << std::endl;
        return 1;
    }
    CppPush pusher;
    pusher.push(argv[1], argv[2]);
    return 0;
}
