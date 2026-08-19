cat > remote_control.cpp << 'EOF'
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define PORT 9162
#define BUFFER_SIZE 4096

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
    if (argc < 3) {
        cout << "Usage: remote_control <IP> <COMMAND>" << endl;
        return 1;
    }
    
    string ip = argv[1];
    string cmd;
    for (int i = 2; i < argc; i++) {
        if (i > 2) cmd += " ";
        cmd += argv[i];
    }
    
    string response = send_cmd(ip, cmd);
    cout << response << endl;
    
    return 0;
}
EOF
