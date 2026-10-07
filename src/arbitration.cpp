#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "morp.hpp"

struct UnifiedState {
    std::string archetype;
    double order, chaos;
    int heartbeat;
    bool anomaly;
    int gen;
};

std::string arbitrate(const UnifiedState& s) {
    std::string status = "STABLE";
    if (s.anomaly) status = "ANOMALOUS";
    else if (std::abs(s.order - s.chaos) < 0.05) status = "COHERENT";

    std::ostringstream oss;
    oss << "Gen: " << s.gen << "\n"
        << "Archetype: " << s.archetype << "\n"
        << std::fixed << std::setprecision(4)
        << "Order: " << s.order << "\n"
        << "Chaos: " << s.chaos << "\n"
        << "Heartbeat: " << s.heartbeat << "\n"
        << "Status: " << status << "\n"
        << "Anomaly: " << (s.anomaly ? "True" : "False") << "\n";
    return oss.str();
}

int main(int argc, char** argv) {
    int port = 15013;
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { std::cerr << "[arbitration] socket failed\n"; return 1; }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[arbitration] BIND FAILED on port " << port << "\n";
        return 1;
    }
    listen(sock, 8);
    std::cout << "[arbitration] MORP-v2 listening on port " << port << std::endl;

    while (true) {
        int fd = accept(sock, nullptr, nullptr);
        if (fd < 0) continue;

        uint8_t buf[8192];
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n < (ssize_t)morp::FRAME_MIN) { close(fd); continue; }

        uint8_t type = buf[5];
        if (type == morp::MORP_MSG_PROBE || type == morp::MORP_MSG_HEARTBEAT) {
            auto resp = morp::encode(morp::MORP_MSG_ACK);
            send(fd, resp.data(), resp.size(), 0);
        } else if (type == morp::MORP_MSG_COMMAND) {
            UnifiedState s{"order", -1.5650, 0.2416, 97, true, 1};
            std::string txt = arbitrate(s);
            std::vector<uint8_t> pl(txt.begin(), txt.end());
            auto resp = morp::encode(morp::MORP_MSG_ACK, pl);
            send(fd, resp.data(), resp.size(), 0);
        }
        close(fd);
    }
}
