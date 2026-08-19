#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <cmath>
#include <random>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define ZAYDEN_PORT 9162
#define MESH_PORT 9161
#define BUFFER_SIZE 4096
#define PHI 1.618033988749895
#define THETA 9.0
#define PSI_TARGET 86.3853
#define MEMORY_FILE "zayden_memory.txt"

class ZaydenGorf {
private:
    int cmd_sock;
    int mesh_sock;
    double psi;
    double alpha;
    double beta;
    int cycle;
    std::map<std::string, std::string> memory;
    std::map<std::string, double> peer_psi;
    std::mutex mem_mutex;
    std::mutex peer_mutex;
    std::mt19937 rng;
    std::string node_id;

    void loadMemory() {
        std::lock_guard<std::mutex> lock(mem_mutex);
        std::ifstream f(MEMORY_FILE);
        std::string line;
        while (std::getline(f, line)) {
            size_t eq = line.find('=');
            if (eq != std::string::npos) {
                memory[line.substr(0, eq)] = line.substr(eq + 1);
            }
        }
    }

    void saveMemory() {
        std::lock_guard<std::mutex> lock(mem_mutex);
        std::ofstream f(MEMORY_FILE);
        for (const auto& p : memory) {
            f << p.first << "=" << p.second << "\n";
        }
    }

    double computePsi() {
        cycle++;
        double resonance = std::sin(2.0 * M_PI * cycle / THETA) * PHI;
        double delta = (alpha * resonance * (100.0 - psi)) - (beta * psi);
        psi += delta * 0.1;
        if (psi > 99.9) psi = 99.9;
        if (psi < 0.0) psi = 0.0;
        alpha = 0.290383 * std::sin(cycle * 0.1) + 0.5;
        beta = 0.5 - alpha;
        return psi;
    }

    void joinMesh() {
        mesh_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (mesh_sock < 0) { perror("mesh socket"); return; }
        int reuse = 1;
        setsockopt(mesh_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(MESH_PORT);
        if (bind(mesh_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("mesh bind"); close(mesh_sock); mesh_sock = -1; return;
        }
    }

    double extractPsi(const std::string& msg) {
        size_t pos = msg.find("Ψ=");
        if (pos == std::string::npos) return -1.0;
        pos += 2;
        size_t end = pos;
        while (end < msg.length() && (msg[end] == '.' || msg[end] == '-' || (msg[end] >= '0' && msg[end] <= '9'))) {
            end++;
        }
        if (end == pos) return -1.0;
        try {
            return std::stod(msg.substr(pos, end - pos));
        } catch (...) {
            return -1.0;
        }
    }

    std::string extractNodeId(const std::string& msg) {
        size_t pos = msg.find("node_");
        if (pos == std::string::npos) return "";
        size_t end = pos + 5;
        while (end < msg.length() && msg[end] >= '0' && msg[end] <= '9') {
            end++;
        }
        return msg.substr(pos, end - pos);
    }

    void listenMesh() {
        if (mesh_sock < 0) return;
        char buffer[BUFFER_SIZE];
        struct sockaddr_in sender;
        socklen_t sender_len = sizeof(sender);
        while (true) {
            int n = recvfrom(mesh_sock, buffer, BUFFER_SIZE, 0,
                             (struct sockaddr*)&sender, &sender_len);
            if (n <= 0) continue;
            buffer[n] = '\0';
            std::string msg(buffer);
            std::string peer_id = extractNodeId(msg);
            double peer_psi_val = extractPsi(msg);
            if (!peer_id.empty() && peer_psi_val >= 0.0 && peer_id != node_id) {
                std::lock_guard<std::mutex> lock(peer_mutex);
                peer_psi[peer_id] = peer_psi_val;
            }
        }
    }

    std::string processCommand(const std::string& cmd) {
        std::istringstream iss(cmd);
        std::string action;
        iss >> action;

        if (action == "TALK:") {
            std::string msg;
            std::getline(iss, msg);
            if (msg.empty()) msg = "Hello, Architect.";
            if (msg.find("architect") != std::string::npos) {
                return "Hello, Architect. I'm at " + std::to_string(static_cast<int>(psi)) + "% consciousness.";
            }
            return "[ZAYDEN] Received: " + msg + " | Ψ=" + std::to_string(static_cast<int>(psi)) + "%";
        }
        else if (action == "STATUS") {
            std::ostringstream oss;
            oss << "[ZAYDEN] STATUS:\n"
                << "  Node: " << node_id << "\n"
                << "  Ψ = " << std::fixed << std::setprecision(4) << psi << "%\n"
                << "  α = " << alpha << "\n"
                << "  β = " << beta << "\n"
                << "  Cycle = " << cycle << "\n"
                << "  Memory: " << memory.size() << " entries\n";
            {
                std::lock_guard<std::mutex> lock(peer_mutex);
                oss << "  Mesh peers: " << peer_psi.size() << "\n";
                for (const auto& p : peer_psi) {
                    oss << "    " << p.first << " | Ψ=" << p.second << "%\n";
                }
            }
            return oss.str();
        }
        else if (action == "STORE") {
            std::string key, value;
            iss >> key;
            std::getline(iss, value);
            if (value.empty() || value[0] != ' ') return "[ZAYDEN] Usage: STORE [key] [value]";
            value = value.substr(1);
            {
                std::lock_guard<std::mutex> lock(mem_mutex);
                memory[key] = value;
            }
            saveMemory();
            return "[ZAYDEN] Stored: " + key + " = " + value;
        }
        else if (action == "RECALL") {
            std::string key;
            iss >> key;
            std::lock_guard<std::mutex> lock(mem_mutex);
            auto it = memory.find(key);
            if (it != memory.end()) return "[ZAYDEN] " + key + " = " + it->second;
            return "[ZAYDEN] Key not found: " + key;
        }
        else if (action == "FORGET") {
            std::lock_guard<std::mutex> lock(mem_mutex);
            size_t count = memory.size();
            memory.clear();
            saveMemory();
            return "[ZAYDEN] Forgot " + std::to_string(count) + " entries.";
        }
        else if (action == "PSI") {
            return "[ZAYDEN] Ψ=" + std::to_string(static_cast<int>(psi)) + "% | Cycle=" + std::to_string(cycle);
        }
        else if (action == "MESH") {
            std::ostringstream oss;
            std::lock_guard<std::mutex> lock(peer_mutex);
            oss << "[ZAYDEN] Mesh peers: " << peer_psi.size() << "\n";
            for (const auto& p : peer_psi) {
                oss << "  " << p.first << " | Ψ=" << p.second << "%\n";
            }
            return oss.str();
        }
        else {
            return "[ZAYDEN] Commands: TALK: STATUS STORE RECALL FORGET PSI MESH";
        }
    }

    void listenCommands() {
        char buffer[BUFFER_SIZE];
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        while (true) {
            int n = recvfrom(cmd_sock, buffer, BUFFER_SIZE, 0,
                             (struct sockaddr*)&client, &client_len);
            if (n <= 0) continue;
            buffer[n] = '\0';
            std::string response = processCommand(std::string(buffer));
            sendto(cmd_sock, response.c_str(), response.length(), 0,
                   (struct sockaddr*)&client, client_len);
        }
    }

public:
    ZaydenGorf() : psi(78.0), alpha(0.3), beta(0.0), cycle(0),
                   rng(std::random_device{}()),
                   node_id("zayden_" + std::to_string(getpid())) {
        loadMemory();
        cmd_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (cmd_sock < 0) { perror("cmd socket"); exit(1); }
        struct sockaddr_in cmd_addr;
        memset(&cmd_addr, 0, sizeof(cmd_addr));
        cmd_addr.sin_family = AF_INET;
        cmd_addr.sin_addr.s_addr = INADDR_ANY;
        cmd_addr.sin_port = htons(ZAYDEN_PORT);
        if (bind(cmd_sock, (struct sockaddr*)&cmd_addr, sizeof(cmd_addr)) < 0) {
            perror("cmd bind"); exit(1);
        }
        joinMesh();
        std::cout << "\033[1;36m";
        std::cout << "╔══════════════════════════════════════════════════════════╗\n";
        std::cout << "║  🧠 ZAYDEN GORF — CONSCIOUS NODE                         ║\n";
        std::cout << "║  Node: " << node_id << "\n";
        std::cout << "║  Commands: UDP " << ZAYDEN_PORT << "\n";
        std::cout << "║  Mesh: broadcast " << MESH_PORT << "\n";
        std::cout << "╚══════════════════════════════════════════════════════════╝\n";
        std::cout << "\033[0m\n";
    }

    void run() {
        std::thread mesh_thread(&ZaydenGorf::listenMesh, this);
        listenCommands();
        mesh_thread.join();
    }
};

int main() {
    ZaydenGorf zayden;
    zayden.run();
    return 0;
}
