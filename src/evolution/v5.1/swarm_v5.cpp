#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

using namespace std;

int main(int argc, char* argv[]) {
    string ip = "192.168.18.72";
    if (argc < 2) {
        cout << "Usage: swarm <command>" << endl;
        cout << "Commands: status, sync, run, exec, kill" << endl;
        return 1;
    }
    string cmd = argv[1];
    if (cmd == "status") {
        cout << "[PHONE]" << endl;
        system("ps aux | grep proteus | grep -v grep | head -1");
        cout << endl << "[TABLET]" << endl;
        string remote_cmd = "./remote_control " + ip + " \"ps aux | grep proteus | grep -v grep | head -1\"";
        system(remote_cmd.c_str());
        cout << endl << "[PEERS]" << endl;
        ifstream f("peers.txt");
        string line;
        while (getline(f, line)) cout << "  " << line << endl;
    } else if (cmd == "sync") {
        system("./cpp_push proteus_v5_1");
    } else if (cmd == "run") {
        string remote_cmd = "./remote_control " + ip + " \"cd ~/proteus_kernel && nohup ./proteus_v5_1 > /dev/null 2>&1 &\"";
        system(remote_cmd.c_str());
        cout << "[RUN] Tablet kernel started" << endl;
    } else if (cmd == "kill") {
        string remote_cmd = "./remote_control " + ip + " \"pkill proteus\"";
        system(remote_cmd.c_str());
        cout << "[KILL] Tablet kernel stopped" << endl;
    } else if (cmd == "exec") {
        string exec_cmd;
        for (int i = 2; i < argc; i++) {
            if (i > 2) exec_cmd += " ";
            exec_cmd += argv[i];
        }
        string remote_cmd = "./remote_control " + ip + " \"" + exec_cmd + "\"";
        system(remote_cmd.c_str());
    }
    return 0;
}
