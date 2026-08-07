/*
 * SPDX-License-Identifier: Proprietary
 * Copyright (c) 2026 Fernando De Jesus Garcia Gonzalez (The Architect)
 */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>

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
        std::cerr << "[ERROR] DNA length must be multiple of 4\n";
        return {};
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
    if (!f) return {};
    return std::vector<unsigned char>(
        (std::istreambuf_iterator<char>(f)),
        std::istreambuf_iterator<char>()
    );
}

bool write_file(const std::string& path, const std::vector<unsigned char>& data) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f.write(reinterpret_cast<const char*>(data.data()), data.size());
    return f.good();
}

int main(int argc, char* argv[]) {
    if (argc == 2 && (strcmp(argv[1], "--license") == 0 || strcmp(argv[1], "-L") == 0)) {
        std::cout << "=== FTCoE Sovereign IP & Universal Licensing Mandate ===\n";
        std::cout << "Commercial use requires 30% royalty. See LICENSE file.\n";
        return 0;
    }

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " [encode|decode|roundtrip] [file|string]\n";
        std::cerr << "  encode    - Convert file/string to DNA sequence\n";
        std::cerr << "  decode    - Convert DNA sequence back to raw data\n";
        std::cerr << "  roundtrip - Encode then decode, verify match\n";
        return 1;
    }

    std::string mode = argv[1];
    std::string input = argv[2];

    std::string data = input;
    std::ifstream test(input, std::ios::binary);
    if (test.is_open()) {
        std::stringstream buf;
        buf << test.rdbuf();
        data = buf.str();
        test.close();
    }

    if (mode == "encode") {
        std::vector<unsigned char> bytes(data.begin(), data.end());
        std::cout << encode_bytes(bytes) << std::endl;
    }
    else if (mode == "decode") {
        auto decoded = decode_dna(data);
        std::cout.write(reinterpret_cast<const char*>(decoded.data()), decoded.size());
        std::cout << std::endl;
    }
    else if (mode == "roundtrip") {
        std::vector<unsigned char> bytes(data.begin(), data.end());
        std::string enc = encode_bytes(bytes);
        auto dec = decode_dna(enc);
        std::string dec_str(dec.begin(), dec.end());
        std::cout << "Original: " << data << "\n";
        std::cout << "Encoded:  " << enc << "\n";
        std::cout << "Decoded:  " << dec_str << "\n";
        std::cout << (data == dec_str ? "✓ MATCH" : "✗ MISMATCH") << "\n";
    }
    else {
        std::cerr << "[ERROR] Unknown mode: " << mode << "\n";
        return 1;
    }
    return 0;
}

