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
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>

#define SWARM_PORT 9163
#define CHUNK_SIZE 4096
#define TIMEOUT_SEC 30
#define MAX_RETRIES 3

class SwarmNode {
private:
    int sockfd;
    struct sockaddr_in addr;
public:
    SwarmNode() {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) { perror("socket"); exit(1); }
        int opt = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }
    void startServer() {
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(SWARM_PORT);
        if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind"); exit(1);
        }
        listen(sockfd, 5);
        std::cout << "[SWARM] Server listening on port " << SWARM_PORT << std::endl;
        while (true) {
            struct sockaddr_in client_addr;
            socklen_t len = sizeof(client_addr);
            int client_fd = accept(sockfd, (struct sockaddr*)&client_addr, &len);
            if (client_fd < 0) { perror("accept"); continue; }
            std::thread(&SwarmNode::handleClient, this, client_fd).detach();
        }
    }
    void handleClient(int fd) {
        char buffer[CHUNK_SIZE];
        std::string filename;
        int n = recv(fd, buffer, CHUNK_SIZE, 0);
        if (n > 0) {
            filename = std::string(buffer, n);
            std::cout << "[SWARM] Receiving file: " << filename << std::endl;
        }
        std::ofstream out(filename, std::ios::binary);
        while ((n = recv(fd, buffer, CHUNK_SIZE, 0)) > 0) {
            out.write(buffer, n);
        }
        out.close();
        std::cout << "[SWARM] File saved: " << filename << std::endl;
        close(fd);
    }
    void pushFile(const std::string& filename, const std::string& target_ip) {
        struct sockaddr_in target;
        target.sin_family = AF_INET;
        target.sin_port = htons(SWARM_PORT);
        inet_pton(AF_INET, target_ip.c_str(), &target.sin_addr);
        struct timeval tv;
        tv.tv_sec = TIMEOUT_SEC;
        tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        for (int attempt = 0; attempt < MAX_RETRIES; ++attempt) {
            if (connect(sockfd, (struct sockaddr*)&target, sizeof(target)) == 0) {
                break;
            }
            if (attempt == MAX_RETRIES - 1) {
                std::cerr << "[ERROR] Could not connect to " << target_ip << " after " << MAX_RETRIES << " attempts.\n";
                return;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1 << attempt));
        }
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[ERROR] Cannot open: " << filename << std::endl;
            return;
        }
        std::string base = filename.substr(filename.find_last_of("/\\") + 1);
        send(sockfd, base.c_str(), base.length(), 0);
        char buffer[CHUNK_SIZE];
        while (file.read(buffer, CHUNK_SIZE) || file.gcount() > 0) {
            if (send(sockfd, buffer, file.gcount(), 0) < 0) {
                std::cerr << "[ERROR] Send failed.\n";
                break;
            }
        }
        file.close();
        close(sockfd);
        std::cout << "[SWARM] Push complete: " << filename << " → " << target_ip << std::endl;
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
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [server|push <file> <ip>]" << std::endl;
        return 1;
    }
    std::string mode = argv[1];
    if (mode == "server") {
        SwarmNode node;
        node.startServer();
    } else if (mode == "push" && argc == 4) {
        SwarmNode node;
        node.pushFile(argv[2], argv[3]);
    } else {
        std::cerr << "Invalid args" << std::endl;
        return 1;
    }
    return 0;
}
