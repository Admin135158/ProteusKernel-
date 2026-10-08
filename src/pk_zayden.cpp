#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "morp.hpp"

class AIGateway {
    std::mutex mtx;
    uint64_t prompts_seen = 0;
    std::string last_prompt;

public:
    void record(const std::string& p) {
        std::lock_guard<std::mutex> l(mtx);
        prompts_seen++;
        last_prompt = p.substr(0, 200);
    }
    std::string stats() {
        std::lock_guard<std::mutex> l(mtx);
        std::string lp = last_prompt;
        for (auto& c : lp) if (c == '"' || c == '\\') c = ' ';
        return "{\"prompts_seen\":" + std::to_string(prompts_seen)
             + ",\"last\":\"" + lp + "\"}";
    }
};

int main(int argc, char** argv) {
    int port = 15006;
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);

    AIGateway gw;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { std::cerr << "[pk_zayden] socket failed\n"; return 1; }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[pk_zayden] BIND FAILED on port " << port << "\n";
        return 1;
    }
    listen(sock, 8);
    std::cout << "[pk_zayden] MORP-v2 gateway listening on port " << port << std::endl;

    while (true) {
        int fd = accept(sock, nullptr, nullptr);
        if (fd < 0) continue;
        uint8_t buf[8192];
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n >= (ssize_t)morp::FRAME_MIN) {
            uint8_t type = buf[5];
            if (type == morp::MORP_MSG_PROBE || type == morp::MORP_MSG_HEARTBEAT) {
                auto resp = morp::encode(morp::MORP_MSG_ACK);
                send(fd, resp.data(), resp.size(), 0);
            } else if (type == morp::MORP_MSG_COMMAND) {
                std::string payload = morp::payload_as_string(std::vector<uint8_t>(buf, buf + n));
                std::string r;
                if (payload.rfind("ASK:", 0) == 0) {
                    gw.record(payload.substr(4));
                    // In production: forward to local LLM endpoint and return response
                    r = "{\"status\":\"received\",\"note\":\"llm_backend_not_configured\"}";
                } else {
                    r = gw.stats();
                }
                std::vector<uint8_t> pl(r.begin(), r.end());
                auto resp = morp::encode(morp::MORP_MSG_ACK, pl);
                send(fd, resp.data(), resp.size(), 0);
            }
        }
        close(fd);
    }
}
