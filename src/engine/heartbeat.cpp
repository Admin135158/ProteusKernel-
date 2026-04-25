#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <thread>
#include <chrono>
#include <mutex>

#define PORT 9161

std::mutex cout_mutex;  // for safe printing

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

void broadcast() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
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

void listen() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    char buffer[1024];
    struct sockaddr_in sender;
    socklen_t len = sizeof(sender);
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));               // clear buffer before use
        int bytes = recvfrom(sock, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&sender, &len);
        if (bytes > 0) {
            std::string msg(buffer, bytes);              // construct from exact bytes
            if (msg.rfind("PROTEUS_ACTIVE:", 0) == 0) {
                std::string peer = msg.substr(15);
                {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "[PEER] " << peer << std::endl;
                }
            }
        }
    }
}

int main() {
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[HEARTBEAT] SYNC-7 on port " << PORT << std::endl;
        std::cout << "[HEARTBEAT] IP: " << get_local_ip() << std::endl;
    }
    std::thread b(broadcast);
    std::thread l([](){ listen(); });   // lambda fixes overload ambiguity
    b.detach();
    l.detach();
    while (true) std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}
