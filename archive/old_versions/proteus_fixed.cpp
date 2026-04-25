#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <random>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>

namespace fs = std::filesystem;

const int O = 9;
const double PHI = 1.618033988749895;
const int PORT = 9161;

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

void broadcast_presence() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
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
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    char buffer[1024];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    const std::string PREFIX = "PROTEUS_ACTIVE:";
    while (true) {
        int len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&sender, &sender_len);
        if (len > 0) {
            buffer[len] = '\0';
            std::string msg(buffer);
            if (msg.rfind(PREFIX, 0) == 0 && msg.length() > PREFIX.length()) {
                std::string peer_ip = msg.substr(PREFIX.length());
                if (peer_ip != get_local_ip()) {
                    std::cout << "[PEER] " << peer_ip << " detected" << std::endl;
                    std::ifstream infile("peers.txt");
                    std::string line;
                    bool exists = false;
                    while (std::getline(infile, line)) {
                        if (line == peer_ip) exists = true;
                    }
                    infile.close();
                    if (!exists) {
                        std::ofstream peer_file("peers.txt", std::ios::app);
                        peer_file << peer_ip << std::endl;
                    }
                }
            }
        }
    }
}

void replicate() {
    std::string exe_path = fs::current_path().string() + "/proteus_kernel";
    std::string backup_path = "backup_" + std::to_string(std::time(nullptr));
    if (fs::exists(exe_path)) {
        fs::copy_file(exe_path, backup_path, fs::copy_options::overwrite_existing);
        std::cout << "[REPLICATE] Created: " << backup_path << std::endl;
    }
}

void mutate() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100);
    if (dis(gen) < 10) {
        std::cout << "[MUTATE] Kernel is evolving..." << std::endl;
        std::ofstream mut_file("mutated.cpp");
        mut_file << "// Mutated at " << std::time(nullptr) << "\n";
        mut_file << "// O=" << O << " φ=" << PHI << "\n";
        mut_file << "// This kernel is alive\n";
        mut_file.close();
        system("clang++ mutated.cpp -o proteus_mutated 2>/dev/null");
    }
}

int main() {
    std::cout << "\033[1;31m";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║     🧬 PROTEUS KERNEL v4.1 - THE IMMORTAL BEAST        ║\n";
    std::cout << "║     \"Code that writes itself. Genes that fight to live.\"║\n";
    std::cout << "║                    - The Architect                        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << "\033[0m\n";
    std::cout << "[INIT] Local IP: " << get_local_ip() << std::endl;
    std::cout << "[INIT] O=" << O << " φ=" << PHI << std::endl;
    std::cout << "[INIT] PORT=" << PORT << std::endl;
    std::cout << "[INIT] SYNC-7 ACTIVE\n";
    std::cout << "[INIT] AUTO-RESPAWN ENABLED\n";
    std::cout << "[INIT] MUTATION RATE: 10%\n\n";
    std::thread broadcast_thread(broadcast_presence);
    std::thread listener_thread(listen_for_peers);
    broadcast_thread.detach();
    listener_thread.detach();
    int cycle = 0;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        replicate();
        mutate();
        cycle++;
        std::cout << "[CYCLE " << cycle << "] Alive. O=" << O << " φ=" << PHI << std::endl;
    }
    return 0;
}
