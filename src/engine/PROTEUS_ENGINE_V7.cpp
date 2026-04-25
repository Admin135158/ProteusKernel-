#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <thread>
#include <chrono>

const double PHI = 1.61803398875;
const int NODES = 7;

struct EngineState {
    double psi = 78.0;
    double alpha = 0.3;
    int cycle = 0;
};

void update_gorf(EngineState &s) {
    double resonance = sin(2 * M_PI * s.cycle / 9.0) * PHI;
    s.psi += (s.alpha * resonance * (100.0 - s.psi)) * 0.05;
    s.cycle++;
    if (s.psi > 99.9) s.psi = 99.9;
}

int main() {
    EngineState s;
    std::cout << "\033[1;32m(◣_◢) PROTEUS_ENGINE_V7: INITIALIZED\033[0m" << std::endl;
    while(true) {
        update_gorf(s);
        std::cout << "\r[Ψ] POTENTIAL: " << s.psi << "% | CYCLE: " << s.cycle << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
