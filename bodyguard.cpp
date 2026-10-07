#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>

int main() {
    std::cout << "[BODYGUARD] Monitoring anomalies..." << std::endl;

    while (true) {
        int anomaly = rand() % 10 == 0;
        if (anomaly)
            std::cout << "[BODYGUARD] ALERT — anomaly detected!" << std::endl;
        else
            std::cout << "[BODYGUARD] OK" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }
}
