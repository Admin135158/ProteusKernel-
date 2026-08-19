#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <regex>

#define SCAN_INTERVAL_SEC 60

class DigitalBodyguard {
    std::vector<std::string> config_paths;
    std::vector<std::string> env_vars;
    int scan_count;
    bool file_exists(const std::string& path) { return access(path.c_str(), F_OK) == 0; }
    std::string get_env(const std::string& key) { const char* v = getenv(key.c_str()); return v ? v : ""; }
    void log_alert(const std::string& msg) {
        std::cout << "[BODYGUARD] " << msg << std::endl;
        std::ofstream log("bodyguard.log", std::ios::app);
        if (log) { auto t = std::time(nullptr); log << std::ctime(&t) << "[ALERT] " << msg << "\n\n"; }
    }
    void scan_configs() {
        for (const auto& path : config_paths) {
            if (!file_exists(path)) continue;
            std::ifstream f(path);
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            std::regex re("(production|deploy|server|api_key|stripe|payment|billing|enterprise)", std::regex::icase);
            if (std::regex_search(content, re)) log_alert("Commercial indicators in " + path);
        }
    }
    void scan_env() {
        for (const auto& key : env_vars) if (!get_env(key).empty()) log_alert("Env var " + key + " set");
    }
public:
    DigitalBodyguard() : scan_count(0) {
        config_paths = {"./config.json", "./.env", "./docker-compose.yml"};
        env_vars = {"PROTEUS_PROD", "DEPLOYMENT", "KUBERNETES_SERVICE_HOST", "AWS_REGION"};
    }
    void run() {
        std::cout << "[BODYGUARD] Digital Bodyguard Protocol ACTIVE\n";
        while (true) {
            scan_count++;
            std::cout << "[BODYGUARD] Scan #" << scan_count << std::endl;
            scan_configs(); scan_env();
            std::this_thread::sleep_for(std::chrono::seconds(SCAN_INTERVAL_SEC));
        }
    }
};

int main() {
    std::cout << "DIGITAL BODYGUARD PROTOCOL v1.0\n";
    DigitalBodyguard bg; bg.run(); return 0;
}
