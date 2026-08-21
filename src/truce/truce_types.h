#ifndef TRUCE_TYPES_H
#define TRUCE_TYPES_H

#include <string>
#include <vector>
#include <cstdint>

namespace proteus {

struct NodeIdentity {
    std::string node_id;
    std::string public_key;   // base64 encoded
    uint64_t uptime_seconds;
    float reputation;         // 0.0 – 100.0
};

struct Goal {
    std::string hash;         // SHA‑256 of the goal description
    std::string description;  // human‑readable (for logging)
};

struct HoloProof {
    std::string merkle_root;
    std::vector<std::string> path;   // sibling hashes for verification
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
    std::string signature;   // signed by sender's private key
};

} // namespace proteus

#endif
