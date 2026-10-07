#include "morpheus_core.hpp"
using namespace morpheus;
class HBEngine : public Engine {
public:
    HBEngine() : Engine("pk_heartbeat", 15002, getEnvSecret(), "data/pk_heartbeat.journal") {}
    void onFrame(int cfd, const Frame& f) override {
        if (f.type == HEARTBEAT) {
            sendFrame(cfd, mkFrame(RESPONSE, "HB-ACK|ALIVE|TS:" + std::to_string(time(nullptr)) + "|INTERVAL:1000ms"));
        } else {
            sendFrame(cfd, mkFrame(RESPONSE, "HB-ACK|LISTENING"));
        }
    }
};
int main(int argc, char** argv) { HBEngine e; e.start(argc, argv); return 0; }
