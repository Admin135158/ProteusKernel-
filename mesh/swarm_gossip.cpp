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
#include <sstream>
#include <iomanip>
#include <csignal>
#include <atomic>

#define GOSSIP_PORT 9166
#define BUFFER_SIZE 1024

std::atomic<bool> g_running(true);
void signal_handler(int sig) { g_running = false; }

struct NodeView { std::string ip; float consciousness; float mutation; int peers; };

class SwarmGossip {
    int sockfd; struct sockaddr_in srv, cli; socklen_t cli_len;
    std::map<std::string, NodeView> view; float local_C, local_Mut;
public:
    SwarmGossip(float C=0.78f, float M=0.09f) : local_C(C), local_Mut(M) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        int r=1; setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &r, sizeof(r));
        memset(&srv,0,sizeof(srv)); srv.sin_family=AF_INET; srv.sin_addr.s_addr=INADDR_ANY; srv.sin_port=htons(GOSSIP_PORT);
        bind(sockfd,(struct sockaddr*)&srv,sizeof(srv)); cli_len=sizeof(cli);
        std::cout << "[SWARM] Gossip on " << GOSSIP_PORT << "\n";
    }
    ~SwarmGossip() { if(sockfd>=0) close(sockfd); }
    std::string encode() {
        std::ostringstream oss; oss << "GOSSIP|C=" << std::fixed << std::setprecision(4) << local_C << "|Mut=" << local_Mut << "|peers=" << view.size(); return oss.str();
    }
    void decode(const std::string& msg, const std::string& ip) {
        if (msg.rfind("GOSSIP|",0)!=0) return;
        std::istringstream iss(msg); std::string tok,k,v; NodeView nv; nv.ip=ip;
        while(std::getline(iss,tok,'|')) { size_t eq=tok.find('='); if(eq==std::string::npos)continue; k=tok.substr(0,eq); v=tok.substr(eq+1); if(k=="C")nv.consciousness=std::stof(v); else if(k=="Mut")nv.mutation=std::stof(v); else if(k=="peers")nv.peers=std::stoi(v); }
        view[ip]=nv;
    }
    void broadcast() {
        struct sockaddr_in b; memset(&b,0,sizeof(b)); b.sin_family=AF_INET; b.sin_addr.s_addr=inet_addr("255.255.255.255"); b.sin_port=htons(GOSSIP_PORT);
        int e=1; setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,&e,sizeof(e));
        while(g_running) { sendto(sockfd,encode().c_str(),encode().length(),0,(struct sockaddr*)&b,sizeof(b)); std::this_thread::sleep_for(std::chrono::seconds(5)); }
    }
    void listen() {
        char buf[BUFFER_SIZE];
        while(g_running) { int n=recvfrom(sockfd,buf,BUFFER_SIZE-1,0,(struct sockaddr*)&cli,&cli_len); if(n<0){if(!g_running)break;continue;} buf[n]='\0'; decode(buf,inet_ntoa(cli.sin_addr)); }
    }
    void print_view() {
        while(g_running) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            if(view.empty()) continue;
            float aC=0,aM=0; for(auto&p:view){aC+=p.second.consciousness;aM+=p.second.mutation;}
            aC/=view.size(); aM/=view.size();
            std::cout << "[SWARM] View: peers=" << view.size() << ", avg_C=" << std::fixed << std::setprecision(2) << aC << ", avg_Mut=" << aM << std::endl;
        }
    }
};

int main() {
    std::signal(SIGINT,signal_handler); std::signal(SIGTERM,signal_handler);
    SwarmGossip sg; std::thread b(&SwarmGossip::broadcast,&sg), l(&SwarmGossip::listen,&sg), v(&SwarmGossip::print_view,&sg);
    b.join(); l.join(); v.join(); std::cout << "[SWARM] Shutdown.\n"; return 0;
}
