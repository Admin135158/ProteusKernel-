// simple_heartbeat.cpp
// Direct UDP heartbeat - finds nodes without peer discovery bullshit

#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <vector>
#include <map>
#include <fstream>
#include <ctime>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>

#define HEARTBEAT_PORT 9161
#define COMMAND_PORT 9162
#define HEARTBEAT_INTERVAL 3

using namespace std;

map<string, time_t> active_peers;
string my_ip;

vector<string> get_local_ips() {
    vector<string> ips;
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];
    
    if (getifaddrs(&ifaddr) == -1) return ips;
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        if (ifa->ifa_addr->sa_family == AF_INET) {
            int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s == 0) {
                string ip = host;
                if (ip != "127.0.0.1") ips.push_back(ip);
            }
        }
    }
    freeifaddrs(ifaddr);
    return ips;
}

void broadcast_heartbeat() {
    int sock;
    struct sockaddr_in broadcast_addr;
    int broadcast_enable = 1;
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;
    
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));
    
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(HEARTBEAT_PORT);
    broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    string message = "HEARTBEAT:" + my_ip + ":PROTEUS_ACTIVE";
    
    while (true) {
        sendto(sock, message.c_str(), message.length(), 0,
               (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
        this_thread::sleep_for(chrono::seconds(HEARTBEAT_INTERVAL));
    }
    close(sock);
}

void listen_heartbeat() {
    int sock;
    struct sockaddr_in listen_addr;
    char buffer[1024];
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;
    
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(HEARTBEAT_PORT);
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sock, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) < 0) {
        close(sock);
        return;
    }
    
    while (true) {
        int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                        (struct sockaddr*)&sender_addr, &addr_len);
        if (n > 0) {
            buffer[n] = '\0';
            string msg(buffer);
            
            if (msg.find("HEARTBEAT:") == 0) {
                size_t start = msg.find(":") + 1;
                size_t end = msg.find(":", start);
                string sender_ip = msg.substr(start, end - start);
                
                if (sender_ip != my_ip) {
                    active_peers[sender_ip] = time(NULL);
                    cout << "[HEARTBEAT] Peer: " << sender_ip << endl;
                    
                    ofstream peers("peers.txt", ios::app);
                    peers << sender_ip << endl;
                    peers.close();
                }
            }
        }
    }
    close(sock);
}

void cleanup_stale_peers() {
    while (true) {
        this_thread::sleep_for(chrono::seconds(10));
        time_t now = time(NULL);
        
        vector<string> to_remove;
        for (auto& peer : active_peers) {
            if (now - peer.second > 15) to_remove.push_back(peer.first);
        }
        
        for (auto& ip : to_remove) active_peers.erase(ip);
        
        ofstream active("active_peers.txt");
        for (auto& peer : active_peers) active << peer.first << endl;
        active.close();
    }
}

int main() {
    vector<string> ips = get_local_ips();
    if (ips.empty()) {
        cerr << "No network interfaces found" << endl;
        return 1;
    }
    my_ip = ips[0];
    
    cout << "========================================" << endl;
    cout << "SYNC-7 HEARTBEAT NODE" << endl;
    cout << "My IP: " << my_ip << endl;
    cout << "Heartbeat Port: " << HEARTBEAT_PORT << endl;
    cout << "Command Port: " << COMMAND_PORT << endl;
    cout << "========================================" << endl;
    
    thread broadcaster(broadcast_heartbeat);
    thread listener(listen_heartbeat);
    thread cleaner(cleanup_stale_peers);
    
    broadcaster.join();
    listener.join();
    cleaner.join();
    
    return 0;
}
