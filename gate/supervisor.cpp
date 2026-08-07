#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <mutex>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>

std::atomic<bool> running{true};

struct ManagedProcess {
    std::string name;
    std::string binary;
    std::vector<std::string> args;
    pid_t pid;
    int restart_count;
    std::chrono::steady_clock::time_point last_start;
};

std::vector<ManagedProcess> processes;
std::mutex proc_mutex;

void signal_handler(int sig) {
    std::cout << "\n[SUPERVISOR] Caught signal " << sig << ". Shutting down...\n";
    running = false;
}

pid_t spawn(const std::string& binary, const std::vector<std::string>& args) {
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(binary.c_str()));
        for (const auto& a : args) {
            argv.push_back(const_cast<char*>(a.c_str()));
        }
        argv.push_back(nullptr);
        execv(binary.c_str(), argv.data());
        std::cerr << "[SUPERVISOR] exec failed: " << strerror(errno) << " (" << binary << ")\n";
        exit(1);
    }
    return pid;
}

void monitor() {
    while (running) {
        {
            std::lock_guard<std::mutex> lock(proc_mutex);
            for (auto& p : processes) {
                int status;
                pid_t result = waitpid(p.pid, &status, WNOHANG);
                if (result == p.pid) {
                    std::cout << "[SUPERVISOR] " << p.name << " (PID " << p.pid << ") died";
                    if (WIFEXITED(status)) {
                        std::cout << " with exit code " << WEXITSTATUS(status);
                    } else if (WIFSIGNALED(status)) {
                        std::cout << " by signal " << WTERMSIG(status);
                    }
                    std::cout << "\n";

                    auto now = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - p.last_start).count();
                    
                    if (elapsed < 5) {
                        std::cout << "[SUPERVISOR] " << p.name << " restarted too fast. Backing off 5s...\n";
                        std::this_thread::sleep_for(std::chrono::seconds(5));
                    }

                    p.pid = spawn(p.binary, p.args);
                    p.last_start = std::chrono::steady_clock::now();
                    p.restart_count++;
                    std::cout << "[SUPERVISOR] " << p.name << " restarted (PID " << p.pid 
                              << ", restart #" << p.restart_count << ")\n";
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::lock_guard<std::mutex> lock(proc_mutex);
    for (auto& p : processes) {
        if (p.pid > 0) {
            std::cout << "[SUPERVISOR] Killing " << p.name << " (PID " << p.pid << ")...\n";
            kill(p.pid, SIGTERM);
            int status;
            waitpid(p.pid, &status, 0);
        }
    }
    std::cout << "[SUPERVISOR] All processes terminated.\n";
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "\033[1;32m";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  🛡️ PROTEUS SUPERVISOR — PROCESS LIFECYCLE MANAGER        ║\n";
    std::cout << "║  \"The shepherd. The watcher. The hand that restarts.\"     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << "\033[0m\n";

    // Verify binaries exist
    auto check = [](const std::string& path) {
        if (access(path.c_str(), X_OK) != 0) {
            std::cerr << "[SUPERVISOR] WARNING: " << path << " not found or not executable\n";
            return false;
        }
        return true;
    };

    if (check("./heartbeat_secure")) {
        processes.push_back({"Ghost", "./heartbeat_secure", {}, -1, 0, {}});
    }
    if (check("./zayden_gorf")) {
        processes.push_back({"Zayden", "./zayden_gorf", {}, -1, 0, {}});
    }

    for (auto& p : processes) {
        p.pid = spawn(p.binary, p.args);
        p.last_start = std::chrono::steady_clock::now();
        std::cout << "[SUPERVISOR] Started " << p.name << " (PID " << p.pid << ")\n";
    }

    std::cout << "[SUPERVISOR] Monitoring " << processes.size() << " processes\n";
    std::cout << "[SUPERVISOR] Press Ctrl+C to shutdown gracefully\n\n";

    monitor();

    std::cout << "[SUPERVISOR] Shutdown complete.\n";
    return 0;
}
