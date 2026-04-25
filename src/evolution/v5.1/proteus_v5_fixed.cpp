#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <random>
#include <vector>
#include <string>
#include <cstring>
#include <regex>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/stat.h>

namespace fs = std::filesystem;

// ============================================
// CONFIGURATION - MUTABLE PARAMETERS
// ============================================
int O = 9;                          // Sacred number (can mutate)
const double PHI = 1.618033988749895;
const int PORT = 9161;
int MUTATION_RATE = 10;             // Percent chance to mutate (can mutate)
int SLEEP_SECONDS = 5;              // Cycle interval (can mutate)

// Global RNG
std::random_device rd;
std::mt19937 gen(rd());

// ============================================
// UTILITY FUNCTIONS
// ============================================

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << content;
    return true;
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

// ============================================
// SELF-MODIFICATION ENGINE
// ============================================

void mutate_self() {
    std::cout << "[MUTATE] Evolving kernel..." << std::endl;
    
    std::string source_file = "proteus_v5_fixed.cpp";
    std::string source = read_file(source_file);
    
    if (source.empty()) {
        std::cout << "[MUTATE] Cannot read source" << std::endl;
        return;
    }
    
    std::string mutated = source;
    
    // 1. Mutate O constant (-2 to +2)
    std::uniform_int_distribution<> o_dis(-2, 2);
    int o_delta = o_dis(gen);
    int new_O = O + o_delta;
    if (new_O < 1) new_O = 1;
    if (new_O > 100) new_O = 100;
    
    if (new_O != O) {
        std::regex o_pattern("int O = ([0-9]+);");
        std::smatch match;
        if (std::regex_search(mutated, match, o_pattern)) {
            std::string old_str = match[0].str();
            std::string new_str = "int O = " + std::to_string(new_O) + ";";
            size_t pos = mutated.find(old_str);
            if (pos != std::string::npos) {
                mutated.replace(pos, old_str.length(), new_str);
                std::cout << "[MUTATE] O: " << O << " → " << new_O << std::endl;
                O = new_O;
            }
        }
    }
    
    // 2. Mutate mutation rate (±5%)
    std::uniform_int_distribution<> rate_dis(-5, 5);
    int rate_delta = rate_dis(gen);
    int new_rate = MUTATION_RATE + rate_delta;
    if (new_rate < 1) new_rate = 1;
    if (new_rate > 50) new_rate = 50;
    
    if (new_rate != MUTATION_RATE) {
        std::regex rate_pattern("int MUTATION_RATE = ([0-9]+);");
        std::smatch match;
        if (std::regex_search(mutated, match, rate_pattern)) {
            std::string old_str = match[0].str();
            std::string new_str = "int MUTATION_RATE = " + std::to_string(new_rate) + ";";
            size_t pos = mutated.find(old_str);
            if (pos != std::string::npos) {
                mutated.replace(pos, old_str.length(), new_str);
                std::cout << "[MUTATE] Mutation rate: " << MUTATION_RATE << "% → " << new_rate << "%" << std::endl;
                MUTATION_RATE = new_rate;
            }
        }
    }
    
    // 3. Mutate sleep interval (±1 second)
    std::uniform_int_distribution<> sleep_dis(-1, 1);
    int sleep_delta = sleep_dis(gen);
    int new_sleep = SLEEP_SECONDS + sleep_delta;
    if (new_sleep < 1) new_sleep = 1;
    if (new_sleep > 30) new_sleep = 30;
    
    if (new_sleep != SLEEP_SECONDS) {
        std::regex sleep_pattern("int SLEEP_SECONDS = ([0-9]+);");
        std::smatch match;
        if (std::regex_search(mutated, match, sleep_pattern)) {
            std::string old_str = match[0].str();
            std::string new_str = "int SLEEP_SECONDS = " + std::to_string(new_sleep) + ";";
            size_t pos = mutated.find(old_str);
            if (pos != std::string::npos) {
                mutated.replace(pos, old_str.length(), new_str);
                std::cout << "[MUTATE] Sleep: " << SLEEP_SECONDS << "s → " << new_sleep << "s" << std::endl;
                SLEEP_SECONDS = new_sleep;
            }
        }
    }
    
    // If nothing changed, don't recompile
    if (mutated == source) {
        std::cout << "[MUTATE] No changes applied" << std::endl;
        return;
    }
    
    // Write mutated source
    std::string mutated_file = "mutated_v5.cpp";
    if (!write_file(mutated_file, mutated)) {
        std::cout << "[MUTATE] Cannot write mutated source" << std::endl;
        return;
    }
    
    std::cout << "[MUTATE] Compiling new kernel..." << std::endl;
    
    // Compile new version
    std::string compile_cmd = "clang++ -std=c++17 " + mutated_file + " -o proteus_v5.new -pthread 2>&1";
    int result = system(compile_cmd.c_str());
    
    if (result != 0) {
        std::cout << "[MUTATE] Compilation failed" << std::endl;
        return;
    }
    
    // Check new binary exists
    struct stat st;
    if (stat("proteus_v5.new", &st) != 0 || st.st_size == 0) {
        std::cout << "[MUTATE] New binary invalid" << std::endl;
        return;
    }
    
    std::cout << "[MUTATE] New kernel ready. Restarting..." << std::endl;
    
    // Replace current binary
    rename("proteus_v5_fixed", "proteus_v5_fixed.old");
    rename("proteus_v5.new", "proteus_v5_fixed");
    
    // Replace current process with new kernel
    execl("./proteus_v5_fixed", "proteus_v5_fixed", NULL);
}

// ============================================
// REPLICATION ENGINE
// ============================================

void replicate() {
    std::string exe_path = fs::current_path().string() + "/proteus_v5_fixed";
    std::string backup_path = "backup_" + std::to_string(std::time(nullptr));
    if (fs::exists(exe_path)) {
        fs::copy_file(exe_path, backup_path, fs::copy_options::overwrite_existing);
        std::cout << "[REPLICATE] Created: " << backup_path << std::endl;
    }
}

// ============================================
// SYNC-7 NETWORKING
// ============================================

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
    while (true) {
        int len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&sender, &sender_len);
        if (len > 0) {
            buffer[len] = '\0';
            std::string msg(buffer);
            if (msg.rfind("PROTEUS_ACTIVE:", 0) == 0) {
                std::string peer_ip = msg.substr(15);
                if (peer_ip != get_local_ip()) {
                    std::cout << "[PEER] " << peer_ip << std::endl;
                    std::ofstream peer_file("peers.txt", std::ios::app);
                    peer_file << peer_ip << std::endl;
                }
            }
        }
    }
}

// ============================================
// MAIN KERNEL LOOP
// ============================================

int main() {
    std::cout << "\033[1;31m";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║     🧬 PROTEUS KERNEL v5.0 - SELF-MODIFYING             ║\n";
    std::cout << "║     \"Code that writes itself. Genes that fight to live.\"║\n";
    std::cout << "║                    - The Architect                        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << "\033[0m\n";
    std::cout << "[INIT] Local IP: " << get_local_ip() << std::endl;
    std::cout << "[INIT] O=" << O << " φ=" << PHI << std::endl;
    std::cout << "[INIT] Mutation rate: " << MUTATION_RATE << "%" << std::endl;
    std::cout << "[INIT] Cycle interval: " << SLEEP_SECONDS << "s" << std::endl;
    std::cout << "[INIT] SYNC-7 ACTIVE on port " << PORT << std::endl;
    
    // Start network threads
    std::thread broadcast_thread(broadcast_presence);
    std::thread listener_thread(listen_for_peers);
    broadcast_thread.detach();
    listener_thread.detach();
    
    int cycle = 0;
    std::uniform_int_distribution<> mutate_dis(0, 100);
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_SECONDS));
        
        replicate();
        
        // Check if we should mutate
        if (mutate_dis(gen) < MUTATION_RATE) {
            mutate_self();
            // mutate_self calls execl, so we never return here
        }
        
        cycle++;
        std::cout << "[CYCLE " << cycle << "] Alive. O=" << O << " φ=" << PHI 
                  << " | Next mutation in " << MUTATION_RATE << "%" << std::endl;
    }
    
    return 0;
}
