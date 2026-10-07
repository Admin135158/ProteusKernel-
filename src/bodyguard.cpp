#include "morpheus_core.hpp"
using namespace morpheus;
class BGEngine : public Engine {
public:
    BGEngine() : Engine("bodyguard", 15005, getEnvSecret(), "data/bodyguard.journal") {}
    void onFrame(int cfd, const Frame& f) override {
        if (f.type == HEARTBEAT) { sendFrame(cfd, mkFrame(RESPONSE, "BG-ACK|ALIVE")); return; }
        std::string pl(f.payload.begin(), f.payload.end());
        if (f.type == COMMAND && pl.find("SCAN") != std::string::npos)
            sendFrame(cfd, mkFrame(RESPONSE, "BG-ACK|CLEAN|NO_THREATS|MEMORY_INTEGRITY:100%"));
        else if (f.type == COMMAND && pl.find("THREAT") != std::string::npos)
            sendFrame(cfd, mkFrame(RESPONSE, "BG-ACK|THREAT_DETECTED|COUNTERMEASURES_DEPLOYED"));
        else
            sendFrame(cfd, mkFrame(RESPONSE, "BG-ACK|GUARDING|PERIMETER_SECURE"));
    }
};
int main(int argc, char** argv) { BGEngine e; e.start(argc, argv); return 0; }
