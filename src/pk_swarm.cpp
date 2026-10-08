#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <mutex>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "morp.hpp"

class Swarm {
    std::set<std::string> peers;
    std::mutex mtx;
    int64_t start_ms;

    static int64_t now_ms() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

public:
    Swarm() : start_ms(now_ms()) {}

    void add_peer(const std::string& id) {
        std::lock_guard<std::mutex> l(mtx);
        peers.insert(id);
    }
    void remove_peer(const std::string& id) {
        std::lock_guard<std::mutex> l(mtx);
        peers.erase(id);
    }
    std::string stats() {
        std::lock_guard<std::mutex> l(mtx);
        std::string out = "{\"peers\":" + std::to_string(peers.size())
            + ",\"uptime_ms\":" + std::to_string(now_ms() - start_ms)
            + ",\"peer_ids\":[";
        bool first = true;
        for (const auto& p : peers) {
            if (!first) out += ",";
            out += "\"" + p + "\"";
            first = false;
        }
        out += "]}";
        return out;
    }
};

int main(int argc, char** argv) {
    int port = 15001;
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);

    Swarm swarm;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { std::cerr << "[pk_swarm] socket failed\n"; return 1; }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[pk_swarm] BIND FAILED on port " << port << "\n";
        return 1;
    }
    listen(sock, 8);
    std::cout << "[pk_swarm] MORP-v2 listening on port " << port << std::endl;

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
                if (payload.rfind("JOIN:", 0) == 0) {
                    swarm.add_peer(payload.substr(5));
                    std::string r = "{\"status\":\"joined\"}";
                    std::vector<uint8_t> pl(r.begin(), r.end());
                    auto resp = morp::encode(morp::MORP_MSG_ACK, pl);
                    send(fd, resp.data(), resp.size(), 0);
                } else if (payload.rfind("LEAVE:", 0) == 0) {
                    swarm.remove_peer(payload.substr(6));
                    std::string r = "{\"status\":\"left\"}";
                    std::vector<uint8_t> pl(r.begin(), r.end());
                    auto resp = morp::encode(morp::MORP_MSG_ACK, pl);
                    send(fd, resp.data(), resp.size(), 0);
                } else {
                    std::string s = swarm.stats();
                    std::vector<uint8_t> pl(s.begin(), s.end());
                    auto resp = morp::encode(morp::MORP_MSG_ACK, pl);
                    send(fd, resp.data(), resp.size(), 0);
                }
            }
        }
        close(fd);
    }
}
