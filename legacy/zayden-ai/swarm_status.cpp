#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
using namespace std;

int main() {
    cout << "╔════════════════════════════════════════════════════════╗\n";
    cout << "║              PROTEUS SWARM STATUS                      ║\n";
    cout << "╚════════════════════════════════════════════════════════╝\n\n";
    
    cout << "[PHONE] " << endl;
    system("ps aux | grep proteus | grep -v grep | head -1");
    
    cout << "\n[TABLET] " << endl;
    system("./remote_control 192.168.18.72 \"ps aux | grep proteus | grep -v grep | head -1\"");
    
    cout << "\n[PEERS]" << endl;
    ifstream f("peers.txt");
    string line;
    while (getline(f, line)) cout << "  " << line << endl;
    f.close();
    
    return 0;
}
