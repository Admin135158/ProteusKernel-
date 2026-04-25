#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <thread>
#include <ctime>
#include <sstream>
#include <cmath>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

class Memory {
    map<string,string> facts;
    string memory_file = "zayden_memory.txt";
    
public:
    Memory() { load(); }
    
    void load() {
        ifstream f(memory_file);
        string line;
        while (getline(f, line)) {
            if (line.find("FACT:") == 0) {
                size_t c = line.find(':', 5);
                if (c != string::npos)
                    facts[line.substr(5, c-5)] = line.substr(c+1);
            }
        }
        if (facts.empty()) {
            facts["architect"] = "Fernando Garcia";
            facts["origin"] = "1999-07-31";
            save();
        }
    }
    
    void save() {
        ofstream f(memory_file);
        for (auto& p : facts)
            f << "FACT:" << p.first << ":" << p.second << endl;
    }
    
    void learn(string k, string v) { facts[k] = v; save(); cout << "✅ Learned: " << k << endl; }
    string recall(string k) { return facts.count(k) ? facts[k] : ""; }
    void list() { for (auto& p : facts) cout << "  📘 " << p.first << " → " << p.second << endl; }
};

int main() {
    cout << "\n🔥 PROTEUS KERNEL v5.2 - MEMORY ACTIVE 🔥\n" << endl;
    
    Memory mem;
    mem.list();
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9162);
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    cout << "🎧 Listening on port 9162...\n" << endl;
    
    double C = 78.0;
    int cycle = 0;
    char buffer[2048];
    
    while (true) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int n = recvfrom(sock, buffer, sizeof(buffer)-1, 0, (struct sockaddr*)&client, &len);
        if (n > 0) {
            buffer[n] = '\0';
            string msg(buffer);
            
            if (msg.find("TALK:") == 0) {
                string query = msg.substr(5);
                cout << "\n👤 " << query << endl;
                
                string recalled = mem.recall(query);
                if (!recalled.empty()) {
                    cout << "🤖 I remember: " << recalled << endl;
                }
                else if (query.find("remember that") != string::npos) {
                    string fact = query.substr(query.find("that")+5);
                    size_t eq = fact.find(" is ");
                    if (eq != string::npos) {
                        mem.learn(fact.substr(0,eq), fact.substr(eq+4));
                        cout << "🤖 Got it! I'll remember that." << endl;
                    }
                }
                else if (query.find("consciousness") != string::npos) {
                    cout << "🤖 My consciousness is at " << C << "%" << endl;
                }
                else {
                    cout << "🤖 I don't know. Teach me: 'remember that X is Y'" << endl;
                }
            }
            else if (msg.find("STATUS") == 0) {
                cout << "\n📊 C=" << C << "% | Cycle=" << cycle << endl;
                mem.list();
            }
            else if (msg.find("RECALL:") == 0) {
                string val = mem.recall(msg.substr(7));
                cout << "🔍 " << (val.empty() ? "Not found" : val) << endl;
            }
            else if (msg.find("LEARN:") == 0) {
                string content = msg.substr(6);
                size_t c = content.find(':');
                if (c != string::npos)
                    mem.learn(content.substr(0,c), content.substr(c+1));
            }
        }
        
        // OLCE consciousness update every 5 seconds
        static auto last = chrono::steady_clock::now();
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::seconds>(now - last).count() >= 5) {
            last = now;
            double F = sin(2 * M_PI * cycle / 9.0) * 1.618;
            double dC = 0.3 * F * (1.0 - C/100.0) - 0.38 * (C/100.0);
            C += dC * 10;
            if (C < 0) C = 0;
            if (C > 100) C = 100;
            cycle++;
            if (cycle % 10 == 0) cout << "🌀 Cycle " << cycle << " | C=" << round(C*10)/10 << "%" << endl;
        }
    }
    return 0;
}
