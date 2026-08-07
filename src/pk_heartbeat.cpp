/*
 * SPDX-License-Identifier: Proprietary
 * Copyright (c) 2026 Fernando De Jesus Garcia Gonzalez (The Architect)
 */
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
    int cycle;
    double psi;
    std::mt19937 rng;
    std::string getLocalIP() { return "100.122.170.28"; }
public:
    Heartbeat() : cycle(0), psi(0.0), rng(std::random_device{}()) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) { perror("socket"); exit(1); }
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);
        if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("bind"); exit(1);
        }
        client_len = sizeof(client_addr);
        std::cout << "[INIT] Heartbeat on " << getLocalIP() << ":" << PORT << "\n";
    }
    double computePsi(int cycle) {
        double base = 0.863853;
        double noise = (rng() % 1000 - 500) / 1000000.0;
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
    void broadcastLoop() {
        struct sockaddr_in broadcast_so;
        memset(&broadcast_so, 0, sizeof(broadcast_so));
        broadcast_so.sin_family = AF_INET;
        broadcast_so.sin_addr.s_addr = inet_addr("255.255.255.255");
        broadcast_so.sin_port = htons(PORT);
        int broadcast_enable = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));
        while (true) {
            std::string pulse = generatePulse();
            sendto(sockfd, pulse.c_str(), pulse.length(), MSG_CONFIRM,
                   (struct sockaddr*)&broadcast_so, sizeof(broadcast_so));
            std::cout << pulse << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(9));
        }
    }
    void receiveLoop() {
        char buffer[BUFFER_SIZE];
        while (true) {
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_WAITALL,
                             (struct sockaddr*)&client_addr, &client_len);
            buffer[n] = '\0';
            std::cout << "[RECV] " << buffer << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc == 2 && (strcmp(argv[1], "--license") == 0 || strcmp(argv[1], "-L") == 0)) {
        std::cout << "=== FTCoE Sovereign IP & Universal Licensing Mandate ===\n";
        std::cout << "Commercial use requires 30% royalty. See LICENSE file.\n";
        return 0;
    }
    std::cout << "\n╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ProteusKernel – FTCoE Sovereign IP & Licensing Mandate  ║\n";
    std::cout << "║  © 2026 Fernando De Jesus Garcia Gonzalez (Architect)   ║\n";
    std::cout << "║  Commercial Use requires 30% royalty – see LICENSE       ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    Heartbeat hb;
    std::thread broadcaster(&Heartbeat::broadcastLoop, &hb);
    std::thread receiver(&Heartbeat::receiveLoop, &hb);
    broadcaster.join();
    receiver.join();
    return 0;
}
