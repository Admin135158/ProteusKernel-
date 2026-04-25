#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <set>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>

#define PORT 9161
#define BUFFER_SIZE 1024

std::set<std::string> peers;

std::string get_local_ip() {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];
    if (getifaddrs(&ifaddr) == -1) return "127.0.0.1";
    std::string ip = "127.0.0.1";
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;
        if (strcmp(ifa->ifa_name, "lo") == 0) continue;
        int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
        if (s == 0) {
            ip = host;
            break;
        }
    }
    freeifaddrs(ifaddr);
    return ip;
}

void broadcast_heartbeat() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_BROADCAST;
    std::string msg = "PROTEUS_ACTIVE:" + get_local_ip();
    while (true) {
        sendto(sock, msg.c_str(), msg.length(), 0, (struct sockaddr*)&addr, sizeof(addr));
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void listen_for_peers() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    char buffer[BUFFER_SIZE];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    while (true) {
        int bytes = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&sender, &sender_len);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            std::string msg(buffer);
            if (msg.rfind("PROTEUS_ACTIVE:", 0) == 0) {
                std::string peer_ip = msg.substr(15);
                if (peer_ip != get_local_ip() && peers.find(peer_ip) == peers.end()) {
                    peers.insert(peer_ip);
                    std::cout << "[PEER] " << peer_ip << " DETECTED" << std::endl;
                    std::ofstream f;
                    f.open("peers.txt", std::ios::app);
                    f << peer_ip << std::endl;
                    f.close();
                }
            }
        }
    }
}

int main() {
    std::cout << "[HEARTBEAT] SYNC-7 ACTIVE on port " << PORT << std::endl;
    std::cout << "[HEARTBEAT] Local IP: " << get_local_ip() << std::endl;
    
    std::thread b(broadcast_heartbeat);
    std::thread l(listen_for_peers);
    b.detach();
    l.detach();
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        if (!peers.empty()) {
            std::cout << "[SWARM] " << peers.size() << " peer(s): ";
            for (const auto& p : peers) std::cout << p << " ";
            std::cout << std::endl;
        }
    }
    return 0;
}
