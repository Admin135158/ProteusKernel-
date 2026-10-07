#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <mutex>
#include <map>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

namespace morpheus {

static std::atomic<bool> g_shutdown{false};
static void on_sig(int) { g_shutdown = true; }

inline std::string getEnvSecret() {
    const char* s = getenv("PK_SECRET");
    return s ? s : "MORPHEUS_DEV_KEY_CHANGE_IN_PRODUCTION";
}

// ==================== SHA-256 ====================
class SHA256 {
    uint32_t h[8];
    uint8_t buf[64];
    size_t buf_len;
    uint64_t bit_len;
    static const uint32_t K[64];
    static uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    static uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    static uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    static uint32_t ep0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    static uint32_t ep1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    static uint32_t sig0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    static uint32_t sig1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }
    void transform(const uint8_t* data) {
        uint32_t W[64], a, b, c, d_, e, f, g, h_, t1, t2;
        for (int i = 0; i < 16; ++i)
            W[i] = (data[i * 4] << 24) | (data[i * 4 + 1] << 16) | (data[i * 4 + 2] << 8) | data[i * 4 + 3];
        for (int i = 16; i < 64; ++i)
            W[i] = sig1(W[i - 2]) + W[i - 7] + sig0(W[i - 15]) + W[i - 16];
        a = h[0]; b = h[1]; c = h[2]; d_ = h[3]; e = h[4]; f = h[5]; g = h[6]; h_ = h[7];
        for (int i = 0; i < 64; ++i) {
            t1 = h_ + ep1(e) + ch(e, f, g) + K[i] + W[i];
            t2 = ep0(a) + maj(a, b, c);
            h_ = g; g = f; f = e; e = d_ + t1; d_ = c; c = b; b = a; a = t1 + t2;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d_; h[4] += e; h[5] += f; h[6] += g; h[7] += h_;
    }
public:
    SHA256() { init(); }
    void init() {
        h[0] = 0x6a09e667; h[1] = 0xbb67ae85; h[2] = 0x3c6ef372; h[3] = 0xa54ff53a;
        h[4] = 0x510e527f; h[5] = 0x9b05688c; h[6] = 0x1f83d9ab; h[7] = 0x5be0cd19;
        bit_len = 0; buf_len = 0;
    }
    void update(const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            buf[buf_len++] = data[i];
            if (buf_len == 64) { transform(buf); buf_len = 0; bit_len += 512; }
        }
    }
    void final(uint8_t hash[32]) {
        uint64_t total_bits = bit_len + buf_len * 8;
        buf[buf_len++] = 0x80;
        if (buf_len > 56) {
            while (buf_len < 64) buf[buf_len++] = 0;
            transform(buf); buf_len = 0;
        }
        while (buf_len < 56) buf[buf_len++] = 0;
        for (int i = 7; i >= 0; --i) buf[55 + (7 - i)] = (total_bits >> (i * 8)) & 0xff;
        transform(buf);
        for (int i = 0; i < 8; ++i) {
            hash[i * 4] = (h[i] >> 24) & 0xff; hash[i * 4 + 1] = (h[i] >> 16) & 0xff;
            hash[i * 4 + 2] = (h[i] >> 8) & 0xff; hash[i * 4 + 3] = h[i] & 0xff;
        }
    }
    static std::string hex(const uint8_t hash[32]) {
        std::ostringstream oss;
        for (int i = 0; i < 32; ++i) oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        return oss.str();
    }
};
const uint32_t SHA256::K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

// ==================== HMAC-SHA256 ====================
inline void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* msg, size_t msglen, uint8_t out[32]) {
    uint8_t kbuf[64] = {0};
    if (keylen <= 64) memcpy(kbuf, key, keylen);
    else { SHA256 s; s.update(key, keylen); s.final(kbuf); }
    uint8_t ipad[64], opad[64];
    for (int i = 0; i < 64; ++i) { ipad[i] = kbuf[i] ^ 0x36; opad[i] = kbuf[i] ^ 0x5c; }
    uint8_t inner[32];
    SHA256 s1; s1.update(ipad, 64); s1.update(msg, msglen); s1.final(inner);
    SHA256 s2; s2.update(opad, 64); s2.update(inner, 32); s2.final(out);
}

// ==================== Protocol ====================
struct Frame {
    uint8_t version = 0x02;
    uint8_t type = 0x00;
    uint8_t flags = 0x00;
    uint8_t reserved = 0x00;
    uint32_t payloadLen = 0;
    std::vector<uint8_t> payload;
    uint8_t hmac[32] = {0};
};

enum MsgType : uint8_t {
    NOP=0x00, HEARTBEAT=0x01, AUTH_CHALLENGE=0x02, AUTH_RESPONSE=0x03,
    REGISTER=0x04, ROUTE=0x05, DATA=0x06, COMMAND=0x07,
    RESPONSE=0x08, ERROR=0x09, GOSSIP=0x0A, STATUS=0x0B
};

inline uint32_t be32(const uint8_t* p) { return (p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3]; }
inline void be32(uint8_t* p, uint32_t v) { p[0]=v>>24; p[1]=v>>16; p[2]=v>>8; p[3]=v; }

class Protocol {
    std::string secret_;
public:
    explicit Protocol(const std::string& secret) : secret_(secret) {}
    bool encode(const Frame& f, std::vector<uint8_t>& out) {
        out.resize(44 + f.payloadLen);
        memcpy(out.data(), "MORP", 4);
        out[4]=f.version; out[5]=f.type; out[6]=f.flags; out[7]=f.reserved;
        be32(out.data()+8, f.payloadLen);
        memset(out.data()+12, 0, 32);  // <-- ZERO HMAC REGION BEFORE SIGNING
        if (f.payloadLen) memcpy(out.data()+44, f.payload.data(), f.payloadLen);
        uint8_t sig[32];
        hmac_sha256((const uint8_t*)secret_.data(), secret_.size(), out.data(), out.size(), sig);
        memcpy(out.data()+12, sig, 32);
        return true;
    }
    bool decode(const uint8_t* data, size_t len, Frame& f) {
        if (len < 44) return false;
        if (memcmp(data, "MORP", 4) != 0) return false;
        f.version=data[4]; f.type=data[5]; f.flags=data[6]; f.reserved=data[7];
        f.payloadLen=be32(data+8);
        memcpy(f.hmac, data+12, 32);
        if (len < 44 + f.payloadLen) return false;
        f.payload.assign(data+44, data+44+f.payloadLen);
        std::vector<uint8_t> tmp(data, data+44+f.payloadLen);
        memset(tmp.data()+12, 0, 32);
        uint8_t sig[32];
        hmac_sha256((const uint8_t*)secret_.data(), secret_.size(), tmp.data(), tmp.size(), sig);
        return memcmp(sig, f.hmac, 32) == 0;
    }
};

// ==================== Journal ====================
class Journal {
    std::string path_;
    std::mutex mtx_;
public:
    explicit Journal(const std::string& path) : path_(path) {}
    bool append(const std::vector<uint8_t>& data) {
        std::lock_guard<std::mutex> lk(mtx_);
        std::ofstream f(path_, std::ios::app | std::ios::binary);
        if (!f) return false;
        uint8_t hdr[8];
        memcpy(hdr, "JNL!", 4);
        be32(hdr+4, (uint32_t)data.size());
        f.write((char*)hdr, 8);
        f.write((char*)data.data(), data.size());
        uint32_t trailer = (uint32_t)data.size();
        f.write((char*)&trailer, 4);
        return f.good();
    }
};

// ==================== Engine Base ====================
class Engine {
protected:
    std::string name_;
    int port_;
    int fd_;
    std::atomic<bool> run_{true};
    Protocol proto_;
    Journal journal_;
    bool readAll(int cfd, uint8_t* buf, size_t need) {
        size_t got = 0;
        while (got < need) {
            ssize_t n = read(cfd, buf+got, need-got);
            if (n <= 0) return false;
            got += n;
        }
        return true;
    }
    void clientLoop(int cfd) {
        while (run_ && !g_shutdown) {
            uint8_t hdr[44];
            if (!readAll(cfd, hdr, 44)) break;
            if (memcmp(hdr, "MORP", 4) != 0) { close(cfd); return; }
            uint32_t plen = be32(hdr+8);
            if (plen > 16*1024*1024) { close(cfd); return; }
            std::vector<uint8_t> full(44 + plen);
            memcpy(full.data(), hdr, 44);
            if (plen > 0 && !readAll(cfd, full.data()+44, plen)) break;
            Frame f;
            if (!proto_.decode(full.data(), full.size(), f)) { close(cfd); return; }
            onFrame(cfd, f);
        }
        close(cfd);
    }
    virtual void onFrame(int cfd, const Frame& f) = 0;
public:
    Engine(const std::string& name, int defaultPort, const std::string& secret, const std::string& journalPath)
        : name_(name), port_(defaultPort), fd_(-1), proto_(secret), journal_(journalPath) {}
    virtual ~Engine() { if (fd_ >= 0) close(fd_); }
    void start(int argc, char** argv) {
        for (int i = 1; i < argc; ++i)
            if (std::string(argv[i]) == "--port" && i+1 < argc) port_ = atoi(argv[i+1]);
        signal(SIGINT, on_sig);
        signal(SIGTERM, on_sig);
        fd_ = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(port_); a.sin_addr.s_addr = inet_addr("127.0.0.1");
        if (bind(fd_, (sockaddr*)&a, sizeof(a)) < 0) {
            std::cerr << "[" << name_ << "] BIND FAILED on port " << port_ << "\n"; return;
        }
        listen(fd_, 10);
        std::cout << "[" << name_ << "] MORP-v2 | PID:" << getpid() << " | Port:" << port_ << "\n";
        while (run_ && !g_shutdown) {
            int c = accept(fd_, nullptr, nullptr);
            if (c < 0) { if (errno == EINTR && !g_shutdown) continue; break; }
            std::thread(&Engine::clientLoop, this, c).detach();
        }
    }
    bool sendFrame(int cfd, const Frame& f) {
        std::vector<uint8_t> buf;
        if (!proto_.encode(f, buf)) return false;
        size_t sent = 0;
        while (sent < buf.size()) {
            ssize_t n = write(cfd, buf.data()+sent, buf.size()-sent);
            if (n < 0) return false;
            sent += n;
        }
        return true;
    }
    Frame mkFrame(MsgType t, const std::string& s) {
        Frame f; f.type = t; f.payloadLen = s.size();
        f.payload.assign(s.begin(), s.end());
        return f;
    }
};

} // namespace morpheus
