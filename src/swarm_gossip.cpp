#include "morpheus_core.hpp"
using namespace morpheus;

int main(int argc, char** argv) {
    int port = 15008;
    for (int i = 1; i < argc; ++i) if (std::string(argv[i]) == "--port" && i+1 < argc) port = atoi(argv[i+1]);
    signal(SIGINT, on_sig); signal(SIGTERM, on_sig);
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    int opt = 1; setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(fd, (sockaddr*)&a, sizeof(a));
    std::cout << "[swarm_gossip] MORP-v2 UDP | PID:" << getpid() << " | Port:" << port << "\n";
    Protocol proto(getEnvSecret());
    uint8_t buf[65536];
    sockaddr_in cli; socklen_t clen = sizeof(cli);
    while (!g_shutdown) {
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, (sockaddr*)&cli, &clen);
        if (n < 44) continue;
        Frame f;
        if (!proto.decode(buf, n, f)) continue;
        std::string pl(f.payload.begin(), f.payload.end());
        std::string rep;
        if (f.type == GOSSIP || f.type == HEARTBEAT || pl.find("DISCOVER") != std::string::npos)
            rep = "GOSSIP-ACK|PEER_DISCOVERED|ACCEPTED|SYNC-7_COMPATIBLE";
        else
            rep = "GOSSIP-ACK|LISTENING";
        Frame out = Frame(); out.type = RESPONSE; out.payloadLen = rep.size();
        out.payload.assign(rep.begin(), rep.end());
        std::vector<uint8_t> pkt;
        proto.encode(out, pkt);
        sendto(fd, pkt.data(), pkt.size(), 0, (sockaddr*)&cli, clen);
    }
    close(fd); return 0;
}
