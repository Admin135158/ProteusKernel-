#include "morpheus_core.hpp"
using namespace morpheus;
class SupEngine : public Engine {
public:
    SupEngine() : Engine("supervisor", 15004, getEnvSecret(), "data/supervisor.journal") {}
    void onFrame(int cfd, const Frame& f) override {
        if (f.type == HEARTBEAT) { sendFrame(cfd, mkFrame(RESPONSE, "SUPER-ACK|ALIVE")); return; }
        std::string pl(f.payload.begin(), f.payload.end());
        if (f.type == COMMAND && pl.find("ARBITRATE") != std::string::npos)
            sendFrame(cfd, mkFrame(RESPONSE, "SUPER-ACK|ARBITRATION_READY|NO_CONFLICTS|TRUCE_PROTOCOL:HONORED"));
        else
            sendFrame(cfd, mkFrame(RESPONSE, "SUPER-ACK|ONLINE"));
    }
};
int main(int argc, char** argv) { SupEngine e; e.start(argc, argv); return 0; }
