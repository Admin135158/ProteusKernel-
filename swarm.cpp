#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <vector>

#define SWARM_PORT 9163
#define CHUNK_SIZE 1024

class SwarmNode {
private:
    int sockfd;
    struct sockaddr_in addr;
    
public:
    SwarmNode() {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) { perror("socket"); exit(1); }
    }
    
    void pushFile(const std::string& filename, const std::string& target_ip) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[ERROR] Cannot open: " << filename << std::endl;
            return;
        }
        
        std::vector<char> data((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
        file.close();
        
        std::string encoded = base64_encode(data.data(), data.size());
        
        std::cout << "[PUSH] " << filename << " → " << target_ip << std::endl;
        std::cout << "       Size: " << data.size() << " bytes | Base64: " 
                  << encoded.size() << " chars" << std::endl;
        
        struct sockaddr_in target;
        target.sin_family = AF_INET;
        target.sin_port = htons(SWARM_PORT);
        inet_pton(AF_INET, target_ip.c_str(), &target.sin_addr);
        
        if (connect(sockfd, (struct sockaddr*)&target, sizeof(target)) < 0) {
            perror("connect failed");
            return;
        }
        
        for (size_t i = 0; i < encoded.size(); i += CHUNK_SIZE) {
            size_t len = std::min(static_cast<size_t>(CHUNK_SIZE), encoded.size() - i);
            send(sockfd, encoded.c_str() + i, len, 0);
        }
        
        close(sockfd);
    }
    
    std::string base64_encode(const char* data, size_t len) {
        return "[BASE64_ENCODED_DATA]";
    }
};

int main() {
    SwarmNode swarm;
    return 0;
}
