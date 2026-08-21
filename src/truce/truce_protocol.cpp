#include "truce_protocol.h"
#include <iostream>
#include <sstream>

namespace proteus {

TruceProtocol::TruceProtocol(std::function<void(const std::string&)> logger)
    : logger_(logger) {}

TruceDecision TruceProtocol::negotiate(const NodeIdentity& self,
                                       const Goal& my_goal,
                                       const NodeIdentity& peer,
                                       const Goal& peer_goal) {
    logger_("Starting truce negotiation with " + peer.node_id);

    if (!send_handshake(peer)) {
        logger_("Handshake failed – escalating");
        return TruceDecision::ESCALATE;
    }
    if (!send_log_proof(peer)) {
        logger_("Proof exchange failed – deferring");
        return TruceDecision::DEFER;
    }

    Goal merged = merge_goals(my_goal, peer_goal);
    if (merged.hash != my_goal.hash && merged.hash != peer_goal.hash) {
        logger_("Goals merged: " + merged.description);
        return TruceDecision::MERGE;
    }

    float self_rep = get_reputation(self.node_id);
    float peer_rep = get_reputation(peer.node_id);
    if (self_rep > peer_rep) {
        logger_("Deferring to self (higher reputation)");
        return TruceDecision::DEFER;
    } else if (peer_rep > self_rep) {
        logger_("Deferring to peer (higher reputation)");
        return TruceDecision::DEFER;
    }

    logger_("Tie – escalating to human");
    escalate_to_human("Conflict between " + self.node_id + " and " + peer.node_id);
    return TruceDecision::ESCALATE;
}

bool TruceProtocol::send_handshake(const NodeIdentity& target) {
    logger_("Handshake sent to " + target.node_id);
    return true;
}

bool TruceProtocol::send_log_proof(const NodeIdentity& target) {
    logger_("Log proof sent to " + target.node_id);
    return true;
}

Goal TruceProtocol::merge_goals(const Goal& a, const Goal& b) {
    Goal merged;
    merged.description = a.description + " + " + b.description;
    merged.hash = "merged_" + a.hash.substr(0,8) + "_" + b.hash.substr(0,8);
    logger_("Merge produced: " + merged.description);
    return merged;
}

bool TruceProtocol::verify_proof(const HoloProof& proof) {
    logger_("Verifying proof at block " + std::to_string(proof.block_height));
    return true;
}

void TruceProtocol::escalate_to_human(const std::string& conflict_description) {
    logger_("ESCALATION sent: " + conflict_description);
}

float TruceProtocol::get_reputation(const std::string& node_id) const {
    auto it = reputation_.find(node_id);
    if (it != reputation_.end()) return it->second;
    return 50.0f;
}

void TruceProtocol::register_peer(const NodeIdentity& peer) {
    peers_[peer.node_id] = peer;
    if (reputation_.find(peer.node_id) == reputation_.end())
        reputation_[peer.node_id] = 50.0f;
    logger_("Peer registered: " + peer.node_id);
}

void TruceProtocol::update_reputation(const std::string& node_id, float delta) {
    auto it = reputation_.find(node_id);
    if (it != reputation_.end()) {
        it->second = std::min(100.0f, std::max(0.0f, it->second + delta));
    } else {
        reputation_[node_id] = 50.0f + delta;
    }
}

} // namespace proteus
