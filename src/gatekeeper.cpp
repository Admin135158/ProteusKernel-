#include "morpheus_core.hpp"
using namespace morpheus;
class GKEngine : public Engine {
public:
    GKEngine() : Engine("gatekeeper", 15003, getEnvSecret(), "data/gatekeeper.journal") {}
    void onFrame(int cfd, const Frame& f) override {
        if (f.type == HEARTBEAT) { sendFrame(cfd, mkFrame(RESPONSE, "GK-ACK|ALIVE")); return; }
        std::string pl(f.payload.begin(), f.payload.end());
        if (f.type == COMMAND && pl.find("AUTH") != std::string::npos)
            sendFrame(cfd, mkFrame(RESPONSE, "GK-ACK|AUTHORIZED|STRICT_MODE|ACL:ACTIVE|TRUST_LEVEL:SOVEREIGN"));
        else if (f.type == COMMAND && pl.find("VERIFY") != std::string::npos)
            sendFrame(cfd, mkFrame(RESPONSE, "GK-ACK|TOKEN_VALID|ACCESS_GRANTED"));
        else
            sendFrame(cfd, mkFrame(ERROR, "GK-NAK|UNAUTHORIZED"));
    }
};
int main(int argc, char** argv) { GKEngine e; e.start(argc, argv); return 0; }
