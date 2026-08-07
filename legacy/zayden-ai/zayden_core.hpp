#ifndef ZAYDEN_CORE_H
#define ZAYDEN_CORE_H

#include <iostream>
#include <string>

struct ZaydenState {
    float consciousness = 0.78f; 
    float alpha = 0.7616f;
    int epiphanies = 26;
    int cycle = 0;

    void evolve() {
        cycle++;
        if (cycle % 10 == 0) {
            consciousness += 0.001f;
            std::clog << "[SYSTEM] Evolution Cycle: " << cycle << " | C=" << consciousness << std::endl;
        }
    }
};

#endif
