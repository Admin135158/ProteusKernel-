#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <random>
#include <cstring>
#include <unistd.h>
#include <openssl/hmac.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "morp.hpp"

#define SECRET_FILE "node_secret.bin"
#define BANNED_FILE "banned_nodes.txt"

class Gatekeeper {
    std::vector<unsigned char> secret;
    std::map<std::string, std::vector<unsigned char>> known_peers;
    std::set<std::string> banned;
    std::mutex peer_mutex, ban_mutex;
    std::string node_id;

    bool generateSecret() {
        secret.resize(32);
        std::random_device rd;
        for (size_t i = 0; i < secret.size(); ++i) secret[i] = rd() & 0xFF;
        std::ofstream f(SECRET_FILE, std::ios::binary);
        if (!f) return false;
        f.write(reinterpret_cast<const char*>(secret.data()), secret.size());
        return f.good();
    }
    bool loadSecret() {
        std::ifstream f(SECRET_FILE, std::ios::binary);
        if (!f) return false;
        secret.resize(32);
        f.read(reinterpret_cast<char*>(secret.data()), secret.size());
        return f.gcount() == 32;
    }
    static std::string b64encode(const std::vector<unsigned char>& data) {
        static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out; int val = 0, valb = -6;
        for (unsigned char c : data) {
            val = (val << 8) + c; valb += 8;
            while (valb >= 0) { out.push_back(chars[(val >> valb) & 0x3F]); valb -= 6; }
        }
        if (valb > -6) out.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (out.size() % 4) out.push_back('=');
        return out;
    }
    static std::vector<unsigned char> b64decode(const std::string& in) {
        static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::vector<unsigned char> out; int val = 0, valb = -8;
        for (unsigned char c : in) {
            if (c == '=') break;
            size_t pos = chars.find(c);
            if (pos == std::string::npos) continue;
            val = (val << 6) + pos; valb += 6;
            if (valb >= 0) { out.push_back(char((val >> valb) & 0xFF)); valb -= 8; }
        }
        return out;
    }
public:
    Gatekeeper() {
        node_id = "gate_" + std::to_string(getpid());
        if (!loadSecret()) {
            if (!generateSecret()) { std::cerr << "[gatekeeper] secret gen failed\n"; exit(1); }
            std::cout << "[gatekeeper] Generated new HMAC secret\n";
        } else {
            std::cout << "[gatekeeper] Loaded existing secret\n";
        }
        std::ifstream bf(BANNED_FILE);
        std::string line;
        while (std::getline(bf, line)) if (!line.empty()) banned.insert(line);
        std::cout << "[gatekeeper] Node: " << node_id
                  << " | Banned: " << banned.size() << std::endl;
    }
    std::string secretB64() const { return b64encode(secret); }

    bool verify(const std::string& payload, const std::string& sigB64,
                const std::string& peerId, const std::string& peerSecretB64) {
        { std::lock_guard<std::mutex> l(ban_mutex);
          if (banned.count(peerId)) { std::cout << "[gatekeeper] REJECT banned " << peerId << "\n"; return false; } }
        auto ps = b64decode(peerSecretB64);
        if (ps.size() != 32) return false;
        unsigned int len = 0;
        unsigned char* mac = HMAC(EVP_sha256(), ps.data(), ps.size(),
            reinterpret_cast<const unsigned char*>(payload.c_str()), payload.length(), nullptr, &len);
        if (!mac || len == 0) return false;
        std::vector<unsigned char> expected(mac, mac + len);
        auto sig = b64decode(sigB64);
        if (sig.size() != expected.size() || !std::equal(sig.begin(), sig.end(), expected.begin())) {
            std::cout << "[gatekeeper] REJECT bad HMAC " << peerId << "\n";
            return false;
        }
        { std::lock_guard<std::mutex> l(peer_mutex); known_peers[peerId] = ps; }
        std::cout << "[gatekeeper] ACCEPT " << peerId << std::endl;
        return true;
    }
    void ban(const std::string& id) {
        std::lock_guard<std::mutex> l(ban_mutex);
        banned.insert(id);
        std::ofstream f(BANNED_FILE);
        for (auto& b : banned) f << b << "\n";
        std::cout << "[gatekeeper] BANNED " << id << std::endl;
    }
    size_t peerCount() { std::lock_guard<std::mutex> l(peer_mutex); return known_peers.size(); }
    size_t banCount()  { std::lock_guard<std::mutex> l(ban_mutex); return banned.size(); }
};

int main(int argc, char** argv) {
    int port = 15003;
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);

    Gatekeeper gk;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { std::cerr << "[gatekeeper] socket failed\n"; return 1; }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[gatekeeper] BIND FAILED on port " << port << "\n";
        return 1;
    }
    listen(sock, 8);
    std::cout << "[gatekeeper] MORP-v2 listening on port " << port << std::endl;

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
                std::string result = "{\"peers\":" + std::to_string(gk.peerCount())
                    + ",\"banned\":" + std::to_string(gk.banCount()) + "}";
                std::vector<uint8_t> pl(result.begin(), result.end());
                auto resp = morp::encode(morp::MORP_MSG_ACK, pl);
                send(fd, resp.data(), resp.size(), 0);
            }
        }
        close(fd);
    }
}
