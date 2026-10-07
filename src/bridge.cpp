#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include "morp.hpp"

class ProteusKernelBridge {
public:
    ProteusKernelBridge() : sx_(1.0), sy_(1.0), gen_(0), rng_(std::random_device{}()) {}
    void rotate() {
        const double theta = 2.0 * M_PI / 9.0;
        double c = std::cos(theta), s = std::sin(theta);
        double nx = sx_ * c - sy_ * s;
        double ny = sx_ * s + sy_ * c;
        sx_ = nx; sy_ = ny;
    }
    double drift_spike() {
        std::uniform_real_distribution<double> drift(-0.02, 0.02);
        std::uniform_real_distribution<double> z(0.0, 1.0);
        std::uniform_real_distribution<double> spike(-0.2, 0.2);
        double v = drift(rng_);
        if (z(rng_) < 0.05) v += spike(rng_);
        return v;
    }
    std::string process() {
        gen_++;
        rotate();
        sx_ += drift_spike();
        sy_ += drift_spike();
        bool anomaly = std::abs(sx_) > 1.5 || std::abs(sy_) > 1.5;
        std::uniform_int_distribution<int> hb(60, 120);
        int heartbeat = hb(rng_);
        std::ostringstream pre;
        pre << gen_ << sy_;
        std::string pre_str = pre.str();
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)pre_str.c_str(), pre_str.size(), hash);
        std::ostringstream hex;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
            hex << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        double ts = (double)std::time(nullptr);
        std::ostringstream out;
        out << std::fixed << std::setprecision(6)
            << "{\"gen\":" << gen_
            << ",\"swarm_state\":{"
            << "\"order\":" << sx_
            << ",\"chaos\":" << sy_
            << ",\"heartbeat\":" << heartbeat
            << ",\"anomaly\":" << (anomaly ? "true" : "false") << "}"
            << ",\"holo_hash\":\"" << hex.str() << "\""
            << ",\"timestamp\":" << ts << "}";
        return out.str();
    }
private:
    double sx_, sy_;
    int gen_;
    std::mt19937 rng_;
};

int main(int argc, char** argv) {
    int port = 15012;
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { std::cerr << "[bridge] socket failed\n"; return 1; }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[bridge] BIND FAILED on port " << port << "\n";
        return 1;
    }
    listen(sock, 8);
    std::cout << "[bridge] MORP-v2 listening on port " << port << std::endl;
    ProteusKernelBridge bridge;
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
                std::string j = bridge.process();
                std::vector<uint8_t> pl(j.begin(), j.end());
                auto resp = morp::encode(morp::MORP_MSG_ACK, pl);
                send(fd, resp.data(), resp.size(), 0);
            }
        }
        close(fd);
    }
}
