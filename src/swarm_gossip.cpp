#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "morp.hpp"

int main(int argc, char** argv) {
    int port = 15008;
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { std::cerr << "[gossip] socket failed\n"; return 1; }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[gossip] BIND FAILED on port " << port << "\n";
        return 1;
    }
    std::cout << "[gossip] UDP listener active on port " << port << std::endl;

    while (true) {
        uint8_t buf[8192];
        sockaddr_in from{};
        socklen_t fromlen = sizeof(from);
        ssize_t n = recvfrom(sock, buf, sizeof(buf), 0,
                             (sockaddr*)&from, &fromlen);
        if (n <= 0) continue;

        // Reply with a signed MORP ACK frame
        auto resp = morp::encode(morp::MORP_MSG_ACK);
        sendto(sock, resp.data(), resp.size(), 0,
               (sockaddr*)&from, fromlen);
    }
}
