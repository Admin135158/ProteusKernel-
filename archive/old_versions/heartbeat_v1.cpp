#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <algorithm>
#define PORT 9161

std::vector<std::string> peers;

int main() {
    int sock;
    struct sockaddr_in addr;
    int broadcast = 1;
    char buffer[1024];
    
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) return 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &broadcast, sizeof(broadcast));
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_BROADCAST;
    
    std::cout << "[SYNC-7] HEARTBEAT ACTIVE - PORT " << PORT << std::endl;
    
    while(true) {
        // Broadcast presence
        sendto(sock, "PROTEUS_ACTIVE", 14, 0, (struct sockaddr*)&addr, sizeof(addr));
        
        // Listen for peers
        struct sockaddr_in rev_addr;
        socklen_t addr_len = sizeof(rev_addr);
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        int len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&rev_addr, &addr_len);
        if (len > 0) {
            std::string peer_ip = inet_ntoa(rev_addr.sin_addr);
            if (std::find(peers.begin(), peers.end(), peer_ip) == peers.end()) {
                peers.push_back(peer_ip);
                std::cout << "[PEER] " << peer_ip << " JOINED THE SWARM" << std::endl;
            }
        }
        
        // Show swarm status
        if (!peers.empty()) {
            std::cout << "[SWARM] " << peers.size() << " NODES: ";
            for (const auto& p : peers) std::cout << p << " ";
            std::cout << std::endl;
        }
        
        sleep(2);
    }
    return 0;
}
