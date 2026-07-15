#include <iomanip>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

class MutatedProteus {
private:
    double theta;
    double psi;
    int cycle;
    std::vector<double> mutation_history;
    std::mt19937 rng;
    
public:
    MutatedProteus() : theta(9.0), psi(0.10), cycle(0), rng(std::random_device{}()) {
        std::cout << "[MUTATION] Starting at θ=" << theta << " | Ψ=10%" << std::endl;
    }
    
    void testTheta(int new_theta) {
        double old_theta = theta;
        theta = new_theta;
        std::cout << "[MUTATION] θ: " << old_theta << " → " << theta << std::endl;
    }
    
    void evolve() {
        cycle++;
        if (cycle % 12 == 0) {
            double target = (theta == 9.0) ? 5.0 : 9.0;
            testTheta(target);
        }
        
        psi = 0.10 + 0.90 * (1.0 - exp(-cycle * 0.01 / theta));
        psi = std::min(psi, 0.863853);
        
        mutation_history.push_back(psi);
        
        std::cout << "[CYCLE " << cycle << "] θ=" << theta 
                  << " | Ψ=" << std::fixed << std::setprecision(4) << (psi * 100) << "%" << std::endl;
    }
};

int main() {
    MutatedProteus mutant;
    for (int i = 0; i < 50; i++) {
        mutant.evolve();
    }
    return 0;
}
