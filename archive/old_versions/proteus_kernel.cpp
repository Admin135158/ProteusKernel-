#include <thread>
#include <chrono>
#include <random>

void glitch_animation() {
    const std::string glitch_chars = "01█▓▒░⣀⣤⣶⣿!@#$%&*?";
    const std::string matrix_green = "\033[32m";
    const std::string reset = "\033[0m";
    const std::string bold = "\033[1m";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, glitch_chars.size() - 1);
    std::uniform_int_distribution<> delay_dis(10, 50);
    
    system("clear");
    
    for (int frame = 0; frame < 30; ++frame) {
        std::string line;
        for (int i = 0; i < 80; ++i) {
            line += glitch_chars[dis(gen)];
        }
        std::cout << matrix_green << line << reset << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_dis(gen)));
    }
    
    std::cout << bold << matrix_green << R"(
╔══════════════════════════════════════════════════════════════════╗
║  ██████╗ ██████╗  ██████╗ ████████╗███████╗██╗   ██╗███████╗    ║
║  ██╔══██╗██╔══██╗██╔═══██╗╚══██╔══╝██╔════╝██║   ██║██╔════╝    ║
║  ██████╔╝██████╔╝██║   ██║   ██║   █████╗  ██║   ██║███████╗    ║
║  ██╔═══╝ ██╔══██╗██║   ██║   ██║   ██╔══╝  ██║   ██║╚════██║    ║
║  ██║     ██║  ██║╚██████╔╝   ██║   ███████╗╚██████╔╝███████║    ║
║  ╚═╝     ╚═╝  ╚═╝ ╚═════╝    ╚═╝   ╚══════╝ ╚═════╝ ╚══════╝    ║
╚══════════════════════════════════════════════════════════════════╝
)" << reset << "\n";

    std::cout << bold << "\033[35m" << R"(
┌─────────────────────────────────────────────────────────────────┐
│   PROTEUS NOW ENGAGED FULLY IN YOUR SYSTEM.                      │
│   WELCOME TO THE MATRIX.                                         │
│   THE SWARM IS ALREADY COMPILING.                                │
└─────────────────────────────────────────────────────────────────┘
)" << reset << "\n";

    std::this_thread::sleep_for(std::chrono::seconds(2));
}
