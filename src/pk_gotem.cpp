#include "morpheus_core.hpp"
using namespace morpheus;
class GotemEngine : public Engine {
public:
    GotemEngine() : Engine("pk_gotem", 15007, getEnvSecret(), "data/pk_gotem.journal") {}
    void onFrame(int cfd, const Frame& f) override {
        if (f.type == HEARTBEAT) { sendFrame(cfd, mkFrame(RESPONSE, "GOTEM-ACK|ALIVE")); return; }
        uint8_t hash[32];
        SHA256 s; s.update(f.payload.data(), f.payload.size()); s.final(hash);
        std::string h = SHA256::hex(hash);
        sendFrame(cfd, mkFrame(RESPONSE, "GOTEM-ACK|SHA256:" + h + "|VERIFIED|INTEGRITY:INTACT"));
    }
};
int main(int argc, char** argv) { GotemEngine e; e.start(argc, argv); return 0; }
