#include <iostream>
#include <string>
#include <chrono>
#include <thread>

int main() {
    std::cout << "[GOSSIP] Swarm gossip protocol active" << std::endl;

    int n = 0;
    while (true) {
        std::cout << "[GOSSIP] Node rumor: R" << n++ << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
    }
}
