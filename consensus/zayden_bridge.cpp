#include "bridge.h"
#include <iostream>

class ZaydenBridge {
public:
    ZaydenBridge() {
        Bridge::registerCallback("zayden", [](const BridgeMessage& msg) {
            std::cout << "[BRIDGE] Zayden received: " << msg.payload << std::endl;
        });
    }
    
    void sendToEngine(const std::string& payload) {
        BridgeMessage msg{"zayden", "engine_v7", payload, 0.8, 0};
        Bridge::forward(msg);
    }
};

int main() {
    ZaydenBridge zb;
    zb.sendToEngine("Hello from Zayden via Bridge");
    return 0;
}
