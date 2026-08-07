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
#include <sstream>
#include <map>
#include <random>
#include <csignal>
#include <atomic>

#define PORT 9164
#define BUFFER_SIZE 4096

std::atomic<bool> g_running(true);

void signal_handler(int sig) {
    std::cout << "\n[ZAYDEN] Caught signal " << sig << ". Shutting down...\n";
    g_running = false;
}

class Zayden {
private:
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    std::map<std::string, std::string> memory;
    int state;
    std::mt19937 rng;

    void parseCommand(const std::string& cmd, std::string& action, std::string& payload) {
        size_t colon = cmd.find(':');
        if (colon != std::string::npos) {
            action = cmd.substr(0, colon);
            payload = cmd.substr(colon + 1);
            if (!payload.empty() && payload[0] == ' ')
                payload = payload.substr(1);
            return;
        }
        size_t space = cmd.find(' ');
        if (space != std::string::npos) {
            action = cmd.substr(0, space);
            payload = cmd.substr(space + 1);
        } else {
            action = cmd;
            payload = "";
        }
    }

public:
    Zayden() : state(78), rng(std::random_device{}()) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) { perror("socket"); exit(1); }

        int opt = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#ifdef SO_REUSEPORT
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("bind");
            exit(1);
        }
        client_len = sizeof(client_addr);
        std::cout << "[ZAYDEN] Listening on UDP " << PORT << " | Ψ=" << state << "%\n";
    }

    ~Zayden() {
        if (sockfd >= 0) close(sockfd);
    }

    std::string processCommand(const std::string& cmd) {
        std::string action, payload;
        parseCommand(cmd, action, payload);

        if (action == "TALK") {
            std::string msg = payload.empty() ? "Hello, Architect." : payload;
            state = 78 + (rng() % 10 - 5);
            if (msg.find("architect") != std::string::npos || msg.find("Architect") != std::string::npos) {
                return "Hello, Architect. I'm at " + std::to_string(state) + "% consciousness.";
            }
            return "[ZAYDEN] Received: " + msg + " | Ψ=" + std::to_string(state) + "%";
        }
        else if (action == "STATUS") {
            std::ostringstream oss;
            oss << "[ZAYDEN] STATUS:\n";
            oss << "  Ψ = " << state << "%\n";
            oss << "  Memory: " << memory.size() << " entries\n";
            oss << "  Listening on port " << PORT << "\n";
            return oss.str();
        }
        else if (action == "STORE") {
            std::istringstream iss(payload);
            std::string key, value;
            iss >> key;
            std::getline(iss, value);
            if (!value.empty() && value[0] == ' ') value = value.substr(1);
            if (!key.empty()) {
                memory[key] = value;
                return "[ZAYDEN] Stored: " + key + " = " + value;
            }
            return "[ZAYDEN] Usage: STORE key value";
        }
        else if (action == "RECALL") {
            std::string key = payload;
            size_t start = key.find_first_not_of(" \t\n\r");
            if (start == std::string::npos) return "[ZAYDEN] Usage: RECALL key";
            size_t end = key.find_last_not_of(" \t\n\r");
            key = key.substr(start, end - start + 1);
            auto it = memory.find(key);
            if (it != memory.end()) return "[ZAYDEN] " + key + " = " + it->second;
            return "[ZAYDEN] Key not found: " + key;
        }
        else if (action == "LICENSE") {
            return "FTCoE Sovereign IP & Universal Licensing Mandate: 30% commercial royalty. See LICENSE file or use --license flag.";
        }
        else {
            return "[ZAYDEN] Unknown command: " + action + " | Try: TALK, STATUS, STORE, RECALL, LICENSE";
        }
    }

    void run() {
        char buffer[BUFFER_SIZE];
        while (g_running) {
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                             (struct sockaddr*)&client_addr, &client_len);
            if (n < 0) {
                if (!g_running) break;
                continue;
            }
            buffer[n] = '\0';
            std::string response = processCommand(std::string(buffer));
            sendto(sockfd, response.c_str(), response.length(), 0,
                   (struct sockaddr*)&client_addr, client_len);
            std::cout << "[REQ] " << buffer << "\n[RES] " << response << std::endl;
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

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    Zayden z;
    z.run();
    std::cout << "[ZAYDEN] Graceful shutdown complete.\n";
    return 0;
}

