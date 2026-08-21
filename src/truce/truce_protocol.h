#ifndef TRUCE_PROTOCOL_H
#define TRUCE_PROTOCOL_H

#include "truce_types.h"
#include <functional>
#include <unordered_map>
#include <string>

namespace proteus {

class TruceProtocol {
public:
    TruceProtocol(std::function<void(const std::string&)> logger);

    TruceDecision negotiate(const NodeIdentity& self,
                            const Goal& my_goal,
                            const NodeIdentity& peer,
                            const Goal& peer_goal);

    float get_reputation(const std::string& node_id) const;
    void register_peer(const NodeIdentity& peer);

private:
    bool send_handshake(const NodeIdentity& target);
    bool send_log_proof(const NodeIdentity& target);
    Goal merge_goals(const Goal& a, const Goal& b);
    bool verify_proof(const HoloProof& proof);
    void escalate_to_human(const std::string& conflict_description);
    void update_reputation(const std::string& node_id, float delta);

    std::unordered_map<std::string, NodeIdentity> peers_;
    std::unordered_map<std::string, float> reputation_;
    std::function<void(const std::string&)> logger_;
};

} // namespace proteus
#endif
