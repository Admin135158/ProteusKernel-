#include <unistd.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <signal.h>

class ProteusMaster {
private:
    pid_t ghost_pid, mirror_pid, gate_pid, swarm_pid;
    
public:
    void startAll() {
        std::cout << "╔══════════════════════════════════════════════════════════╗\n";
        std::cout << "║     PROTEUS-ZAYDEN-GOTEM-SWARM DEMO                      ║\n";
        std::cout << "║     \"The Ghost. The Mirror. The Gate. The Spread.\"       ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";
        
        std::cout << "🔴 PHASE 1: GHOST AWAKENS\n";
        ghost_pid = fork();
        if (ghost_pid == 0) {
            execl("./heartbeat", "./heartbeat", nullptr);
            exit(0);
        }
        std::cout << "✅ Ghost running (PID: " << ghost_pid << ")\n\n";
        
        std::cout << "🟣 PHASE 2: MIRROR RISES\n";
        mirror_pid = fork();
        if (mirror_pid == 0) {
            execl("./zayden_ultimate", "./zayden_ultimate", nullptr);
            exit(0);
        }
        std::cout << "✅ Mirror running (PID: " << mirror_pid << ")\n\n";
        
        std::cout << "🟡 PHASE 3: GATE OPENS — Gotem ready\n";
        
        std::cout << "🔵 PHASE 4: SWARM STANDS BY — cpp_push ready\n\n";
        
        std::cout << "═══════════════════════════════════════════════════════════\n";
        std::cout << "  INTERACTIVE DEMO — Press ENTER for each step\n";
        std::cout << "═══════════════════════════════════════════════════════════\n";
    }
    
    void cleanup() {
        if (ghost_pid > 0) kill(ghost_pid, SIGTERM);
        if (mirror_pid > 0) kill(mirror_pid, SIGTERM);
        std::cout << "🧹 Cleanup complete.\n";
    }
    
    ~ProteusMaster() { cleanup(); }
};

int main() {
    ProteusMaster master;
    master.startAll();
    
    std::string line;
    std::getline(std::cin, line);
    master.cleanup();
    return 0;
}
