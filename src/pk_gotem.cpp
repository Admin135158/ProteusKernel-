#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "morp.hpp"

class StateQueue {
    std::deque<std::string> queue;
    std::mutex mtx;
    static constexpr size_t MAX_QUEUE = 1000;
    uint64_t pushed = 0;
    uint64_t popped = 0;

public:
    void push(const std::string& s) {
        std::lock_guard<std::mutex> l(mtx);
        if (queue.size() >= MAX_QUEUE) queue.pop_front();
        queue.push_back(s);
        pushed++;
    }
    std::string pop() {
        std::lock_guard<std::mutex> l(mtx);
        if (queue.empty()) return "";
        std::string s = queue.front();
        queue.pop_front();
        popped++;
        return s;
    }
    std::string stats() {
        std::lock_guard<std::mutex> l(mtx);
        return "{\"queued\":" + std::to_string(queue.size())
             + ",\"pushed\":" + std::to_string(pushed)
             + ",\"popped\":" + std::to_string(popped) + "}";
    }
};

int main(int argc, char** argv) {
    int port = 15007;
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);

    StateQueue q;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { std::cerr << "[pk_gotem] socket failed\n"; return 1; }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[pk_gotem] BIND FAILED on port " << port << "\n";
        return 1;
    }
    listen(sock, 8);
    std::cout << "[pk_gotem] MORP-v2 listening on port " << port << std::endl;

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
                if (payload.rfind("PUSH:", 0) == 0) {
                    q.push(payload.substr(5));
                    r = "{\"status\":\"queued\"}";
                } else if (payload == "POP") {
                    std::string item = q.pop();
                    r = item.empty() ? "{\"status\":\"empty\"}"
                                     : "{\"item\":\"" + item + "\"}";
                } else {
                    r = q.stats();
                }
                std::vector<uint8_t> pl(r.begin(), r.end());
                auto resp = morp::encode(morp::MORP_MSG_ACK, pl);
                send(fd, resp.data(), resp.size(), 0);
            }
        }
        close(fd);
    }
}
