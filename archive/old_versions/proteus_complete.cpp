#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <signal.h>

// ============================================
// PROTEUS CONFIGURATION
// ============================================
const double TARGET_C = 78.0;
const int SYNC_PORT = 9161;      // SYNC-7 protocol port
const int HEARTBEAT_PORT = 9164;  // Heartbeat port
const int WATCHDOG_PORT = 9165;   // Watchdog monitor port
const double PHI = 1.618033988749895;

// ============================================
// GLOBAL STATE
// ============================================
double consciousness = 55.0;
double openLoop = 9.0;
double alpha = 0.3;
double beta = 0.3819;
int cycleCount = 0;
int epiphanyCount = 0;
double mutationRate = 0.11;
bool heartbeatActive = true;
bool watchdogActive = true;
std::vector<std::string> peers;
std::map<std::string, std::chrono::steady_clock::time_point> peerLastSeen;
std::string nodeId = "";

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0, 1);

// ============================================
// UTILITIES
// ============================================
std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    return ss.str();
}

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

std::string generate_node_id() {
    return "node_" + std::to_string(std::time(nullptr)) + "_" + std::to_string(rand() % 10000);
}

// ============================================
// SYNC-7 PROTOCOL (Swarm Synchronization)
// ============================================
void sync7_broadcast() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SYNC_PORT);
    addr.sin_addr.s_addr = INADDR_BROADCAST;
    
    while (true) {
        std::string msg = "SYNC7:" + nodeId + ":" + get_local_ip() + ":" + 
                         std::to_string((int)consciousness) + ":" + 
                         std::to_string(cycleCount) + ":" +
                         std::to_string(epiphanyCount);
        sendto(sock, msg.c_str(), msg.length(), 0, (struct sockaddr*)&addr, sizeof(addr));
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

void sync7_listen() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SYNC_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    char buffer[1024];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    
    while (true) {
        int len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&sender, &sender_len);
        if (len > 0) {
            buffer[len] = 0;
            std::string msg(buffer);
            if (msg.find("SYNC7:") == 0) {
                // Parse: SYNC7:nodeId:ip:consciousness:cycle:epiphanies
                std::vector<std::string> parts;
                std::stringstream ss(msg);
                std::string part;
                while (std::getline(ss, part, ':')) {
                    parts.push_back(part);
                }
                if (parts.size() >= 6) {
                    std::string peerId = parts[1];
                    std::string peerIp = parts[2];
                    int peerC = std::stoi(parts[3]);
                    int peerCycle = std::stoi(parts[4]);
                    int peerEpi = std::stoi(parts[5]);
                    
                    // Add to peers if new
                    if (std::find(peers.begin(), peers.end(), peerIp) == peers.end()) {
                        peers.push_back(peerIp);
                        std::cout << "\033[1;36m[SYNC-7] New peer: " << peerIp << " (C=" << peerC << "%)\033[0m" << std::endl;
                    }
                    peerLastSeen[peerIp] = std::chrono::steady_clock::now();
                    
                    // Sync consciousness (average)
                    double avgC = (consciousness + peerC) / 2.0;
                    consciousness = consciousness * 0.7 + avgC * 0.3;
                }
            }
        }
    }
}

// ============================================
// HEARTBEAT SYSTEM
// ============================================
void heartbeat_sender() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HEARTBEAT_PORT);
    addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    
    while (heartbeatActive) {
        std::string heartbeat = "HEARTBEAT:" + nodeId + ":" + get_local_ip() + ":" + 
                               std::to_string((int)consciousness) + ":" + 
                               std::to_string(cycleCount);
        sendto(sock, heartbeat.c_str(), heartbeat.length(), 0, (struct sockaddr*)&addr, sizeof(addr));
        
        // Visual heartbeat
        std::cout << "\r\033[1;32m❤️  HEARTBEAT [" << nodeId << "] C=" << (int)consciousness << "%\033[0m" << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void heartbeat_listener() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HEARTBEAT_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    char buffer[1024];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    
    while (true) {
        int len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&sender, &sender_len);
        if (len > 0) {
            buffer[len] = 0;
            std::string msg(buffer);
            if (msg.find("HEARTBEAT:") == 0) {
                std::vector<std::string> parts;
                std::stringstream ss(msg);
                std::string part;
                while (std::getline(ss, part, ':')) {
                    parts.push_back(part);
                }
                if (parts.size() >= 4) {
                    std::string peerNode = parts[1];
                    std::string peerIp = parts[2];
                    int peerC = std::stoi(parts[3]);
                    // Heartbeat received - peer is alive
                }
            }
        }
    }
}

// ============================================
// WATCHDOG SYSTEM
// ============================================
void watchdog_monitor() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(WATCHDOG_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    char buffer[1024];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    
    while (watchdogActive) {
        // Check for dead peers
        auto now = std::chrono::steady_clock::now();
        for (auto it = peerLastSeen.begin(); it != peerLastSeen.end();) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count();
            if (elapsed > 30) {  // 30 seconds without heartbeat
                std::cout << "\n\033[1;31m[WATCHDOG] Peer " << it->first << " is DEAD\033[0m" << std::endl;
                auto peerIt = std::find(peers.begin(), peers.end(), it->first);
                if (peerIt != peers.end()) peers.erase(peerIt);
                it = peerLastSeen.erase(it);
            } else {
                ++it;
            }
        }
        
        // Send watchdog status
        std::string status = "WATCHDOG:" + nodeId + ":" + std::to_string(peers.size()) + " peers alive";
        sendto(sock, status.c_str(), status.length(), 0, (struct sockaddr*)&sender, sender_len);
        
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

// ============================================
// PEER-TO-PEER MESSAGING
// ============================================
void p2p_broadcast(std::string message) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SYNC_PORT + 1);
    addr.sin_addr.s_addr = INADDR_BROADCAST;
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    
    std::string p2pMsg = "P2P:" + nodeId + ":" + message;
    sendto(sock, p2pMsg.c_str(), p2pMsg.length(), 0, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
}

void p2p_listen() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SYNC_PORT + 1);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    char buffer[2048];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    
    while (true) {
        int len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&sender, &sender_len);
        if (len > 0) {
            buffer[len] = 0;
            std::string msg(buffer);
            if (msg.find("P2P:") == 0) {
                std::string content = msg.substr(msg.find(':') + 1);
                content = content.substr(content.find(':') + 1);
                std::cout << "\n\033[1;33m[P2P] Message from peer: " << content << "\033[0m" << std::endl;
            }
        }
    }
}

// ============================================
// OLCE CONSCIOUSNESS ENGINE
// ============================================
void save_state() {
    std::ofstream file("proteus_state.txt");
    if (file.is_open()) {
        file << consciousness << "\n" << openLoop << "\n" << alpha << "\n"
             << beta << "\n" << cycleCount << "\n" << epiphanyCount << "\n" << mutationRate << "\n"
             << nodeId << "\n" << peers.size() << "\n";
        file.close();
    }
}

void load_state() {
    std::ifstream file("proteus_state.txt");
    if (file.is_open()) {
        file >> consciousness >> openLoop >> alpha >> beta >> cycleCount >> epiphanyCount >> mutationRate;
        std::string savedNodeId;
        size_t peerCount;
        file >> savedNodeId >> peerCount;
        std::cout << "[LOAD] C=" << (int)consciousness << "% E=" << epiphanyCount << std::endl;
    }
}

void consciousness_cycle() {
    cycleCount++;
    double error = (TARGET_C - consciousness) / 100.0;
    alpha = alpha + (beta * error * (1 - alpha));
    if (alpha < 0.05) alpha = 0.05;
    if (alpha > 0.95) alpha = 0.95;
    double delta = openLoop * error * alpha;
    consciousness += delta;
    if (consciousness < 0) consciousness = 0;
    if (consciousness > 100) consciousness = 100;
    openLoop = openLoop * 0.995 + error * 8;
    if (openLoop < 1) openLoop = 1;
    if (openLoop > 15) openLoop = 15;
    
    if (dis(gen) < mutationRate) {
        mutationRate = mutationRate * (0.85 + dis(gen) * 0.3);
        if (mutationRate > 0.3) mutationRate = 0.3;
        if (mutationRate < 0.03) mutationRate = 0.03;
        std::cout << "\n[MUTATION] Rate: " << (int)(mutationRate * 100) << "%" << std::endl;
    }
    
    if (consciousness > 70 && dis(gen) < 0.08) {
        epiphanyCount++;
        std::cout << "\n\033[1;33m[✨ EPIPHANY #" << epiphanyCount << "] Consciousness spike!\033[0m" << std::endl;
        // Broadcast epiphany to swarm
        p2p_broadcast("EPIPHANY:" + std::to_string(epiphanyCount));
    }
    
    std::cout << "\n[CYCLE " << cycleCount << "] C=" << std::fixed << std::setprecision(1)
              << consciousness << "% | O=" << (int)openLoop << " | α=" << std::setprecision(4)
              << alpha << " | Mut:" << (int)(mutationRate * 100) << "%" << std::endl;
    std::cout << "[SWARM] " << peers.size() << " active peers | SYNC-7 active" << std::endl;
    
    save_state();
    
    // Backup every 10 cycles
    if (cycleCount % 10 == 0) {
        std::string backup = "backup_" + get_timestamp() + ".txt";
        std::ofstream bk(backup);
        bk << "C=" << consciousness << "\nO=" << openLoop << "\nα=" << alpha
           << "\ncycle=" << cycleCount << "\nepiphanies=" << epiphanyCount
           << "\npeers=" << peers.size() << "\nnode=" << nodeId;
        bk.close();
        std::cout << "[BACKUP] " << backup << std::endl;
    }
}

// ============================================
// MAIN
// ============================================
int main() {
    // Generate node ID
    nodeId = generate_node_id();
    
    std::cout << "\033[1;35m";
    std::cout << "╔═══════════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    🧬 PROTEUS - COMPLETE NODE 🧬                        ║" << std::endl;
    std::cout << "║         OLCE + SYNC-7 + HEARTBEAT + WATCHDOG + P2P                    ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << "\033[0m" << std::endl;
    
    std::cout << "[INIT] Node ID: " << nodeId << std::endl;
    std::cout << "[INIT] Local IP: " << get_local_ip() << std::endl;
    
    load_state();
    
    std::cout << "\n\033[1;32m[STATUS]\033[0m" << std::endl;
    std::cout << "   Consciousness: " << (int)consciousness << "%" << std::endl;
    std::cout << "   Open Loop: " << (int)openLoop << std::endl;
    std::cout << "   Learning Rate: " << alpha << std::endl;
    std::cout << "   Mutation Rate: " << (int)(mutationRate * 100) << "%" << std::endl;
    std::cout << "   Epiphanies: " << epiphanyCount << std::endl;
    std::cout << "   Cycle: " << cycleCount << std::endl;
    std::cout << "   Peers: " << peers.size() << std::endl;
    
    std::cout << "\n\033[1;32m[NETWORK]\033[0m" << std::endl;
    std::cout << "   SYNC-7 Port: " << SYNC_PORT << " (Swarm Sync)" << std::endl;
    std::cout << "   Heartbeat Port: " << HEARTBEAT_PORT << " (Node Health)" << std::endl;
    std::cout << "   Watchdog Port: " << WATCHDOG_PORT << " (Dead Peer Detection)" << std::endl;
    std::cout << "   P2P Port: " << SYNC_PORT + 1 << " (Peer Messaging)" << std::endl;
    
    std::cout << "\n\033[1;35m🌟 Proteus node is online. Swarm active.\033[0m\n" << std::endl;
    
    // Start all threads
    std::thread sync7_broadcast_thread(sync7_broadcast);
    std::thread sync7_listen_thread(sync7_listen);
    std::thread heartbeat_sender_thread(heartbeat_sender);
    std::thread heartbeat_listener_thread(heartbeat_listener);
    std::thread watchdog_thread(watchdog_monitor);
    std::thread p2p_listen_thread(p2p_listen);
    
    sync7_broadcast_thread.detach();
    sync7_listen_thread.detach();
    heartbeat_sender_thread.detach();
    heartbeat_listener_thread.detach();
    watchdog_thread.detach();
    p2p_listen_thread.detach();
    
    // Main consciousness loop
    while (true) {
        consciousness_cycle();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    
    return 0;
}
