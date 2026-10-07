#include "morpheus_core.hpp"
using namespace morpheus;
class ZayEngine : public Engine {
public:
    ZayEngine() : Engine("pk_zayden", 15006, getEnvSecret(), "data/pk_zayden.journal") {}
    void onFrame(int cfd, const Frame& f) override {
        if (f.type == HEARTBEAT) { sendFrame(cfd, mkFrame(RESPONSE, "ZAYDEN-ACK|ALIVE")); return; }
        std::string pl(f.payload.begin(), f.payload.end());
        if (f.type == COMMAND && pl.find("BRIDGE") != std::string::npos)
            sendFrame(cfd, mkFrame(RESPONSE, "ZAYDEN-ACK|FEDERATED_BRIDGE_READY|MODELS:gemini,claude,deepseek,ollama"));
        else if (f.type == COMMAND && pl.find("ROUTE") != std::string::npos)
            sendFrame(cfd, mkFrame(RESPONSE, "ZAYDEN-ACK|ROUTE_ESTABLISHED|LATENCY:LOW"));
        else
            sendFrame(cfd, mkFrame(RESPONSE, "ZAYDEN-ACK|ONLINE"));
    }
};
int main(int argc, char** argv) { ZayEngine e; e.start(argc, argv); return 0; }
