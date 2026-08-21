#ifndef TRUCE_TYPES_H
#define TRUCE_TYPES_H

#include <string>
#include <vector>
#include <cstdint>

namespace proteus {

struct NodeIdentity {
    std::string node_id;
    std::string public_key;
    uint64_t uptime_seconds;
    float reputation;
};

struct Goal {
    std::string hash;
    std::string description;
};

struct HoloProof {
    std::string merkle_root;
    std::vector<std::string> path;
    uint64_t block_height;
};

enum class TruceDecision : uint8_t {
    MERGE,
    DEFER,
    ESCALATE,
    TIMEOUT
};

struct NegotiationMessage {
    uint64_t timestamp;
    NodeIdentity sender;
    Goal goal;
    HoloProof proof;
    TruceDecision proposed_action;
    std::string signature;
};

} // namespace proteus
#endif
