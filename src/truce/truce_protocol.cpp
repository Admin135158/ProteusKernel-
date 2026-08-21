#include "truce_protocol.h"
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>

namespace proteus {

TruceProtocol::TruceProtocol(std::function<void(const std::string&)> logger)
    : logger_(logger) {}

TruceDecision TruceProtocol::negotiate(const NodeIdentity& self,
                                       const Goal& my_goal,
                                       const NodeIdentity& peer,
                                       const Goal& peer_goal) {
    logger_("🕊️ Starting truce negotiation with " + peer.node_id);

    // 1. Handshake exchange
    if (!send_handshake(peer)) {
        logger_("⚠️ Handshake failed – escalating");
        return TruceDecision::ESCALATE;
    }

    // 2. Exchange HOLO proofs (to verify recent actions)
    if (!send_log_proof(peer)) {
        logger_("⚠️ Proof exchange failed – deferring");
        return TruceDecision::DEFER;
    }

    // 3. Try to merge goals
    Goal merged = merge_goals(my_goal, peer_goal);
    if (merged.hash != my_goal.hash && merged.hash != peer_goal.hash) {
        // A true merge is possible
        logger_("✅ Goals merged: " + merged.description);
        // Broadcast new merged goal to swarm (caller handles this)
        return TruceDecision::MERGE;
    }

    // 4. If no merge, check reputations
    float self_rep = get_reputation(self.node_id);
    float peer_rep = get_reputation(peer.node_id);
    if (self_rep > peer_rep) {
        logger_("🗳️ Deferring to self (higher reputation: " + std::to_string(self_rep) + ")");
        return TruceDecision::DEFER;
    } else if (peer_rep > self_rep) {
        logger_("🗳️ Deferring to peer (higher reputation: " + std::to_string(peer_rep) + ")");
        return TruceDecision::DEFER;
    }

    // 5. If all else fails → escalate to human
    logger_("🚨 Tie – escalating to human operator");
    escalate_to_human("Conflict between " + self.node_id + " and " + peer.node_id);
    return TruceDecision::ESCALATE;
}

bool TruceProtocol::send_handshake(const NodeIdentity& target) {
    // In production: send UDP packet with handshake payload
    // For now: simulate success
    logger_("📨 Handshake sent to " + target.node_id);
    return true;
}

bool TruceProtocol::send_log_proof(const NodeIdentity& target) {
    // In production: fetch HOLO proof for recent actions (last 100 entries)
    // Send to target, wait for ACK
    logger_("📜 Log proof sent to " + target.node_id);
    return true;
}

Goal TruceProtocol::merge_goals(const Goal& a, const Goal& b) {
    // Simple deterministic merge: concatenate, hash, take first N chars
    std::string combined = a.description + " + " + b.description;
    // In real code: use SHA‑256 to produce a unique merge
    Goal merged;
    merged.description = combined;
    merged.hash = "merged_" + a.hash.substr(0,8) + "_" + b.hash.substr(0,8);
    logger_("🔀 Merge produced: " + merged.description);
    return merged;
}

bool TruceProtocol::verify_proof(const HoloProof& proof) {
    // In production: verify Merkle path against the root
    logger_("🔍 Verifying proof at block " + std::to_string(proof.block_height));
    return true; // placeholder
}

void TruceProtocol::escalate_to_human(const std::string& conflict_description) {
    // Send UDP packet to port 9164 (configured in .env)
    // Example: "TRUCE_ESCALATION: <conflict_description>"
    logger_("📡 ESCALATION sent: " + conflict_description);
    // Actual socket code would go here (sendto)
    // In practice: use a pre-defined UDP socket to send raw bytes
}

float TruceProtocol::get_reputation(const std::string& node_id) const {
    auto it = reputation_.find(node_id);
    if (it != reputation_.end()) {
        return it->second;
    }
    return 50.0f; // default neutral
}

void TruceProtocol::register_peer(const NodeIdentity& peer) {
    peers_[peer.node_id] = peer;
    // Initialize reputation if not present
    if (reputation_.find(peer.node_id) == reputation_.end()) {
        reputation_[peer.node_id] = 50.0f;
    }
    logger_("🔄 Peer registered: " + peer.node_id);
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
