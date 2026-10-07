#include "morpheus_core.hpp"
#include <map>
using namespace morpheus;

struct NodeInfo { std::string name; int port; std::string tier; uint64_t lastSeen; };
std::map<std::string, NodeInfo> registry;
std::mutex regMtx;

std::string getKV(const std::string& s, const std::string& k) {
    size_t p = s.find(k + "=");
    if (p == std::string::npos) return "";
    p += k.size() + 1;
    size_t e = s.find(';', p);
    if (e == std::string::npos) e = s.size();
    return s.substr(p, e - p);
}

class SwarmEngine : public Engine {
public:
    SwarmEngine() : Engine("pk_swarm", 15001, getEnvSecret(), "data/pk_swarm.journal") {}
    void onFrame(int cfd, const Frame& f) override {
        if (f.type == HEARTBEAT) {
            sendFrame(cfd, mkFrame(RESPONSE, "SYNC-7-ACK|v2.0|ACCEPTED|PROTEUS_KERNEL"));
            return;
        }
        std::string pl(f.payload.begin(), f.payload.end());
        if (f.type == REGISTER) {
            NodeInfo n;
            n.name = getKV(pl, "name");
            n.port = atoi(getKV(pl, "port").c_str());
            n.tier = getKV(pl, "tier");
            n.lastSeen = (uint64_t)time(nullptr);
            { std::lock_guard<std::mutex> lk(regMtx); registry[n.name] = n; }
            journal_.append(std::vector<uint8_t>(pl.begin(), pl.end()));
            sendFrame(cfd, mkFrame(RESPONSE, "REGISTERED|" + n.name));
        } else if (f.type == STATUS) {
            std::string r = "REGISTRY|" + std::to_string(registry.size()) + "|NODES:";
            { std::lock_guard<std::mutex> lk(regMtx); for (auto& kv : registry) r += kv.first + ","; }
            sendFrame(cfd, mkFrame(RESPONSE, r));
        } else if (f.type == ROUTE) {
            std::string dst = getKV(pl, "dst");
            std::lock_guard<std::mutex> lk(regMtx);
            auto it = registry.find(dst);
            if (it != registry.end())
                sendFrame(cfd, mkFrame(RESPONSE, "ROUTE|" + dst + "|127.0.0.1|" + std::to_string(it->second.port)));
            else
                sendFrame(cfd, mkFrame(ERROR, "ROUTE|UNKNOWN_DESTINATION"));
        } else {
            sendFrame(cfd, mkFrame(ERROR, "UNKNOWN_TYPE"));
        }
    }
};

int main(int argc, char** argv) { SwarmEngine e; e.start(argc, argv); return 0; }
