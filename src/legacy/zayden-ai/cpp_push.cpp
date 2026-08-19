#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 9162
#define BUFFER_SIZE 65536

using namespace std;

string send_cmd(const string& ip, const string& cmd) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "ERROR";
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(sock); return "ERROR"; }
    string full_cmd = "CMD:" + cmd;
    send(sock, full_cmd.c_str(), full_cmd.length(), 0);
    char buffer[BUFFER_SIZE] = {0};
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    close(sock);
    return bytes > 0 ? string(buffer, bytes) : "ERROR";
}

string base64_encode(const vector<unsigned char>& data) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    string result;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    for (size_t idx = 0; idx < data.size(); idx++) {
        char_array_3[i++] = data[idx];
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for (int j = 0; j < 4; j++) result += chars[char_array_4[j]];
            i = 0;
        }
    }
    if (i) {
        for (int j = i; j < 3; j++) char_array_3[j] = '\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        for (int j = 0; j < i + 1; j++) result += chars[char_array_4[j]];
        while (i++ < 3) result += '=';
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) { cout << "Usage: cpp_push <file> [target_ip]" << endl; return 1; }
    string ip = (argc >= 3) ? argv[2] : "192.168.18.72";
    string filename = argv[1];
    ifstream file(filename, ios::binary);
    if (!file.is_open()) { cerr << "[ERROR] Cannot open " << filename << endl; return 1; }
    vector<unsigned char> data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    string b64 = base64_encode(data);
    cout << "[PUSH] Sending " << b64.length() << " chars to " << ip << endl;
    send_cmd(ip, "rm -f /tmp/upload.b64");
    size_t chunk_size = 4000;
    for (size_t i = 0; i < b64.length(); i += chunk_size) {
        string chunk = b64.substr(i, chunk_size);
        send_cmd(ip, "echo '" + chunk + "' >> /tmp/upload.b64");
        if (i % 20000 == 0) cout << "  Progress: " << i << "/" << b64.length() << endl;
    }
    string dest = string(getenv("HOME")) + "/proteus_kernel/" + filename;
    send_cmd(ip, "base64 -d /tmp/upload.b64 > " + dest + " && chmod +x " + dest);
    cout << "[PUSH] Complete. File: " << dest << endl;
    string verify = send_cmd(ip, "ls -la " + dest);
    cout << "[VERIFY] " << verify << endl;
    return 0;
}
