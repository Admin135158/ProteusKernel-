#ifndef BRIDGE_H
#define BRIDGE_H

#include <string>
#include <functional>

struct BridgeMessage {
    std::string source;
    std::string target;
    std::string payload;
    double priority;
    uint64_t timestamp;
};

class Bridge {
public:
    static bool forward(const BridgeMessage& msg);
    static void registerCallback(const std::string& target, 
                                 std::function<void(const BridgeMessage&)> callback);
    static void processQueue();
};

#endif
