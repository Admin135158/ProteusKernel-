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
#include <fcntl.h>
#include <vector>

#define PORT 9161
#define BUFFER_SIZE 1024

std::set<std::string> peers;
std::set<std::string> cellular_peers;

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

bool is_cellular(const std::string& ip) {
    return ip.rfind("100.", 0) == 0 || ip.rfind("10.", 0) == 0;
}

void save_peers() {
    std::ofstream f("peers.txt");
    for (const auto& p : peers) f << p << std::endl;
    f.close();
    
    std::ofstream cf("cellular_peers.txt");
    for (const auto& p : cellular_peers) cf << p << std::endl;
    cf.close();
}

void load_peers() {
    std::ifstream f("peers.txt");
    std::string line;
    while (getline(f, line)) {
        if (!line.empty()) peers.insert(line);
        if (is_cellular(line)) cellular_peers.insert(line);
    }
    f.close();
}

void broadcast_heartbeat() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;
    int b = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &b, sizeof(b));
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
    socklen_t len = sizeof(sender);
    while (true) {
        int bytes = recvfrom(sock, buffer, BUFFER_SIZE-1, 0, (struct sockaddr*)&sender, &len);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            std::string msg(buffer);
            if (msg.rfind("PROTEUS_ACTIVE:", 0) == 0) {
                std::string peer = msg.substr(15);
                if (peer != get_local_ip() && peers.find(peer) == peers.end()) {
                    peers.insert(peer);
                    if (is_cellular(peer)) cellular_peers.insert(peer);
                    std::cout << "[PEER] " << peer;
                    if (is_cellular(peer)) std::cout << " (CELLULAR)";
                    std::cout << std::endl;
                    save_peers();
                }
            }
        }
    }
}

void relay_pivot() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(30));
        if (cellular_peers.empty()) continue;
        std::cout << "[RELAY] Attempting TCP tunnels to " << cellular_peers.size() << " cellular peer(s)" << std::endl;
        for (const auto& peer : cellular_peers) {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(PORT + 1);
            inet_pton(AF_INET, peer.c_str(), &addr.sin_addr);
            fcntl(sock, F_SETFL, O_NONBLOCK);
            connect(sock, (struct sockaddr*)&addr, sizeof(addr));
            close(sock);
        }
    }
}

int main() {
    load_peers();
    std::cout << "[HEARTBEAT] SYNC-7 ACTIVE on port " << PORT << std::endl;
    std::cout << "[HEARTBEAT] Local IP: " << get_local_ip() << std::endl;
    std::cout << "[HEARTBEAT] Loaded " << peers.size() << " known peers" << std::endl;
    
    std::thread b(broadcast_heartbeat);
    std::thread l(listen_for_peers);
    std::thread r(relay_pivot);
    b.detach();
    l.detach();
    r.detach();
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(30));
        std::cout << "[SWARM] " << peers.size() << " total peers (" << cellular_peers.size() << " cellular)" << std::endl;
    }
    return 0;
}
