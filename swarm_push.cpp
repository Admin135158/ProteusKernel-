#include <random>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

#define CHUNK_SIZE 4096
#define PORT 9163

const char BASES[] = {'A', 'C', 'G', 'T'};

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

std::string encode_bytes(const std::vector<unsigned char>& data) {
    std::string dna;
    dna.reserve(data.size() * 4);
    for (unsigned char c : data) {
        dna += BASES[(c >> 6) & 0x03];
        dna += BASES[(c >> 4) & 0x03];
        dna += BASES[(c >> 2) & 0x03];
        dna += BASES[c & 0x03];
    }
    return dna;
}

std::vector<unsigned char> decode_dna(const std::string& dna) {
    std::vector<unsigned char> data;
    if (dna.length() % 4 != 0) return {};
    data.reserve(dna.length() / 4);
    for (size_t i = 0; i < dna.length(); i += 4) {
        unsigned char c = 0;
        for (int j = 0; j < 4; ++j) {
            c <<= 2;
            char base = dna[i + j];
            if (base == 'A') c |= 0x00;
            else if (base == 'C') c |= 0x01;
            else if (base == 'G') c |= 0x02;
            else if (base == 'T') c |= 0x03;
            else return {};
        }
        data.push_back(c);
    }
    return data;
}

std::vector<unsigned char> read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    return std::vector<unsigned char>(
        (std::istreambuf_iterator<char>(f)),
        std::istreambuf_iterator<char>()
    );
}

std::string hmac(const std::string& msg, const std::vector<unsigned char>& key) {
    unsigned int len = 0;
    unsigned char* mac = HMAC(EVP_sha256(), key.data(), key.size(),
                              reinterpret_cast<const unsigned char*>(msg.c_str()),
                              msg.length(), nullptr, &len);
    if (!mac || len == 0) return "";
    std::vector<unsigned char> sig(mac, mac + len);
    return base64Encode(sig);
}

bool send_file(const std::string& path, const std::string& ip, const std::vector<unsigned char>& key) {
    auto data = read_file(path);
    if (data.empty()) {
        std::cerr << "[PUSH] Cannot read: " << path << "\n";
        return false;
    }

    std::string dna = encode_bytes(data);
    std::string signature = hmac(dna, key);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return false; }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    std::cout << "[PUSH] Connecting to " << ip << ":" << PORT << "...\n";
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return false;
    }

    // Header: filename | DNA length | signature
    std::ostringstream header;
    header << path << "|" << dna.length() << "|" << signature << "\n";
    std::string h = header.str();
    send(fd, h.c_str(), h.length(), 0);

    // Send DNA in chunks
    size_t sent = 0;
    while (sent < dna.length()) {
        size_t chunk = std::min(static_cast<size_t>(CHUNK_SIZE), dna.length() - sent);
        ssize_t n = send(fd, dna.c_str() + sent, chunk, 0);
        if (n < 0) { perror("send"); close(fd); return false; }
        sent += n;
        std::cout << "[PUSH] Sent " << sent << "/" << dna.length() << " bases\r" << std::flush;
    }
    std::cout << "\n[PUSH] Transfer complete: " << path << " -> " << ip << "\n";

    // Wait for ACK
    char ack[16];
    int n = recv(fd, ack, sizeof(ack), 0);
    if (n > 0) {
        ack[n] = '\0';
        std::cout << "[PUSH] Peer response: " << ack << "\n";
    }

    close(fd);
    return true;
}

bool receive_file() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return false; }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(listen_fd); return false;
    }

    if (listen(listen_fd, 1) < 0) {
        perror("listen"); close(listen_fd); return false;
    }

    std::cout << "[PULL] Listening on TCP " << PORT << "...\n";

    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int fd = accept(listen_fd, (struct sockaddr*)&client, &client_len);
    if (fd < 0) { perror("accept"); close(listen_fd); return false; }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client.sin_addr, client_ip, INET_ADDRSTRLEN);
    std::cout << "[PULL] Connection from " << client_ip << "\n";

    // Read header
    std::string header;
    char c;
    while (recv(fd, &c, 1, 0) == 1 && c != '\n') {
        header += c;
    }

    size_t p1 = header.find('|');
    size_t p2 = header.find('|', p1 + 1);
    if (p1 == std::string::npos || p2 == std::string::npos) {
        std::cerr << "[PULL] Invalid header\n";
        close(fd); close(listen_fd); return false;
    }

    std::string filename = header.substr(0, p1);
    size_t dna_len = std::stoul(header.substr(p1 + 1, p2 - p1 - 1));
    std::string signature = header.substr(p2 + 1);

    std::cout << "[PULL] Receiving " << filename << " (" << dna_len << " bases)...\n";

    // Read DNA
    std::string dna;
    dna.reserve(dna_len);
    char buffer[CHUNK_SIZE];
    while (dna.length() < dna_len) {
        ssize_t n = recv(fd, buffer, std::min(static_cast<size_t>(CHUNK_SIZE), dna_len - dna.length()), 0);
        if (n <= 0) break;
        dna.append(buffer, n);
        std::cout << "[PULL] Received " << dna.length() << "/" << dna_len << " bases\r" << std::flush;
    }
    std::cout << "\n";

    if (dna.length() != dna_len) {
        std::cerr << "[PULL] Incomplete transfer\n";
        close(fd); close(listen_fd); return false;
    }

    // Decode
    auto binary = decode_dna(dna);
    if (binary.empty()) {
        std::cerr << "[PULL] DNA decode failed\n";
        close(fd); close(listen_fd); return false;
    }

    // Write file
    std::string outname = "received_" + filename;
    std::ofstream out(outname, std::ios::binary);
    out.write(reinterpret_cast<const char*>(binary.data()), binary.size());
    out.close();

    chmod(outname.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    std::cout << "[PULL] Saved: " << outname << " (" << binary.size() << " bytes)\n";

    // ACK
    std::string ack = "OK|" + std::to_string(binary.size());
    send(fd, ack.c_str(), ack.length(), 0);

    close(fd);
    close(listen_fd);

    // Execute
    std::cout << "[PULL] Executing: " << outname << "\n";
    pid_t pid = fork();
    if (pid == 0) {
        execl(("./" + outname).c_str(), outname.c_str(), nullptr);
        perror("exec");
        exit(1);
    } else if (pid > 0) {
        std::cout << "[PULL] Spawned (PID: " << pid << ")\n";
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [push|receive] [file] [ip]\n";
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "push") {
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0] << " push [file] [ip]\n";
            return 1;
        }
        std::string file = argv[2];
        std::string ip = argv[3];

        // Load secret from file or generate
        std::vector<unsigned char> key(32);
        std::ifstream sf("node_secret.bin", std::ios::binary);
        if (sf) {
            sf.read(reinterpret_cast<char*>(key.data()), 32);
        } else {
            std::random_device rd;
            for (size_t i = 0; i < 32; ++i) key[i] = rd() & 0xFF;
        }

        return send_file(file, ip, key) ? 0 : 1;
    }

    if (mode == "receive") {
        return receive_file() ? 0 : 1;
    }

    std::cerr << "Unknown mode: " << mode << "\n";
    return 1;
}
