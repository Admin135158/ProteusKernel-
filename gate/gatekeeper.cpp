#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>
#include <cstring>
#include <unistd.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

#define SECRET_FILE "node_secret.bin"
#define BANNED_FILE "banned_nodes.txt"

class Gatekeeper {
private:
    std::vector<unsigned char> secret;
    std::map<std::string, std::vector<unsigned char>> known_peers;
    std::set<std::string> banned;
    std::mutex peer_mutex;
    std::mutex ban_mutex;
    std::string node_id;
    std::mt19937 rng;

    bool generateSecret() {
        secret.resize(32);
        std::random_device rd;
        for (size_t i = 0; i < secret.size(); ++i) {
            secret[i] = rd() & 0xFF;
        }
        return saveSecret();
    }

    bool saveSecret() {
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

    bool loadBanned() {
        std::ifstream f(BANNED_FILE);
        std::string line;
        while (std::getline(f, line)) {
            if (!line.empty()) banned.insert(line);
        }
        return true;
    }

    bool saveBanned() {
        std::ofstream f(BANNED_FILE);
        for (const auto& id : banned) {
            f << id << "\n";
        }
        return true;
    }

    std::string base64Encode(const std::vector<unsigned char>& data) {
        static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        int val = 0, valb = -6;
        for (unsigned char c : data) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                out.push_back(chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) out.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (out.size() % 4) out.push_back('=');
        return out;
    }

    std::vector<unsigned char> base64Decode(const std::string& in) {
        static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::vector<unsigned char> out;
        int val = 0, valb = -8;
        for (unsigned char c : in) {
            if (c == '=') break;
            size_t pos = chars.find(c);
            if (pos == std::string::npos) continue;
            val = (val << 6) + pos;
            valb += 6;
            if (valb >= 0) {
                out.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return out;
    }

public:
    Gatekeeper() : rng(std::random_device{}()) {
        node_id = "gate_" + std::to_string(getpid());
        if (!loadSecret()) {
            if (generateSecret()) {
                std::cout << "[GATEKEEPER] Generated new HMAC secret\n";
            } else {
                std::cerr << "[GATEKEEPER] Secret generation failed\n";
                exit(1);
            }
        } else {
            std::cout << "[GATEKEEPER] Loaded existing secret\n";
        }
        loadBanned();
        std::cout << "[GATEKEEPER] Node: " << node_id << "\n";
        std::cout << "[GATEKEEPER] Known peers: " << known_peers.size()
                  << " | Banned: " << banned.size() << "\n";
    }

    std::string getSecretBase64() {
        return base64Encode(secret);
    }

    std::string signPulse(const std::string& payload) {
        unsigned int len = 0;
        unsigned char* mac = HMAC(EVP_sha256(), secret.data(), secret.size(),
                                  reinterpret_cast<const unsigned char*>(payload.c_str()),
                                  payload.length(), nullptr, &len);
        if (!mac || len == 0) return "";
        std::vector<unsigned char> sig(mac, mac + len);
        return base64Encode(sig);
    }

    bool verifyPulse(const std::string& payload, const std::string& sigB64,
                     const std::string& peerId, const std::string& peerSecretB64) {
        {
            std::lock_guard<std::mutex> lock(ban_mutex);
            if (banned.count(peerId)) {
                std::cout << "[GATEKEEPER] REJECTED — banned node: " << peerId << "\n";
                return false;
            }
        }

        std::vector<unsigned char> peerSecret = base64Decode(peerSecretB64);
        if (peerSecret.size() != 32) {
            std::cout << "[GATEKEEPER] REJECTED — invalid secret length: " << peerId << "\n";
            return false;
        }

        unsigned int len = 0;
        unsigned char* mac = HMAC(EVP_sha256(), peerSecret.data(), peerSecret.size(),
                                  reinterpret_cast<const unsigned char*>(payload.c_str()),
                                  payload.length(), nullptr, &len);
        if (!mac || len == 0) return false;
        std::vector<unsigned char> expected(mac, mac + len);
        std::vector<unsigned char> sig = base64Decode(sigB64);

        if (sig.size() != expected.size() || !std::equal(sig.begin(), sig.end(), expected.begin())) {
            std::cout << "[GATEKEEPER] REJECTED — bad HMAC: " << peerId << "\n";
            return false;
        }

        {
            std::lock_guard<std::mutex> lock(peer_mutex);
            known_peers[peerId] = peerSecret;
        }

        std::cout << "[GATEKEEPER] ACCEPTED — verified: " << peerId << "\n";
        return true;
    }

    void banNode(const std::string& peerId) {
        std::lock_guard<std::mutex> lock(ban_mutex);
        banned.insert(peerId);
        saveBanned();
        std::cout << "[GATEKEEPER] BANNED: " << peerId << "\n";
    }

    void status() {
        std::lock_guard<std::mutex> lock1(peer_mutex);
        std::lock_guard<std::mutex> lock2(ban_mutex);
        std::cout << "\n═══════════════════════════════════════════════\n";
        std::cout << "  GATEKEEPER STATUS\n";
        std::cout << "═══════════════════════════════════════════════\n";
        std::cout << "  Node: " << node_id << "\n";
        std::cout << "  Known peers: " << known_peers.size() << "\n";
        std::cout << "  Banned: " << banned.size() << "\n";
        for (const auto& id : banned) {
            std::cout << "    ❌ " << id << "\n";
        }
        std::cout << "═══════════════════════════════════════════════\n";
    }
};

int main() {
    Gatekeeper gk;
    std::string secret = gk.getSecretBase64();
    if (secret.empty()) {
        std::cerr << "[FAIL] Could not export secret\n";
        return 1;
    }
    std::cout << "[TEST] Secret: " << secret.substr(0, 32) << "...\n";

    std::string payload = "[CYCLE 1] node_1234 pulses. Ψ=86.3853%";
    std::string sig = gk.signPulse(payload);
    if (sig.empty()) {
        std::cerr << "[FAIL] Could not sign\n";
        return 1;
    }
    std::cout << "[TEST] HMAC: " << sig.substr(0, 32) << "...\n";

    bool ok = gk.verifyPulse(payload, sig, "node_1234", secret);
    std::cout << "[TEST] Self-verify: " << (ok ? "PASS" : "FAIL") << "\n";

    bool bad = gk.verifyPulse(payload + "X", sig, "node_1234", secret);
    std::cout << "[TEST] Tamper-verify: " << (bad ? "FAIL (wrong)" : "PASS (rejected)") << "\n";

    gk.status();
    return 0;
}
