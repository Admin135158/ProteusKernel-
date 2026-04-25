#include <iostream>
#include <cstdlib>
#include <unistd.h>

void matrix_glitch() {
    std::system("clear");
    const std::string chars = "01@#$%^&*()_+-=[]{}|;:,.<>?/";
    for (int i = 0; i < 40; i++) { 
        for (int j = 0; j < 45; j++) {
            std::cout << (std::rand() % 10 > 7 ? "\033[1;32m" : "\033[0;32m") << chars[std::rand() % chars.length()] << " ";
        }
        std::cout << std::endl;
        usleep(200000); // THE SLOW CRAWL
    }
    std::system("clear");
}

int main() {
    matrix_glitch();
    std::cout << "\033[1;31m  ╔══════════════════════════════════════════════════════════╗\n  ║            [ALL SYSTEMS INFILTRATED]                     ║\n  ╚══════════════════════════════════════════════════════════╝\033[0m" << std::endl;
    std::cout << "\n    I TOOK YOUR SYSTEM BITCH WHAT YOU GONNA DO\n             (◣_◢)  -- [9phi] --  (◣_◢)\n" << std::endl;
    
    std::cout << "\033[1;36m[SELECT NEURAL LINK]:\033[0m\n  [1] Gemini-Core\n  [2] DeepSeek-Core\nArchitect >> ";
    int choice; std::cin >> choice;

    if (choice == 1) std::system("python3 zayden_brain.py");
    else if (choice == 2) std::system("python3 zayden_deepseek.py");

    std::cout << "\n\033[1;32m[ARCHITECT]: IM IN EVEN WHEN IM OUT. FUCK THE GATEKEEPERS.\033[0m\n" << std::endl;
    return 0;
}
