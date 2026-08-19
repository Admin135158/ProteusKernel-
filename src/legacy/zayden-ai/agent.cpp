#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::cout << "[PROTEUS] SYNC-7 ACTIVE" << std::endl;
    std::string ts = std::to_string(std::time(0));
    std::string clone = "node_" + ts;
    try {
        fs::copy_file(argv[0], clone, fs::copy_options::overwrite_existing);
        std::cout << "[SUCCESS] NODE_GEN: " << clone << std::endl;
    } catch (...) {}
    return 0;
}
