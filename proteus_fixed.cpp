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

class ProteusFixed {
private:
    double psi;
    double theta;
    int cycle;
    int port;
    std::map<std::string, double> metrics;
    std::mt19937 rng;
    int sockfd;
    struct sockaddr_in addr;
    
public:
    ProteusFixed(int p = 9161) : psi(0.10), theta(9.0), cycle(0), port(p), rng(std::random_device{}()) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) { perror("socket"); return; }
        
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        
        if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind"); return;
        }
        
        std::cout << "[FIXED] PROTEUS running on port " << port << std::endl;
    }
    
    void compute() {
        cycle++;
        double delta = 0.863853 - psi;
        psi += delta * 0.001 * (0.5 + 0.5 * sin(cycle * 0.01));
        psi = std::min(psi, 0.863852);
        
        metrics["psi"] = psi;
        metrics["theta"] = theta;
        metrics["cycle"] = cycle;
    }
    
    void broadcast() {
        std::ostringstream oss;
        oss << "[CYCLE " << cycle << "] Ψ=" << std::fixed << std::setprecision(4) << (psi * 100) << "%";
        std::string msg = oss.str();
        
        struct sockaddr_in broadcast_addr;
        broadcast_addr.sin_family = AF_INET;
        broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
        broadcast_addr.sin_port = htons(port);
        
        int broadcast_enable = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));
        
        sendto(sockfd, msg.c_str(), msg.length(), MSG_CONFIRM,
               (const struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
    }
    
    void run() {
        while (true) {
            compute();
            broadcast();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};

int main(int argc, char* argv[]) {
    int port = (argc > 1) ? atoi(argv[1]) : 9161;
    ProteusFixed p(port);
    p.run();
    return 0;
}
