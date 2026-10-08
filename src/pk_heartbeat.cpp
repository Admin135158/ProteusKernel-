#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "morp.hpp"

class Heartbeat {
    std::atomic<uint64_t> beat_count{0};
    std::atomic<int64_t> last_beat_ms{0};
    std::mutex mtx;

    static int64_t now_ms() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

public:
    void tick() {
        beat_count++;
        last_beat_ms = now_ms();
        std::cout << "[heartbeat] tick #" << beat_count.load()
                  << " at " << last_beat_ms.load() << std::endl;
    }

    std::string stats() {
        std::lock_guard<std::mutex> l(mtx);
        return "{\"beats\":" + std::to_string(beat_count.load())
             + ",\"last_ms\":" + std::to_string(last_beat_ms.load()) + "}";
    }
};

int main(int argc, char** argv) {
    int port = 15002;
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);

    Heartbeat hb;

    // Tick thread — every 2 seconds
    std::thread([&hb]() {
        while (true) {
            hb.tick();
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }).detach();

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { std::cerr << "[heartbeat] socket failed\n"; return 1; }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[heartbeat] BIND FAILED on port " << port << "\n";
        return 1;
    }
    listen(sock, 8);
    std::cout << "[heartbeat] MORP-v2 listening on port " << port << std::endl;

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
                std::string s = hb.stats();
                std::vector<uint8_t> pl(s.begin(), s.end());
                auto resp = morp::encode(morp::MORP_MSG_ACK, pl);
                send(fd, resp.data(), resp.size(), 0);
            }
        }
        close(fd);
    }
}
