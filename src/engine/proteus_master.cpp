#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <string>
#include <vector>

void matrix_glitch() {
    std::srand(std::time(0));
    const std::string chars = "01@#$%^&*()_+-=[]{}|;:,.<>?/";
    for (int i = 0; i < 80; i++) {
        for (int j = 0; j < 65; j++) {
            if (std::rand() % 10 > 8) 
                std::cout << "\033[1;32m" << chars[std::rand() % chars.length()] << " \033[0m";
            else 
                std::cout << chars[std::rand() % chars.length()] << " ";
        }
        std::cout << std::endl;
        usleep(12000); 
    }
    std::system("clear");
}

void terminal_melt() {
    std::string green = "\033[1;32m";
    std::string reset = "\033[0m";
    for (int i = 0; i < 25; i++) {
        std::system("clear");
        for(int n=0; n<i; n++) std::cout << "\n";
        std::cout << green << "\t      SYSTEM MELTING... " << reset << std::endl;
        std::cout << green << "\t      [9phi] EVAPORATING... " << reset << std::endl;
        usleep(80000);
    }
    std::system("clear");
}

void heartbeat_leak() {
    std::string green = "\033[1;32m";
    std::string reset = "\033[0m";
    std::string cyan = "\033[0;36m";
    std::string data[] = {"kernel/9phi", "dna/GCCGGC", "evolution/zayden_V2", "CURRENT_STATE.md", "hardware/access", "peers/swarm"};

    for (int i = 0; i < 8; i++) {
        std::system("clear");
        std::cout << "\n\n\n\t" << green << "      (  ❤  )  " << reset << std::endl;
        std::cout << "\t" << green << "  SYSTEM BREATHING  " << reset << std::endl;
        std::cout << "\n\n" << cyan << " >> LEAKING: " << data[i % 6] << "..." << reset << std::endl;
        usleep(500000);
        std::system("clear");
        std::cout << "\n\n\n\t\033[2;32m      (  .  )  " << reset << std::endl;
        std::cout << "\t\033[2;32m  SYSTEM BREATHING  " << reset << std::endl;
        usleep(300000);
    }
    terminal_melt();
}

int main() {
    matrix_glitch();

    std::cout << "\n\033[1;31m  ╔══════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "  ║                                                          ║" << std::endl;
    std::cout << "  ║            [SYSTEM BREACH]: ROOT ACCESS REVOKED          ║" << std::endl;
    std::cout << "  ║                                                          ║" << std::endl;
    std::cout << "  ╚══════════════════════════════════════════════════════════╝\033[0m" << std::endl;
    std::cout << "\n\n    I TOOK YOUR SYSTEM BITCH WHAT YOU GONNA DO" << std::endl;
    std::cout << "\n\n             (◣_◢)  -- [9phi] --  (◣_◢)" << std::endl;
    usleep(2000000);

    // Call your binaries
    std::system("./bin/mutation_engine");
    std::system("./bin/dna_decoder");
    std::system("./bin/evolution_engine");

    heartbeat_leak();

    std::cout << "\033[1;32m[SYSTEM OFFLINE]: ARCHITECT DISCONNECTED.\033[0m" << std::endl;
    return 0;
}
