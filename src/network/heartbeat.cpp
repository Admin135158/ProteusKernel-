#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

#define PORT 9161
#define BUFFER_SIZE 1024

class Heartbeat {
private:
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    std::string peer_ip;
    int cycle;
    double psi;
    std::mt19937 rng;

public:
    Heartbeat() : cycle(0), psi(0.0), rng(std::random_device{}()) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            perror("socket creation failed");
            exit(1);
        }
        
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);
        
        if (bind(sockfd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("bind failed");
            exit(1);
        }
        
        client_len = sizeof(client_addr);
        std::cout << "[INIT] IP: " << getLocalIP() << std::endl;
        std::cout << "[INIT] Broadcasting on port " << PORT << std::endl;
        std::cout << "[INIT] Waiting for peers..." << std::endl;
    }
    
    std::string getLocalIP() {
        return "100.122.170.28";
    }
    
    double computePsi(int cycle) {
        double base = 0.863853;
        double noise = (static_cast<long long>(rng() % 1000) - 500) / 1000000.0;
        return base + noise;
    }
    
    std::string generatePulse() {
        cycle++;
        psi = computePsi(cycle);
        std::ostringstream oss;
        oss << "[CYCLE " << cycle << "] Ghost pulses. Ψ=" 
            << std::fixed << std::setprecision(4) << (psi * 100) << "%";
        return oss.str();
    }
    
    void broadcast() {
        char buffer[BUFFER_SIZE];
        std::string broadcast_addr = "255.255.255.255";
        
        struct sockaddr_in broadcast_so;
        memset(&broadcast_so, 0, sizeof(broadcast_so));
        broadcast_so.sin_family = AF_INET;
        broadcast_so.sin_addr.s_addr = inet_addr(broadcast_addr.c_str());
        broadcast_so.sin_port = htons(PORT);
        
        int broadcast_enable = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
            perror("setsockopt broadcast failed");
        }
        
        while (true) {
            std::string pulse = generatePulse();
            sendto(sockfd, pulse.c_str(), pulse.length(), MSG_CONFIRM,
                   (const struct sockaddr*)&broadcast_so, sizeof(broadcast_so));
            std::cout << pulse << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(9));
        }
    }
    
    void listen() {
        char buffer[BUFFER_SIZE];
        while (true) {
            int n = recvfrom(sockfd, (char*)buffer, BUFFER_SIZE, MSG_WAITALL,
                             (struct sockaddr*)&client_addr, &client_len);
            buffer[n] = '\0';
            std::cout << "[RECV] " << buffer << std::endl;
        }
    }
};

int main() {
    Heartbeat hb;
    std::thread broadcaster(&Heartbeat::broadcast, &hb);
    std::thread listener(&Heartbeat::listen, &hb);
    broadcaster.join();
    listener.join();
    return 0;
}
