#include <iostream>
#include <chrono>
#include <thread>

int main() {
    while (true) {
        std::cout << "[HEARTBEAT] SYNC-7 ACTIVE" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(700));
    }
    return 0;
}
