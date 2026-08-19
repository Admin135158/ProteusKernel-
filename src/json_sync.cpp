#include <iostream>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

int main() {
    std::string filename = "state.json";

    // 1. Read existing JSON file
    std::ifstream input_file(filename);
    if (!input_file.is_open()) {
        std::cerr << "[-] Error opening file: " << filename << std::endl;
        return 1;
    }

    json state_matrix;
    input_file >> state_matrix;
    input_file.close();

    // 2. Access and mutate internal state keys
    std::cout << "[*] Current Configuration Alpha: " << state_matrix["alpha"] << std::endl;
    
    int current_epiphanies = state_matrix["epiphanies"];
    state_matrix["epiphanies"] = current_epiphanies + 1;

    // 3. Serialize and save back (4‑space indentation)
    std::ofstream output_file(filename);
    output_file << state_matrix.dump(4);
    output_file.close();

    std::cout << "[+] State file successfully synchronized." << std::endl;
    return 0;
}

