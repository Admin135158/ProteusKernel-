#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "morp.hpp"

constexpr int PORT_CHAOS      = 15011;
constexpr int PORT_BRIDGE     = 15012;
constexpr int PORT_ARBITRATION= 15013;
constexpr int PORT_UDP_OUT    = 9164;

void udp_broadcast(const std::string& payload) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;
    sockaddr_in dst{};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(PORT_UDP_OUT);
    dst.sin_addr.s_addr = inet_addr("127.0.0.1");
    sendto(sock, payload.data(), payload.size(), 0, (sockaddr*)&dst, sizeof(dst));
    close(sock);
}

std::string call_engine(int port) {
    auto resp = morp::request(port, morp::MORP_MSG_COMMAND);
    if (resp.empty()) return "";
    return morp::payload_as_string(resp);
}

int main(int argc, char** argv) {
    int port = 15015;
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);

    // Bind the orchestrator port so the supervisor can probe us
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) { std::cerr << "[scs] socket failed\n"; return 1; }
    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(server, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[scs] BIND FAILED on port " << port << "\n";
        return 1;
    }
    listen(server, 8);
    std::cout << "[scs_main] MORP-v2 orchestrator listening on port " << port << std::endl;

    // Probe responder thread
    std::thread([server]() {
        while (true) {
            int fd = accept(server, nullptr, nullptr);
            if (fd < 0) continue;
            uint8_t buf[8192];
            ssize_t n = recv(fd, buf, sizeof(buf), 0);
            if (n >= (ssize_t)morp::FRAME_MIN && buf[5] == morp::MORP_MSG_PROBE) {
                auto resp = morp::encode(morp::MORP_MSG_ACK);
                send(fd, resp.data(), resp.size(), 0);
            }
            close(fd);
        }
    }).detach();

    // Orchestration loop
    int gen = 0;
    while (true) {
        gen++;
        std::string chaos_json  = call_engine(PORT_CHAOS);
        std::string bridge_json = call_engine(PORT_BRIDGE);
        std::string arb_summary = call_engine(PORT_ARBITRATION);

        // Compose a unified state blob
        std::ostringstream unified;
        unified << "{\"gen\":" << gen
                << ",\"chaos\":" << (chaos_json.empty() ? "{}" : chaos_json)
                << ",\"bridge\":" << (bridge_json.empty() ? "{}" : bridge_json)
                << "}";
        std::string blob = unified.str();

        udp_broadcast(blob);

        std::cout << "\n[FINAL OUTPUT]\n";
        if (!arb_summary.empty()) std::cout << arb_summary << std::endl;
        else std::cout << "(no arbitration response)" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
