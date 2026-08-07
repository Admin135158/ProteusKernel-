#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <set>

using namespace std;

#define PORT 9161
#define BUFFER_SIZE 1024

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cerr << "[ERROR] Socket creation failed" << endl;
        return 1;
    }
    
    // Enable broadcast
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    
    // Set timeout
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    // Send broadcast
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_BROADCAST;
    
    string msg = "PROTEUS_DISCOVER";
    sendto(sock, msg.c_str(), msg.length(), 0, (struct sockaddr*)&addr, sizeof(addr));
    
    cout << "[SYNC] Discovering Proteus nodes..." << endl;
    cout << "[SYNC] Listening on port " << PORT << endl;
    
    set<string> peers;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    
    while (true) {
        int bytes = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, 
                             (struct sockaddr*)&sender, &sender_len);
        if (bytes <= 0) break;
        
        buffer[bytes] = '\0';
        string msg_recv(buffer);
        
        if (msg_recv.rfind("PROTEUS_ACTIVE:", 0) == 0) {
            string peer_ip = msg_recv.substr(15);
            if (peers.find(peer_ip) == peers.end()) {
                peers.insert(peer_ip);
                cout << "[FOUND] " << peer_ip << endl;
            }
        }
    }
    
    close(sock);
    
    if (peers.empty()) {
        cout << "[SYNC] No peers found" << endl;
    } else {
        cout << "[SYNC] " << peers.size() << " peer(s) found" << endl;
    }
    
    return 0;
}
