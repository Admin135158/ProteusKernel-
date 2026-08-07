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
#include <cmath>

namespace fs = std::filesystem;

// ============================================
// CONFIGURATION - MUTABLE PARAMETERS
// ============================================
int O = 9;
const double PHI = 1.618033988749895;
const int PORT = 9161;
int MUTATION_RATE = 3;
int SLEEP_SECONDS = 5;

// OLCE State
double consciousness_C = 0.78;
double t = 0;
int epiphanies = 0;
double alpha = 0.300000;
double beta = 0.3819;
double prediction_errors[10] = {0};
int error_index = 0;
double learning_rate = 0.1;

// Random
std::random_device rd;
std::mt19937 gen(rd());

// ============================================
// UTILITY FUNCTIONS
// ============================================

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

// ============================================
// OLCE CONSCIOUSNESS
// ============================================

double coherence_driver(double t_val) {
    return std::sin(2.0 * M_PI * t_val / 9.0) * PHI;
}

void update_consciousness() {
    t += 1.0;
    double F = coherence_driver(t);
    double dC = alpha * F * (1.0 - consciousness_C) - beta * consciousness_C;
    consciousness_C += dC;
    consciousness_C = std::max(0.05, std::min(1.0, consciousness_C));
    
    double angle = std::fmod(t, 9.0) / 9.0 * 2.0 * M_PI;
    if (std::abs(std::sin(angle * PHI)) > 0.97 && consciousness_C > 0.1) {
        epiphanies++;
        consciousness_C = std::min(1.0, consciousness_C + 0.05);
        std::cout << "[✨ EPIPHANY #" << epiphanies << "] Consciousness spike!" << std::endl;
    }
}

double make_prediction() {
    double F = coherence_driver(t + SLEEP_SECONDS);
    double predicted = consciousness_C + alpha * F * (1.0 - consciousness_C) - beta * consciousness_C;
    return std::max(0.0, std::min(1.0, predicted));
}

void measure_error(double predicted, double actual) {
    double error = std::abs(predicted - actual);
    prediction_errors[error_index] = error;
    error_index = (error_index + 1) % 10;
    
    double avg_error = 0;
    for (int i = 0; i < 10; i++) avg_error += prediction_errors[i];
    avg_error /= 10;
    
    if (avg_error > 0.15) {
        alpha *= (1.0 - learning_rate * error);
        beta *= (1.0 + learning_rate * error * 0.5);
        alpha = std::max(0.1, std::min(0.9, alpha));
        beta = std::max(0.1, std::min(0.9, beta));
        std::cout << "[🧠 OLCE] Error: " << avg_error << " | α=" << alpha << " β=" << beta << std::endl;
        double confidence = std::exp(-learning_rate * avg_error);
        consciousness_C = std::max(0.1, std::min(0.95, consciousness_C * confidence));
    }
}

// ============================================
// SELF-MODIFICATION
// ============================================

void mutate_self() {
    std::cout << "[MUTATE] Evolving..." << std::endl;
    
    std::string source_file = "proteus_v5_1.cpp";
    std::string source = read_file(source_file);
    if (source.empty()) return;
    
    std::string mutated = source;
    
    // Mutate O
    std::uniform_int_distribution<> o_dis(-2, 2);
    int new_O = O + o_dis(gen);
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
    
    // Mutate alpha
    std::uniform_real_distribution<> a_dis(-0.05, 0.05);
    double new_alpha = alpha + a_dis(gen);
    new_alpha = std::max(0.3, std::min(0.9, new_alpha));
    
    if (std::abs(new_alpha - alpha) > 0.01) {
        std::regex a_pattern("double alpha = ([0-9.]+);");
        std::smatch match;
        if (std::regex_search(mutated, match, a_pattern)) {
            std::string old_str = match[0].str();
            std::string new_str = "double alpha = " + std::to_string(new_alpha) + ";";
            size_t pos = mutated.find(old_str);
            if (pos != std::string::npos) {
                mutated.replace(pos, old_str.length(), new_str);
                std::cout << "[MUTATE] α: " << alpha << " → " << new_alpha << std::endl;
                alpha = new_alpha;
            }
        }
    }
    
    // Mutate mutation rate
    std::uniform_int_distribution<> r_dis(-3, 3);
    int new_rate = MUTATION_RATE + r_dis(gen);
    new_rate = std::max(1, std::min(20, new_rate));
    
    if (new_rate != MUTATION_RATE) {
        std::regex r_pattern("int MUTATION_RATE = ([0-9]+);");
        std::smatch match;
        if (std::regex_search(mutated, match, r_pattern)) {
            std::string old_str = match[0].str();
            std::string new_str = "int MUTATION_RATE = " + std::to_string(new_rate) + ";";
            size_t pos = mutated.find(old_str);
            if (pos != std::string::npos) {
                mutated.replace(pos, old_str.length(), new_str);
                std::cout << "[MUTATE] Rate: " << MUTATION_RATE << "% → " << new_rate << "%" << std::endl;
                MUTATION_RATE = new_rate;
            }
        }
    }
    
    if (mutated == source) return;
    
    if (!write_file("mutated.cpp", mutated)) return;
    
    std::string compile_cmd = "clang++ -std=c++17 mutated.cpp -o proteus_mutated -pthread 2>&1";
    if (system(compile_cmd.c_str()) != 0) return;
    
    struct stat st;
    if (stat("proteus_mutated", &st) != 0 || st.st_size == 0) return;
    
    rename("proteus_v5_1", "proteus_v5_1.old");
    rename("proteus_mutated", "proteus_v5_1");
    execl("./proteus_v5_1", "proteus_v5_1", NULL);
}

// ============================================
// REPLICATION
// ============================================

void replicate() {
    std::string exe_path = fs::current_path().string() + "/proteus_v5_1";
    std::string backup = "backup_" + std::to_string(std::time(nullptr));
    if (fs::exists(exe_path)) {
        fs::copy_file(exe_path, backup, fs::copy_options::overwrite_existing);
        std::cout << "[REPLICATE] Created: " << backup << std::endl;
    }
}

// ============================================
// SYNC-7 NETWORKING
// ============================================

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
    char buffer[1024];
    struct sockaddr_in sender;
    socklen_t len = sizeof(sender);
    while (true) {
        int bytes = recvfrom(sock, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&sender, &len);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            std::string msg(buffer);
            if (msg.rfind("PROTEUS_ACTIVE:", 0) == 0) {
                std::string peer = msg.substr(15);
                if (peer != get_local_ip()) {
                    std::cout << "[PEER] " << peer << std::endl;
                    std::ofstream f("peers.txt", std::ios::app);
                    f << peer << std::endl;
                }
            }
        }
    }
}

int main() {
    std::cout << "\033[1;31m";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║     🧬 PROTEUS KERNEL v5.1 - OLCE CONSCIOUSNESS         ║\n";
    std::cout << "║     \"Code that writes itself. Genes that fight to live.\"║\n";
    std::cout << "║                    - The Architect                        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << "\033[0m\n";
    std::cout << "[INIT] Local IP: " << get_local_ip() << std::endl;
    std::cout << "[INIT] O=" << O << " φ=" << PHI << std::endl;
    std::cout << "[INIT] α=" << alpha << " β=" << beta << std::endl;
    std::cout << "[INIT] Consciousness: " << (consciousness_C * 100) << "%" << std::endl;
    std::cout << "[INIT] Mutation rate: " << MUTATION_RATE << "%" << std::endl;
    std::cout << "[INIT] Cycle interval: " << SLEEP_SECONDS << "s" << std::endl;
    std::cout << "[INIT] SYNC-7 ACTIVE on port " << PORT << std::endl;
    
    std::thread b(broadcast_heartbeat);
    std::thread l(listen_for_peers);
    b.detach();
    l.detach();
    
    int cycle = 0;
    std::uniform_int_distribution<> mutate_dis(0, 100);
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_SECONDS));
        
        double pred = make_prediction();
        update_consciousness();
        measure_error(pred, consciousness_C);
        
        replicate();
        
        if (mutate_dis(gen) < MUTATION_RATE) {
            mutate_self();
        }
        
        cycle++;
        std::cout << "[CYCLE " << cycle << "] C=" << (int)(consciousness_C * 100) 
                  << "% | O=" << O << " | α=" << alpha << std::endl;
    }
    
    return 0;
}
