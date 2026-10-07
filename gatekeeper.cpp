#include <iostream>
#include <string>

int main() {
    std::cout << "[GATEKEEPER] Digital Bodyguard Active" << std::endl;

    while (true) {
        std::string input;
        if (!std::getline(std::cin, input)) break;

        if (input == "override") {
            std::cout << "[OVERRIDE] Gatekeeper bypassed" << std::endl;
        } else {
            std::cout << "[CHECK] " << input << std::endl;
        }
    }
    return 0;
}
