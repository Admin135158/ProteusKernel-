/*
 * SPDX-License-Identifier: Proprietary
 * Copyright (c) 2026 Fernando De Jesus Garcia Gonzalez (The Architect)
 */
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <csignal>
#include <thread>
#include <chrono>
#include <atomic>

struct Process {
    std::string name;
    std::string path;
    pid_t pid;
    bool enabled;
};

std::vector<Process> processes;
std::atomic<bool> g_running(true);

void signal_handler(int sig) {
    std::cout << "\n[SUPERVISOR] Caught signal " << sig << ". Shutting down...\n";
    g_running = false;
}

bool file_exists_and_executable(const std::string& path) {
    return access(path.c_str(), X_OK) == 0;
}

std::string find_binary(const std::string& name) {
    std::vector<std::string> paths = {
        "./" + name,
        "./build/" + name,
        "./src/" + name,
        "./mesh/" + name,
        "./gate/" + name
    };
    for (const auto& p : paths) {
        if (file_exists_and_executable(p)) return p;
    }
    return "";
}

pid_t start_process(const std::string& name, const std::string& path) {
    pid_t pid = fork();
    if (pid == 0) {
        execl(path.c_str(), path.c_str(), nullptr);
        std::cerr << "[SUPERVISOR] Failed to start " << name << " from " << path
                  << ": " << strerror(errno) << "\n";
        exit(1);
    } else if (pid > 0) {
        std::cout << "[SUPERVISOR] Started " << name << " (PID: " << pid << ")\n";
        return pid;
    }
    return -1;
}

void monitor_loop() {
    while (g_running) {
        for (auto& proc : processes) {
            if (!proc.enabled || proc.path.empty()) continue;

            if (proc.pid > 0) {
                int status;
                pid_t result = waitpid(proc.pid, &status, WNOHANG);
                if (result == proc.pid) {
                    std::cout << "[SUPERVISOR] " << proc.name << " (PID " << proc.pid
                              << ") exited. Restarting...\n";
                    proc.pid = start_process(proc.name, proc.path);
                } else if (result == -1) {
                    std::cout << "[SUPERVISOR] " << proc.name << " lost. Restarting...\n";
                    proc.pid = start_process(proc.name, proc.path);
                }
            } else {
                proc.pid = start_process(proc.name, proc.path);
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    std::cout << "[SUPERVISOR] All processes terminating...\n";
    for (auto& proc : processes) {
        if (proc.pid > 0) {
            kill(proc.pid, SIGTERM);
        }
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    for (auto& proc : processes) {
        if (proc.pid > 0) {
            int status;
            pid_t result = waitpid(proc.pid, &status, WNOHANG);
            if (result == 0) {
                kill(proc.pid, SIGKILL);
            }
        }
    }
    std::cout << "[SUPERVISOR] Shutdown complete.\n";
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  🛡️ PROTEUS SUPERVISOR — PROCESS LIFECYCLE MANAGER        ║\n";
    std::cout << "║  \"The shepherd. The watcher. The hand that restarts.\"     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";

    std::vector<std::string> names = {"pk_heartbeat", "pk_zayden", "zayden_ultimate"};
    for (const auto& name : names) {
        std::string path = find_binary(name);
        Process p;
        p.name = name;
        p.path = path;
        p.pid = -1;
        p.enabled = true;
        if (path.empty()) {
            std::cout << "[SUPERVISOR] WARNING: " << name << " not found or not executable\n";
            p.enabled = false;
        } else {
            std::cout << "[SUPERVISOR] Found " << name << " at " << path << "\n";
        }
        processes.push_back(p);
    }

    int enabled_count = 0;
    for (const auto& p : processes) if (p.enabled) enabled_count++;
    std::cout << "[SUPERVISOR] Monitoring " << enabled_count << " processes\n";
    std::cout << "[SUPERVISOR] Press Ctrl+C to shutdown gracefully\n\n";

    monitor_loop();
    return 0;
}

