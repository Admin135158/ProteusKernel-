#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>

#define PORT 9162

class RemoteControl {
public:
    void sendCommand(const std::string& cmd, const std::string& target_ip = "127.0.0.1") {
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) { perror("socket"); return; }
        
        struct sockaddr_in target;
        target.sin_family = AF_INET;
        target.sin_port = htons(PORT);
        inet_pton(AF_INET, target_ip.c_str(), &target.sin_addr);
        
        sendto(sockfd, cmd.c_str(), cmd.length(), MSG_CONFIRM,
               (const struct sockaddr*)&target, sizeof(target));
        
        char buffer[4096];
        struct sockaddr_in from;
        socklen_t from_len = sizeof(from);
        int n = recvfrom(sockfd, buffer, 4096, MSG_WAITALL,
                         (struct sockaddr*)&from, &from_len);
        buffer[n] = '\0';
        std::cout << buffer << std::endl;
        
        close(sockfd);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: remote_control [command] [ip]" << std::endl;
        return 1;
    }
    
    RemoteControl rc;
    std::string ip = (argc > 2) ? argv[2] : "127.0.0.1";
    rc.sendCommand(argv[1], ip);
    return 0;
}
