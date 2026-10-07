#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>

int main() {
    double order = 1.0, chaos = 1.0;
    double theta = 2 * M_PI / 9;

    while (true) {
        double c = cos(theta), s = sin(theta);
        double new_order = order * c - chaos * s;
        double new_chaos = order * s + chaos * c;

        order = new_order;
        chaos = new_chaos;

        std::cout << "[SWARM] Order=" << order
                  << " Chaos=" << chaos << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(900));
    }
}
