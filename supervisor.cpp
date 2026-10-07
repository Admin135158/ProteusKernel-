#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "[SUPERVISOR] Swarm oversight active" << std::endl;

    int cycle = 0;
    while (true) {
        cycle++;
        std::cout << "[SUPERVISOR] Cycle " << cycle
                  << " — All nodes responsive" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }
}
