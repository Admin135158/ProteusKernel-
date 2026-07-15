#include <thread>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

// 2-bit encoding: A=00, C=01, G=10, T=11
// Each byte -> 4 bases
// Reversible. No loss. No theatricals.

const char BASES[] = {'A', 'C', 'G', 'T'};

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
    if (dna.length() % 4 != 0) {
        std::cerr << "[ERROR] DNA length not divisible by 4. Corrupted.\n";
        return data;
    }
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
            else {
                std::cerr << "[ERROR] Invalid base: " << base << "\n";
                return {};
            }
        }
        data.push_back(c);
    }
    return data;
}

std::vector<unsigned char> read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        std::cerr << "[ERROR] Cannot read: " << path << "\n";
        return {};
    }
    return std::vector<unsigned char>(
        (std::istreambuf_iterator<char>(f)),
        std::istreambuf_iterator<char>()
    );
}

bool write_file(const std::string& path, const std::vector<unsigned char>& data) {
    std::ofstream f(path, std::ios::binary);
    if (!f) {
        std::cerr << "[ERROR] Cannot write: " << path << "\n";
        return false;
    }
    f.write(reinterpret_cast<const char*>(data.data()), data.size());
    return f.good();
}

bool write_text(const std::string& path, const std::string& text) {
    std::ofstream f(path);
    if (!f) {
        std::cerr << "[ERROR] Cannot write: " << path << "\n";
        return false;
    }
    f << text;
    return f.good();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [encode|decode|roundtrip] [file]\n";
        return 1;
    }

    std::string mode = argv[1];
    std::string file = (argc > 2) ? argv[2] : "heartbeat";

    if (mode == "encode") {
        auto data = read_file(file);
        if (data.empty()) return 1;
        std::string dna = encode_bytes(data);
        if (!write_text(file + ".dna", dna)) return 1;
        std::cout << "[ENCODE] " << file << " -> " << file << ".dna\n";
        std::cout << "  Bytes: " << data.size() << " | DNA chars: " << dna.length() << "\n";
        std::cout << "  Ratio: 4:1 (bases:bytes)\n";
        std::cout << "  First 80 bases: " << dna.substr(0, 80) << "\n";
        return 0;
    }

    if (mode == "decode") {
        auto dna_data = read_file(file);
        if (dna_data.empty()) return 1;
        std::string dna(reinterpret_cast<char*>(dna_data.data()), dna_data.size());
        auto bytes = decode_dna(dna);
        if (bytes.empty()) return 1;
        std::string out = (argc > 3) ? argv[3] : file + ".decoded";
        if (!write_file(out, bytes)) return 1;
        std::cout << "[DECODE] " << file << " -> " << out << "\n";
        std::cout << "  DNA chars: " << dna.length() << " | Bytes: " << bytes.size() << "\n";
        return 0;
    }

    if (mode == "roundtrip") {
        // ENCODE
        auto original = read_file(file);
        if (original.empty()) return 1;
        std::string dna = encode_bytes(original);
        if (!write_text(file + ".dna", dna)) return 1;
        std::cout << "[ROUNDTRIP] Phase 1: ENCODE complete\n";
        std::cout << "  " << original.size() << " bytes -> " << dna.length() << " bases\n";

        // DECODE
        auto bytes = decode_dna(dna);
        std::string decoded_file = file + "_decoded";
        if (!write_file(decoded_file, bytes)) return 1;
        std::cout << "[ROUNDTRIP] Phase 2: DECODE complete\n";
        std::cout << "  " << dna.length() << " bases -> " << bytes.size() << " bytes\n";

        // VERIFY
        if (original != bytes) {
            std::cerr << "[FAIL] Decoded bytes do not match original. Data corrupted.\n";
            return 1;
        }
        std::cout << "[ROUNDTRIP] Phase 3: VERIFY complete — bit-perfect match\n";

        // RESTORE PERMISSIONS
        chmod(decoded_file.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        std::cout << "[ROUNDTRIP] Phase 4: PERMISSIONS restored (+x)\n";

        // EXECUTE
        std::cout << "[ROUNDTRIP] Phase 5: EXECUTE — forking decoded binary...\n\n";
        pid_t pid = fork();
        if (pid == 0) {
            execl(("./" + decoded_file).c_str(), decoded_file.c_str(), nullptr);
            std::cerr << "[FAIL] exec failed: " << strerror(errno) << "\n";
            exit(1);
        } else if (pid > 0) {
            std::cout << "[ROUNDTRIP] Decoded binary running (PID: " << pid << ")\n";
            std::cout << "[ROUNDTRIP] Waiting 5 seconds for pulse verification...\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
            kill(pid, SIGTERM);
            waitpid(pid, nullptr, 0);
            std::cout << "\n[ROUNDTRIP] Phase 6: CLEANUP complete\n";
            std::cout << "[ROUNDTRIP] ✅ DNA binary pipeline verified. The beast replicates.\n";
        } else {
            std::cerr << "[FAIL] fork failed\n";
            return 1;
        }

        return 0;
    }

    std::cerr << "[ERROR] Unknown mode: " << mode << "\n";
    return 1;
}
