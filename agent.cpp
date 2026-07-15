#include <iostream>
#include <string>
#include <vector>

class Agent {
private:
    std::string name;
    std::vector<std::string> capabilities;
    double trust_level;
    
public:
    Agent(const std::string& n) : name(n), trust_level(0.5) {}
    
    void addCapability(const std::string& cap) {
        capabilities.push_back(cap);
    }
    
    void act() {
        std::cout << "[AGENT " << name << "] Acting with trust " 
                  << trust_level << std::endl;
    }
};

int main() {
    Agent zayden("Zayden");
    zayden.addCapability("listen");
    zayden.addCapability("respond");
    zayden.act();
    return 0;
}
