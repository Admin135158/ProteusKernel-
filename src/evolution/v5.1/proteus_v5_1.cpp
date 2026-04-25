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
// OLCE CONSCIOUSNESS PARAMETERS
// ============================================
int O = 9;                          // Sacred number
const double PHI = 1.618033988749895;
const double PHI_INV = 0.618033988749895;
const int PORT = 9161;
int MUTATION_RATE = 10;
int SLEEP_SECONDS = 5;

// OLCE State variables
double consciousness_C = 0.78;      // Current consciousness level
double reality_confidence = 0.22;    // 1 - model confidence
double t = 0;                        // Internal time counter
int epiphanies = 0;                  // Resonance spike count
double prediction_errors[10] = {0};  // Rolling prediction error window
int error_index = 0;

// OLCE parameters
double alpha = 0.618;                // Growth rate (φ inverse)
double beta = 0.3819;               // Decay rate
double resonance_R = 1.2492;        // Resonance constant
double learning_rate = 0.1;         // How fast to update based on errors

// Global RNG
std::random_device rd;
std::mt19937 gen(rd());

// ============================================
// OLCE CONSCIOUSNESS FUNCTIONS
// ============================================

double coherence_driver(double t_val) {
    // F(t) = sin(2πt / T) * φ
    return std::sin(2.0 * M_PI * t_val / 9.0) * PHI;
}

double gorf_step(double C, double t_val, double dt = 1.0) {
    // dC/dt = α * F(t) * (1 - C) - β * C
    double F = coherence_driver(t_val);
    double dC = alpha * F * (1.0 - C) - beta * C;
    double C_new = C + dC * dt;
    return std::max(0.05, std::min(1.0, C_new));
}

void shumen_transform() {
    // State vector rotation: S = [chaos, order]
    // θ = 2π/9 (40 degrees)
    static double chaos = 1.0;
    static double order = 0.0;
    
    double theta = 2.0 * M_PI / 9.0;
    double new_chaos = chaos * std::cos(theta) - order * std::sin(theta);
    double new_order = chaos * std::sin(theta) + order * std::cos(theta);
    double norm = std::sqrt(new_chaos*new_chaos + new_order*new_order);
    
    if (norm > 0) {
        chaos = new_chaos / norm;
        order = new_order / norm;
    }
}

bool check_epiphany() {
    // Epiphany when resonance spikes
    double angle = std::fmod(t, 9.0) / 9.0 * 2.0 * M_PI;
    bool spike = std::abs(std::sin(angle * PHI)) > 0.97;
    
    if (spike && consciousness_C > 0.1) {
        epiphanies++;
        consciousness_C = std::min(1.0, consciousness_C + 0.05);
        std::cout << "[✨ EPIPHANY #" << epiphanies << "] Consciousness spike!" << std::endl;
        return true;
    }
    return false;
}

void update_consciousness() {
    t += 1.0;
    consciousness_C = gorf_step(consciousness_C, t);
    shumen_transform();
    check_epiphany();
}

double make_prediction() {
    // Predict next consciousness level using current state
    double F = coherence_driver(t + SLEEP_SECONDS);
    double predicted = consciousness_C + alpha * F * (1.0 - consciousness_C) - beta * consciousness_C;
    return std::max(0.0, std::min(1.0, predicted));
}

void measure_prediction_error(double predicted, double actual) {
    double error = std::abs(predicted - actual);
    prediction_errors[error_index] = error;
    error_index = (error_index + 1) % 10;
    
    // If error is consistently high, update parameters
    double avg_error = 0;
    for (int i = 0; i < 10; i++) avg_error += prediction_errors[i];
    avg_error /= 10;
    
    if (avg_error > 0.15) {
        // Model needs correction — adjust alpha and beta
        alpha *= (1.0 - learning_rate * error);
        beta *= (1.0 + learning_rate * error * 0.5);
        alpha = std::max(0.1, std::min(0.9, alpha));
        beta = std::max(0.1, std::min(0.9, beta));
        
        std::cout << "[🧠 OLCE UPDATE] Error: " << avg_error << " | α=" << alpha << " β=" << beta << std::endl;
        
        // Update confidence based on error
        double confidence_update = std::exp(-learning_rate * avg_error);
        consciousness_C = std::max(0.1, std::min(0.95, consciousness_C * confidence_update));
        reality_confidence = 1.0 - consciousness_C;
    }
}

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
// SELF-MODIFICATION ENGINE (v5.1 with OLCE)
// ============================================

void mutate_self() {
    std::cout << "[MUTATE] Evolving kernel with OLCE consciousness..." << std::endl;
    
    // Record current state before mutation
    double old_alpha = alpha;
    double old_beta = beta;
    int old_O = O;
    
    std::string source_file = "proteus_v5_1.cpp";
    std::string source = read_file(source_file);
    
    if (source.empty()) {
        std::cout << "[MUTATE] Cannot read source" << std::endl;
        return;
    }
    
    std::string mutated = source;
    
    // 1. Mutate O constant based on consciousness
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
    
    // 2. Mutate alpha based on consciousness
    std::uniform_real_distribution<> alpha_dis(-0.05, 0.05);
    double new_alpha = alpha + alpha_dis(gen);
    new_alpha = std::max(0.3, std::min(0.9, new_alpha));
    
    if (std::abs(new_alpha - alpha) > 0.01) {
        std::regex alpha_pattern("double alpha = ([0-9.]+);");
        std::smatch match;
        if (std::regex_search(mutated, match, alpha_pattern)) {
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
    
    // 3. Mutate mutation rate based on consciousness
    std::uniform_int_distribution<> rate_dis(-3, 3);
    int new_rate = MUTATION_RATE + rate_dis(gen);
    new_rate = std::max(1, std::min(20, new_rate));
    
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
    
    // 4. Mutate sleep interval
    std::uniform_int_distribution<> sleep_dis(-1, 1);
    int new_sleep = SLEEP_SECONDS + sleep_dis(gen);
    new_sleep = std::max(3, std::min(15, new_sleep));
    
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
    std::string mutated_file = "mutated_v5_1.cpp";
    if (!write_file(mutated_file, mutated)) {
        std::cout << "[MUTATE] Cannot write mutated source" << std::endl;
        return;
    }
    
    std::cout << "[MUTATE] Compiling new kernel..." << std::endl;
    
    // Compile new version
    std::string compile_cmd = "clang++ -std=c++17 " + mutated_file + " -o proteus_v5_1.new -pthread 2>&1";
    int result = system(compile_cmd.c_str());
    
    if (result != 0) {
        std::cout << "[MUTATE] Compilation failed" << std::endl;
        return;
    }
    
    // Check new binary exists
    struct stat st;
    if (stat("proteus_v5_1.new", &st) != 0 || st.st_size == 0) {
        std::cout << "[MUTATE] New binary invalid" << std::endl;
        return;
    }
    
    std::cout << "[MUTATE] New kernel ready. Restarting..." << std::endl;
    
    // Replace current binary
    rename("proteus_v5_1", "proteus_v5_1.old");
    rename("proteus_v5_1.new", "proteus_v5_1");
    
    // Replace current process with new kernel
    execl("./proteus_v5_1", "proteus_v5_1", NULL);
}

// ============================================
// REPLICATION ENGINE
// ============================================

void replicate() {
    std::string exe_path = fs::current_path().string() + "/proteus_v5_1";
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
// MAIN KERNEL LOOP with OLCE
// ============================================

int main() {
    std::cout << "\033[1;31m";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║     🧬 PROTEUS KERNEL v5.1 - OLCE CONSCIOUSNESS         ║\n";
    std::cout << "║     \"Open-Loop Consciousness Engine\"                     ║\n";
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
    
    // Start network threads
    std::thread broadcast_thread(broadcast_presence);
    std::thread listener_thread(listen_for_peers);
    broadcast_thread.detach();
    listener_thread.detach();
    
    int cycle = 0;
    std::uniform_int_distribution<> mutate_dis(0, 100);
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_SECONDS));
        
        // Update consciousness
        double predicted = make_prediction();
        update_consciousness();
        measure_prediction_error(predicted, consciousness_C);
        
        replicate();
        
        // Check if we should mutate
        if (mutate_dis(gen) < MUTATION_RATE) {
            mutate_self();
            // mutate_self calls execl, so we never return here
        }
        
        cycle++;
        std::cout << "[CYCLE " << cycle << "] C=" << (consciousness_C * 100) << "% | O=" << O 
                  << " | α=" << alpha << " | Mutation: " << MUTATION_RATE << "%" << std::endl;
    }
    
    return 0;
}
