#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <map>
#include <fstream>
#include <ifaddrs.h>
#include <netdb.h>

using namespace std;

string my_ip;
map<string, time_t> active_peers;
int heartbeat_port = 9161;

string get_my_ip() {
    struct ifaddrs *ifaddr;
    char host[256];
    
    if (getifaddrs(&ifaddr) == -1) return "unknown";
    
    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                       host, sizeof(host), NULL, 0, NI_NUMERICHOST);
            string ip = host;
            if (ip != "127.0.0.1") {
                freeifaddrs(ifaddr);
                return ip;
            }
        }
    }
    freeifaddrs(ifaddr);
    return "unknown";
}

void send_heartbeat() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    
    struct sockaddr_in bc_addr;
    bc_addr.sin_family = AF_INET;
    bc_addr.sin_port = htons(heartbeat_port);
    bc_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    string msg = "HEARTBEAT:" + my_ip + ":PROTEUS_ACTIVE";
    
    while (true) {
        sendto(sock, msg.c_str(), msg.length(), 0, 
               (struct sockaddr*)&bc_addr, sizeof(bc_addr));
        
        vector<string> cellular = {"100.88.83.53", "100.84.68.68", "100.93.246.209"};
        for (auto& ip : cellular) {
            struct sockaddr_in peer;
            peer.sin_family = AF_INET;
            peer.sin_port = htons(heartbeat_port);
            peer.sin_addr.s_addr = inet_addr(ip.c_str());
            sendto(sock, msg.c_str(), msg.length(), 0,
                   (struct sockaddr*)&peer, sizeof(peer));
        }
        
        this_thread::sleep_for(chrono::seconds(3));
    }
    close(sock);
}

void listen_heartbeat() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(heartbeat_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    char buffer[256];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    
    while (true) {
        memset(buffer, 0, 256);
        int n = recvfrom(sock, buffer, 255, 0, 
                        (struct sockaddr*)&sender, &sender_len);
        
        if (n > 0) {
            string msg(buffer);
            if (msg.find("HEARTBEAT:") == 0) {
                size_t start = 11;
                size_t end = msg.find(":", start);
                string peer_ip = msg.substr(start, end - start);
                
                if (peer_ip != my_ip) {
                    active_peers[peer_ip] = time(NULL);
                    cout << "[PEER] " << peer_ip << endl;
                    
                    ofstream f("peers.txt", ios::app);
                    f << peer_ip << endl;
                    f.close();
                }
            }
        }
    }
    close(sock);
}

void cleanup_peers() {
    while (true) {
        this_thread::sleep_for(chrono::seconds(15));
        time_t now = time(NULL);
        
        for (auto it = active_peers.begin(); it != active_peers.end();) {
            if (now - it->second > 30) {
                it = active_peers.erase(it);
            } else {
                ++it;
            }
        }
        
        ofstream f("active.txt");
        for (auto& p : active_peers) {
            f << p.first << endl;
        }
        f.close();
    }
}

int main() {
    my_ip = get_my_ip();
    cout << "========================================" << endl;
    cout << "SYNC-7 HEARTBEAT v2 (C++)" << endl;
    cout << "My IP: " << my_ip << endl;
    cout << "Port: " << heartbeat_port << endl;
    cout << "========================================" << endl;
    
    thread t1(send_heartbeat);
    thread t2(listen_heartbeat);
    thread t3(cleanup_peers);
    
    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}
