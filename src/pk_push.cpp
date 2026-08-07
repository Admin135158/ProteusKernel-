/*
 * SPDX-License-Identifier: Proprietary
 * Copyright (c) 2026 Fernando De Jesus Garcia Gonzalez (The Architect)
 */
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc == 2 && (strcmp(argv[1], "--license") == 0 || strcmp(argv[1], "-L") == 0)) {
        std::cout << "=== FTCoE Sovereign IP & Universal Licensing Mandate ===\n";
        std::cout << "Commercial use requires 30% royalty. See LICENSE file.\n";
        return 0;
    }
    if (argc != 3) {
        std::cerr << "Usage: pk_push <file> <target_ip>\n";
        return 1;
    }
    std::string cmd = "./pk_swarm push " + std::string(argv[1]) + " " + std::string(argv[2]);
    std::cout << "[PUSH] " << argv[1] << " → " << argv[2] << std::endl;
    return system(cmd.c_str());
}
