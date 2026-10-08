#include <iostream>
#include <string>
#include <map>
#include <mutex>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "morp.hpp"

static int64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

static std::string local_ip() {
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) return "127.0.0.1";
    sockaddr_in a{}; a.sin_family = AF_INET;
    a.sin_port = htons(80);
    inet_pton(AF_INET, "8.8.8.8", &a.sin_addr);
    if (connect(s, (sockaddr*)&a, sizeof(a)) < 0) { close(s); return "127.0.0.1"; }
    sockaddr_in loc{}; socklen_t l = sizeof(loc);
    getsockname(s, (sockaddr*)&loc, &l); close(s);
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &loc.sin_addr, buf, sizeof(buf));
    return std::string(buf);
}

class Discovery {
    struct Peer { std::string ip; int tcp_port; int64_t last_seen; };
    std::map<std::string, Peer> peers;
    std::mutex mtx;
    std::string node_id, my_ip;
    int tcp_port, udp_port;

    void join_remote(const std::string& ip, int port) {
        int s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) return;
        sockaddr_in a{}; a.sin_family = AF_INET;
        a.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &a.sin_addr);
        timeval tv{2, 0};
        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        if (connect(s, (sockaddr*)&a, sizeof(a)) < 0) { close(s); return; }
        std::string p = "JOIN:" + node_id + "@" + my_ip;
        std::vector<uint8_t> pl(p.begin(), p.end());
        auto f = morp::encode(morp::MORP_MSG_COMMAND, pl);
        send(s, f.data(), f.size(), 0);
        uint8_t buf[512]; recv(s, buf, sizeof(buf), 0); close(s);
    }

public:
    Discovery(const std::string& id, int tcp, int udp)
        : node_id(id), my_ip(local_ip()), tcp_port(tcp), udp_port(udp) {}

    void add_peer(const std::string& id, const std::string& ip, int tcp) {
        bool is_new;
        { std::lock_guard<std::mutex> l(mtx);
          is_new = peers.find(id) == peers.end();
          peers[id] = {ip, tcp, now_ms()}; }
        if (is_new) {
            std::cout << "[discovery] NEW PEER: " << id << " @ " << ip << ":" << tcp << std::endl;
            std::thread([this, id, ip, tcp]() { join_remote(ip, tcp); }).detach();
        }
    }

    void expire() {
        std::lock_guard<std::mutex> l(mtx);
        int64_t cutoff = now_ms() - 30000;
        for (auto it = peers.begin(); it != peers.end(); ) {
            if (it->second.last_seen < cutoff) {
                std::cout << "[discovery] peer expired: " << it->first << std::endl;
                it = peers.erase(it);
            } else ++it;
        }
    }

    std::string peers_json() {
        std::lock_guard<std::mutex> l(mtx);
        std::ostringstream o;
        o << "{\"self\":\"" << node_id << "\",\"ip\":\"" << my_ip << "\",\"peers\":[";
        bool first = true;
        for (auto& [id, p] : peers) {
            if (!first) o << ",";
            o << "{\"id\":\"" << id << "\",\"ip\":\"" << p.ip
              << "\",\"port\":" << p.tcp_port << "}";
            first = false;
        }
        o << "]}";
        return o.str();
    }

    void broadcast_loop() {
        int s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0) return;
        int opt = 1;
        setsockopt(s, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
        sockaddr_in b{}; b.sin_family = AF_INET;
        b.sin_port = htons(udp_port);
        b.sin_addr.s_addr = inet_addr("255.255.255.255");
        std::cout << "[discovery] broadcasting as " << node_id << " @" << my_ip << std::endl;
        while (true) {
            std::string msg = "MORP-DISCOVER|" + node_id + "|" + my_ip + "|" + std::to_string(tcp_port);
            sendto(s, msg.data(), msg.size(), 0, (sockaddr*)&b, sizeof(b));
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    void listen_loop() {
        int s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0) return;
        int opt = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        setsockopt(s, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
        sockaddr_in a{}; a.sin_family = AF_INET;
        a.sin_addr.s_addr = INADDR_ANY;
        a.sin_port = htons(udp_port);
        if (bind(s, (sockaddr*)&a, sizeof(a)) < 0) {
            std::cerr << "[discovery] bind failed on UDP " << udp_port << std::endl;
            return;
        }
        std::cout << "[discovery] listening on UDP " << udp_port << std::endl;
        while (true) {
            char buf[2048]; sockaddr_in from{}; socklen_t fl = sizeof(from);
            ssize_t n = recvfrom(s, buf, sizeof(buf) - 1, 0, (sockaddr*)&from, &fl);
            if (n <= 0) continue;
            buf[n] = 0;
            std::string msg(buf);
            if (msg == "GOSSIP_PROBE") {
                std::string r = "GOSSIP_ACK|" + node_id;
                sendto(s, r.data(), r.size(), 0, (sockaddr*)&from, fl);
            } else if (msg == "PEERS") {
                std::string j = peers_json();
                sendto(s, j.data(), j.size(), 0, (sockaddr*)&from, fl);
            } else if (msg.rfind("MORP-DISCOVER|", 0) == 0) {
                std::string rest = msg.substr(14);
                size_t p1 = rest.find('|');
                size_t p2 = rest.find('|', p1 + 1);
                if (p1 == std::string::npos || p2 == std::string::npos) continue;
                std::string id = rest.substr(0, p1);
                std::string ip = rest.substr(p1 + 1, p2 - p1 - 1);
                int tcp = atoi(rest.substr(p2 + 1).c_str());
                if (id == node_id) continue;
                add_peer(id, ip, tcp);
            }
        }
    }
};

int main(int argc, char** argv) {
    int udp_port = 15008;
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
            udp_port = atoi(argv[++i]);

    const char* id_env = getenv("PK_NODE_ID");
    char host[256]; gethostname(host, sizeof(host));
    std::string node_id = id_env ? id_env : std::string(host);

    Discovery d(node_id, 15001, udp_port);
    std::thread([&d]() { d.broadcast_loop(); }).detach();
    std::thread([&d]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            d.expire();
        }
    }).detach();
    d.listen_loop();
    return 0;
}
