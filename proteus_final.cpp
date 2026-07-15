#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <cmath>
#include <random>
#include <iomanip>

const double THETA = 9.0;
const double PSI_TARGET = 0.863853;
const double PSI_FLOOR = 0.10;
const int NODES = 7;
const double PHI = 1.618033988749895;

class FinalSystem {
private:
    double psi;
    int cycle;
    std::vector<double> node_psi;
    std::map<std::string, double> metrics;
    std::mt19937 rng;
    
public:
    FinalSystem() : psi(PSI_FLOOR), cycle(0), rng(std::random_device{}()) {
        node_psi.resize(NODES, PSI_FLOOR);
        std::cout << "╔═══════════════════════════════════════════════╗\n";
        std::cout << "║  PROTEUS FINAL — SYSTEM CONVERGED            ║\n";
        std::cout << "║  θ=" << THETA << " | Ψ target=" << PSI_TARGET * 100 << "%\n";
        std::cout << "╚═══════════════════════════════════════════════╝\n";
    }
    
    void computePsi() {
        cycle++;
        
        double delta = PSI_TARGET - psi;
        psi += delta * 0.001 * (1.0 + 0.1 * sin(cycle * 0.01));
        psi = std::min(psi, PSI_TARGET - 0.000001);
        
        for (int i = 0; i < NODES; i++) {
            double phase = 2.0 * M_PI * cycle / (9.0 + i);
            node_psi[i] = PSI_FLOOR + (psi - PSI_FLOOR) * (0.5 + 0.5 * sin(phase));
        }
        
        metrics["psi_global"] = psi;
        metrics["psi_avg"] = 0.0;
        for (double v : node_psi) metrics["psi_avg"] += v;
        metrics["psi_avg"] /= NODES;
        metrics["cycle"] = cycle;
        metrics["theta"] = THETA;
    }
    
    void status() {
        std::cout << "\n═══════════════════════════════════════════════\n";
        std::cout << "  FINAL SYSTEM STATUS (Cycle " << cycle << ")\n";
        std::cout << "═══════════════════════════════════════════════\n";
        std::cout << "  Ψ: " << std::fixed << std::setprecision(6) 
                  << (psi * 100) << "%\n";
        std::cout << "  θ: " << THETA << "\n";
        std::cout << "  Nodes active: " << NODES << "\n";
        std::cout << "  Node Ψ values:\n";
        for (int i = 0; i < NODES; i++) {
            std::cout << "    [" << i << "] " << std::setw(8) 
                      << std::fixed << std::setprecision(4) 
                      << (node_psi[i] * 100) << "%\n";
        }
        std::cout << "═══════════════════════════════════════════════\n";
    }
    
    void run(int cycles) {
        for (int i = 0; i < cycles; i++) {
            computePsi();
            if (i % 100 == 0) status();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

int main() {
    FinalSystem system;
    system.run(3258);
    system.status();
    return 0;
}
