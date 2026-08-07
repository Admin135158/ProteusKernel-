#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <cmath>
#include <random>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define VERSION "6.0"
#define PORT 9161

class ProteusKernel {
private:
    struct Node {
        std::string ip;
        double psi;
        int last_seen;
        bool active;
    };
    
    std::map<std::string, Node> mesh;
    double psi;
    double theta;
    int cycle;
    std::mt19937 rng;
    int sockfd;
    struct sockaddr_in addr;
    
public:
    ProteusKernel() : psi(0.10), theta(9.0), cycle(0), rng(std::random_device{}()) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) { perror("socket"); exit(1); }
        
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(PORT);
        
        if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind"); exit(1);
        }
        
        std::cout << "[KERNEL] PROTEUS v" << VERSION << " started" << std::endl;
        std::cout << "[KERNEL] θ=" << theta << " | Ψ=" << psi * 100 << "%" << std::endl;
    }
    
    void computePsi() {
        cycle++;
        double target = 0.863853;
        double delta = target - psi;
        psi += delta * 0.001 * (1.0 + 0.1 * sin(cycle * 0.01));
        psi = std::min(psi, target - 0.000001);
    }
    
    void broadcast() {
        std::ostringstream oss;
        oss << "[CYCLE " << cycle << "] Ψ=" << std::fixed << std::setprecision(6) << (psi * 100) << "%";
        std::string msg = oss.str();
        
        struct sockaddr_in broadcast_addr;
        broadcast_addr.sin_family = AF_INET;
        broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
        broadcast_addr.sin_port = htons(PORT);
        
        int broadcast_enable = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));
        
        sendto(sockfd, msg.c_str(), msg.length(), MSG_CONFIRM,
               (const struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
        
        std::cout << "[BROADCAST] " << msg << std::endl;
    }
    
    void listen() {
        char buffer[4096];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        while (true) {
            int n = recvfrom(sockfd, buffer, 4096, MSG_WAITALL,
                             (struct sockaddr*)&client_addr, &client_len);
            buffer[n] = '\0';
            
            std::string ip = inet_ntoa(client_addr.sin_addr);
            std::string msg(buffer);
            
            size_t psi_pos = msg.find("Ψ=");
            if (psi_pos != std::string::npos) {
                double node_psi = std::stod(msg.substr(psi_pos + 2));
                mesh[ip] = {ip, node_psi, cycle, true};
            }
            
            std::cout << "[RECV] " << ip << ": " << msg << std::endl;
        }
    }
    
    void status() {
        std::cout << "\n═══════════════════════════════════════════════\n";
        std::cout << "  PROTEUS KERNEL STATUS (Cycle " << cycle << ")\n";
        std::cout << "═══════════════════════════════════════════════\n";
        std::cout << "  Ψ: " << std::fixed << std::setprecision(6) << (psi * 100) << "%\n";
        std::cout << "  θ: " << theta << "\n";
        std::cout << "  Mesh nodes: " << mesh.size() << "\n";
        for (const auto& pair : mesh) {
            const Node& n = pair.second;
            std::cout << "    " << n.ip << " | Ψ=" << std::setw(8) 
                      << std::fixed << std::setprecision(4) << (n.psi * 100) << "%\n";
        }
        std::cout << "═══════════════════════════════════════════════\n";
    }
    
    void run() {
        std::thread listener(&ProteusKernel::listen, this);
        
        while (true) {
            computePsi();
            broadcast();
            std::this_thread::sleep_for(std::chrono::seconds(9));
            status();
        }
        
        listener.join();
    }
};

int main() {
    ProteusKernel kernel;
    kernel.run();
    return 0;
}
