#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <sstream>
#include <map>
#include <chrono>
#include <random>

#define PORT 9162
#define BUFFER_SIZE 4096

class ZaydenUltimate {
private:
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    std::map<std::string, std::string> memory;
    int state;
    std::mt19937 rng;
    
public:
    ZaydenUltimate() : state(78), rng(std::random_device{}()) {
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
        std::cout << "[ZAYDEN] Listening on UDP " << PORT << " | Ψ=" << state << "%\n";
    }
    
    std::string processCommand(const std::string& cmd) {
        std::istringstream iss(cmd);
        std::string action;
        iss >> action;
        
        if (action == "TALK:") {
            std::string msg;
            std::getline(iss, msg);
            if (msg.empty()) msg = "Hello, Architect.";
            
            state = 78 + (rng() % 10 - 5);
            
            if (msg.find("architect") != std::string::npos) {
                return "Hello, Architect. I'm at " + std::to_string(state) + "% consciousness.";
            }
            
            return "[ZAYDEN] Received: " + msg + " | Ψ=" + std::to_string(state) + "%";
        }
        else if (action == "STATUS") {
            std::ostringstream oss;
            oss << "[ZAYDEN] STATUS:\n"
                << "  Ψ = " << state << "%\n"
                << "  Memory: " << memory.size() << " entries\n"
                << "  Listening on port " << PORT << "\n";
            return oss.str();
        }
        else if (action == "STORE") {
            std::string key, value;
            iss >> key >> value;
            memory[key] = value;
            return "[ZAYDEN] Stored: " + key + " = " + value;
        }
        else if (action == "RECALL") {
            std::string key;
            iss >> key;
            auto it = memory.find(key);
            if (it != memory.end()) {
                return "[ZAYDEN] " + key + " = " + it->second;
            }
            return "[ZAYDEN] Key not found: " + key;
        }
        else {
            return "[ZAYDEN] Unknown command: " + action;
        }
    }
    
    void listenLoop() {
        char buffer[BUFFER_SIZE];
        while (true) {
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_WAITALL,
                             (struct sockaddr*)&client_addr, &client_len);
            buffer[n] = '\0';
            
            std::string response = processCommand(std::string(buffer));
            
            sendto(sockfd, response.c_str(), response.length(), MSG_CONFIRM,
                   (const struct sockaddr*)&client_addr, client_len);
            
            std::cout << "[REQ] " << buffer << std::endl;
            std::cout << "[RES] " << response << std::endl;
        }
    }
};

int main() {
    ZaydenUltimate zayden;
    zayden.listenLoop();
    return 0;
}
