#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "morp.hpp"

struct ChaosState {
    double x, y, z;
    double load, temp;
    int noise;
    std::string archetype;
};

class ElMalo {
public:
    ElMalo() : x_(0.1), y_(0.0), z_(0.0), rng_(std::random_device{}()) {}
    void step() {
        const double s = 10.0, r = 28.0, b = 2.667, dt = 0.01;
        double dx = s * (y_ - x_);
        double dy = x_ * (r - z_) - y_;
        double dz = x_ * y_ - b * z_;
        x_ += dx * dt;
        y_ += dy * dt;
        z_ += dz * dt;
    }
    ChaosState synthesize() {
        step();
        ChaosState st;
        st.x = x_; st.y = y_; st.z = z_;
        uint16_t raw = 0;
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd >= 0) { read(fd, &raw, 2); close(fd); }
        st.noise = raw % 1000;
        auto now = std::chrono::system_clock::now().time_since_epoch();
        double t = std::chrono::duration<double>(now).count();
        std::uniform_real_distribution<double> jit(0.0, 1.0);
        double jitter = jit(rng_);
        st.load = std::fmod(jitter * t, 5.0);
        st.temp = 30.0 + jitter * 20.0;
        static const char* archetypes[] = {"warrior", "sage", "chaos", "order"};
        std::uniform_int_distribution<int> pick(0, 3);
        st.archetype = archetypes[pick(rng_)];
        return st;
    }
private:
    double x_, y_, z_;
    std::mt19937 rng_;
};

std::string to_json(const ChaosState& s) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6)
        << "{\"lorenz\":[" << s.x << "," << s.y << "," << s.z << "],"
        << "\"entropy\":{\"load\":" << s.load
        << ",\"temp\":" << s.temp
        << ",\"noise\":" << s.noise << "},"
        << "\"archetype\":\"" << s.archetype << "\"}";
    return oss.str();
}

int main(int argc, char** argv) {
    int port = 15011;
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { std::cerr << "[chaos] socket failed\n"; return 1; }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[chaos] BIND FAILED on port " << port << "\n";
        return 1;
    }
    listen(sock, 8);
    std::cout << "[chaos] MORP-v2 listening on port " << port << std::endl;

    ElMalo engine;
    while (true) {
        int fd = accept(sock, nullptr, nullptr);
        if (fd < 0) continue;
        uint8_t buf[8192];
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n >= (ssize_t)morp::FRAME_MIN) {
            uint8_t type = buf[5];
            ChaosState st = engine.synthesize();
            if (type == morp::MORP_MSG_PROBE || type == morp::MORP_MSG_HEARTBEAT) {
                auto resp = morp::encode(morp::MORP_MSG_ACK);
                send(fd, resp.data(), resp.size(), 0);
            } else if (type == morp::MORP_MSG_COMMAND) {
                std::string j = to_json(st);
                std::vector<uint8_t> pl(j.begin(), j.end());
                auto resp = morp::encode(morp::MORP_MSG_ACK, pl);
                send(fd, resp.data(), resp.size(), 0);
            }
        }
        close(fd);
    }
}
