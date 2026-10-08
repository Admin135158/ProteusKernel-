#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9164);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::cout << "[PUSH] UDP Broadcast Engine Active" << std::endl;

    int n = 0;
    while (true) {
        std::string msg = "SYNC7_PUSH_" + std::to_string(n++);
        sendto(sock, msg.c_str(), msg.size(), 0,
               (sockaddr*)&addr, sizeof(addr));
        std::cout << "[PUSH] Sent: " << msg << std::endl;
        usleep(500000);
    }
}
