#include <iostream>
#include <string>
#include <chrono>
#include <thread>

int main() {
    std::cout << "[ZAYDEN NODE] Arbitration Cortex Online" << std::endl;

    int cycle = 0;
    while (true) {
        cycle++;
        std::cout << "[ZAYDEN] Cycle " << cycle
                  << " — Coherence OK" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    }
}
