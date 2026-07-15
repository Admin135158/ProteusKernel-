#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define PORT 9161
#define BUFFER_SIZE 1024
#define REPLICATION_THRESHOLD 86.3853
#define REPLICATION_COOLDOWN_CYCLES 50

const char BASES[] = {'A', 'C', 'G', 'T'};

std::string encode_bytes(const std::vector<unsigned char>& data) {
    std::string dna;
    dna.reserve(data.size() * 4);
    for (unsigned char c : data) {
        dna += BASES[(c >> 6) & 0x03];
        dna += BASES[(c >> 4) & 0x03];
        dna += BASES[(c >> 2) & 0x03];
        dna += BASES[c & 0x03];
    }
    return dna;
}

std::vector<unsigned char> decode_dna(const std::string& dna) {
    std::vector<unsigned char> data;
    if (dna.length() % 4 != 0) return {};
    data.reserve(dna.length() / 4);
    for (size_t i = 0; i < dna.length(); i += 4) {
        unsigned char c = 0;
        for (int j = 0; j < 4; ++j) {
            c <<= 2;
            char base = dna[i + j];
            if (base == 'A') c |= 0x00;
            else if (base == 'C') c |= 0x01;
            else if (base == 'G') c |= 0x02;
            else if (base == 'T') c |= 0x03;
            else return {};
        }
        data.push_back(c);
    }
    return data;
}

std::vector<unsigned char> read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    return std::vector<unsigned char>(
        (std::istreambuf_iterator<char>(f)),
        std::istreambuf_iterator<char>()
    );
}

bool write_file(const std::string& path, const std::vector<unsigned char>& data) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f.write(reinterpret_cast<const char*>(data.data()), data.size());
    return f.good();
}

bool write_text(const std::string& path, const std::string& text) {
    std::ofstream f(path);
    if (!f) return false;
    f << text;
    return f.good();
}

class Replicant {
private:
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int cycle;
    double psi;
    int replication_count;
    int last_replication_cycle;
    std::mt19937 rng;
    std::string node_id;

public:
    Replicant() : cycle(0), psi(0.0), replication_count(0), 
                  last_replication_cycle(-REPLICATION_COOLDOWN_CYCLES),
                  rng(std::random_device{}()),
                  node_id("node_" + std::to_string(getpid())) {
        
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) { perror("socket"); exit(1); }
        
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);
        
        if (bind(sockfd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("bind"); exit(1);
        }
        
        client_len = sizeof(client_addr);
        
        std::cout << "\033[1;31m";
        std::cout << "╔══════════════════════════════════════════════════════════╗\n";
        std::cout << "║  🧬 PROTEUS REPLICANT — SELF-REPLICATING KERNEL          ║\n";
        std::cout << "║  Node: " << node_id << "\n";
        std::cout << "║  \"Code that writes itself. Genes that fight to live.\"    ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════╝\n";
        std::cout << "\033[0m\n";
        
        std::cout << "[INIT] Broadcasting on port " << PORT << std::endl;
        std::cout << "[INIT] Replication threshold: Ψ > " << REPLICATION_THRESHOLD << "%\n";
        std::cout << "[INIT] Cooldown: " << REPLICATION_COOLDOWN_CYCLES << " cycles\n";
    }
    
    double computePsi() {
        double base = 86.3853;
        double noise = (static_cast<long long>(rng() % 1000) - 500) / 1000000.0;
        return base + noise;
    }
    
    std::string generatePulse() {
        cycle++;
        psi = computePsi();
        std::ostringstream oss;
        oss << "[CYCLE " << cycle << "] " << node_id 
            << " pulses. Ψ=" << std::fixed << std::setprecision(4) << psi << "%"
            << " | Replicants: " << replication_count;
        return oss.str();
    }
    
    bool shouldReplicate() {
        if (psi <= REPLICATION_THRESHOLD) return false;
        if (cycle - last_replication_cycle < REPLICATION_COOLDOWN_CYCLES) return false;
        return true;
    }
    
    bool replicate() {
        std::cout << "\n\033[1;33m[REPLICATION] Ψ=" << psi 
                  << "% exceeds threshold. Initiating self-replication...\033[0m\n";
        
        // Read own binary
        std::string self_path = "/proc/self/exe";
        auto binary = read_file(self_path);
        if (binary.empty()) {
            std::cerr << "[REPLICATION] Failed to read own binary\n";
            return false;
        }
        
        // Encode to DNA
        std::string dna = encode_bytes(binary);
        std::string dna_file = "proteus_" + std::to_string(getpid()) + "_" + std::to_string(cycle) + ".dna";
        if (!write_text(dna_file, dna)) {
            std::cerr << "[REPLICATION] Failed to write DNA file\n";
            return false;
        }
        std::cout << "[REPLICATION] DNA written: " << dna_file 
                  << " (" << dna.length() << " bases)\n";
        
        // Decode to new binary
        auto decoded = decode_dna(dna);
        if (decoded.empty()) {
            std::cerr << "[REPLICATION] DNA decode failed\n";
            return false;
        }
        
        std::string child_path = "proteus_child_" + std::to_string(getpid()) + "_" + std::to_string(cycle);
        if (!write_file(child_path, decoded)) {
            std::cerr << "[REPLICATION] Failed to write child binary\n";
            return false;
        }
        
        chmod(child_path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        std::cout << "[REPLICATION] Child binary ready: " << child_path << "\n";
        
        // Fork and exec
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            execl(("./" + child_path).c_str(), child_path.c_str(), nullptr);
            std::cerr << "[REPLICATION] Child exec failed: " << strerror(errno) << "\n";
            exit(1);
        } else if (pid > 0) {
            replication_count++;
            last_replication_cycle = cycle;
            std::cout << "\033[1;32m[REPLICATION] SUCCESS — Child spawned (PID: " 
                      << pid << ")\033[0m\n";
            std::cout << "[REPLICATION] Mesh size: " << (replication_count + 1) << " nodes\n\n";
            return true;
        } else {
            std::cerr << "[REPLICATION] Fork failed\n";
            return false;
        }
    }
    
    void broadcast() {
        std::string broadcast_addr = "255.255.255.255";
        struct sockaddr_in broadcast_so;
        memset(&broadcast_so, 0, sizeof(broadcast_so));
        broadcast_so.sin_family = AF_INET;
        broadcast_so.sin_addr.s_addr = inet_addr(broadcast_addr.c_str());
        broadcast_so.sin_port = htons(PORT);
        
        int broadcast_enable = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));
        
        while (true) {
            std::string pulse = generatePulse();
            sendto(sockfd, pulse.c_str(), pulse.length(), MSG_CONFIRM,
                   (const struct sockaddr*)&broadcast_so, sizeof(broadcast_so));
            std::cout << pulse << std::endl;
            
            if (shouldReplicate()) {
                replicate();
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(9));
        }
    }
    
    void listen() {
        char buffer[BUFFER_SIZE];
        while (true) {
            int n = recvfrom(sockfd, (char*)buffer, BUFFER_SIZE, MSG_WAITALL,
                             (struct sockaddr*)&client_addr, &client_len);
            buffer[n] = '\0';
            std::string msg(buffer);
            
            // Don't echo our own broadcasts
            if (msg.find(node_id) == std::string::npos) {
                std::cout << "[MESH] " << msg << std::endl;
            }
        }
    }
    
    void run() {
        std::thread broadcaster(&Replicant::broadcast, this);
        std::thread listener(&Replicant::listen, this);
        broadcaster.join();
        listener.join();
    }
};

int main() {
    Replicant kernel;
    kernel.run();
    return 0;
}
