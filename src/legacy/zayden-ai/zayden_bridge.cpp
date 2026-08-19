// zayden_bridge.cpp
#include "bridge.h"

int main() {
    Memory mem;
    string architect = mem.recall("architect");
    string prompt = "Hello, I am " + architect + "'s AI. What is our current mission?";
    
    string cmd = "curl -s -X POST http://localhost:11434/api/generate -d '{\"model\":\"tinyllama:1.1b\",\"prompt\":\"" + prompt + "\",\"stream\":false}'";
    
    cout << "Sending to Cortex: " << prompt << endl;
    system(cmd.c_str());
    return 0;
}

