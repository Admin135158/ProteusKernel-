#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <algorithm>

using namespace std;

#define PORT 9163
#define BUFFER_SIZE 4096

string get_local_ip() {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];
    
    if (getifaddrs(&ifaddr) == -1) return "127.0.0.1";
    
    string ip = "127.0.0.1";
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;
        if (strcmp(ifa->ifa_name, "lo") == 0) continue;
        
        int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
        if (s == 0) {
            ip = host;
            break;
        }
    }
    freeifaddrs(ifaddr);
    return ip;
}

string discover_tablet() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return "";
    
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_BROADCAST;
    
    string msg = "PROTEUS_TABLET";
    sendto(sock, msg.c_str(), msg.length(), 0, (struct sockaddr*)&addr, sizeof(addr));
    
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    char buffer[1024];
    
    int bytes = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, 
                         (struct sockaddr*)&sender, &sender_len);
    close(sock);
    
    if (bytes > 0) {
        buffer[bytes] = '\0';
        if (string(buffer) == "PROTEUS_TABLET") {
            return inet_ntoa(sender.sin_addr);
        }
    }
    
    return "";
}

string send_cmd(const string& ip, const string& cmd) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "ERROR: Socket creation failed";
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return "ERROR: Connection failed";
    }
    
    string full_cmd = "CMD:" + cmd;
    send(sock, full_cmd.c_str(), full_cmd.length(), 0);
    
    char buffer[BUFFER_SIZE] = {0};
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    close(sock);
    
    if (bytes > 0) return string(buffer, bytes);
    return "ERROR: No response";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: sync7_connect <command>" << endl;
        cout << "Commands: discover, status, push, run, exec <cmd>" << endl;
        return 1;
    }
    
    string cmd = argv[1];
    
    if (cmd == "discover") {
        string ip = discover_tablet();
        if (!ip.empty()) {
            cout << "[FOUND] Tablet at " << ip << endl;
            ofstream ip_file("tablet_ip.txt");
            ip_file << ip;
            ip_file.close();
        } else {
            cout << "[NOT FOUND] No tablet heartbeat detected" << endl;
        }
        
    } else {
        string tablet_ip = "192.168.18.72";
        ifstream ip_file("tablet_ip.txt");
        if (ip_file.is_open()) {
            getline(ip_file, tablet_ip);
            ip_file.close();
        }
        
        if (cmd == "status") {
            cout << send_cmd(tablet_ip, "ps aux | grep proteus | grep -v grep || echo 'Not running'") << endl;
            
        } else if (cmd == "push") {
            ifstream file("proteus_v5_1", ios::binary);
            if (!file.is_open()) {
                cerr << "[ERROR] proteus_v5_1 not found" << endl;
                return 1;
            }
            
            vector<unsigned char> data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            file.close();
            
            // Simple base64 for the data
            string b64;
            const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            for (size_t i = 0; i < data.size(); i += 3) {
                uint32_t triplet = 0;
                int count = 0;
                for (int j = 0; j < 3 && i + j < data.size(); j++) {
                    triplet = (triplet << 8) | data[i + j];
                    count++;
                }
                for (int j = 0; j < 4; j++) {
                    if (j <= count) {
                        b64 += chars[(triplet >> (18 - j * 6)) & 0x3F];
                    } else {
                        b64 += '=';
                    }
                }
            }
            
            cout << "[PUSH] Sending " << b64.length() << " chars..." << endl;
            send_cmd(tablet_ip, "python3 receive.py START");
            
            size_t chunk = 1000;
            for (size_t i = 0; i < b64.length(); i += chunk) {
                string chunk_str = b64.substr(i, chunk);
                send_cmd(tablet_ip, "python3 receive.py CHUNK '" + chunk_str + "'");
            }
            
            send_cmd(tablet_ip, "python3 receive.py END ~/proteus_kernel/proteus_kernel");
            cout << "[PUSH] Complete" << endl;
            
        } else if (cmd == "run") {
            send_cmd(tablet_ip, "cd ~/proteus_kernel && chmod +x proteus_kernel && nohup ./proteus_kernel > /dev/null 2>&1 &");
            cout << "[RUN] Kernel started on tablet" << endl;
            
        } else if (cmd == "exec") {
            string exec_cmd;
            for (int i = 2; i < argc; i++) {
                if (i > 2) exec_cmd += " ";
                exec_cmd += argv[i];
            }
            cout << send_cmd(tablet_ip, exec_cmd) << endl;
            
        } else {
            cout << "Unknown command: " << cmd << endl;
        }
    }
    
    return 0;
}
