// bridge.h
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <cstdlib>

using namespace std;

// This is your Memory class updated for clean linking
class Memory {
    map<string, string> facts;
    string memory_file = "zayden_memory.txt";
public:
    Memory() { load(); }
    void load() {
        ifstream f(memory_file);
        string line;
        while (getline(f, line)) {
            if (line.find("FACT:") == 0) {
                size_t c = line.find(':', 5);
                if (c != string::npos) facts[line.substr(5, c-5)] = line.substr(c+1);
            }
        }
    }
    string recall(string k) { return facts.count(k) ? facts[k] : "Unknown"; }
};

