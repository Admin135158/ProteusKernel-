
// ========== TRUCE PROTOCOL INTEGRATION ==========
// Place this inside the heartbeat processing loop
// After you have `my_goal`, `peer_goal`, `psi`, `peer_psi`

if (my_goal != peer_goal && psi > 80 && peer_psi > 80) {
    proteus::TruceProtocol truce([](const std::string& msg) {
        std::cout << "[TRUCE] " << msg << std::endl;
    });

    proteus::NodeIdentity self;
    self.node_id = get_node_id(); // your function
    self.reputation = get_self_reputation(); // your function

    proteus::NodeIdentity peer;
    peer.node_id = peer_id; // from heartbeat
    peer.reputation = get_peer_reputation(peer_id);

    proteus::Goal my_goal_struct;
    my_goal_struct.hash = my_goal; // string hash
    my_goal_struct.description = get_goal_description(my_goal);

    proteus::Goal peer_goal_struct;
    peer_goal_struct.hash = peer_goal;
    peer_goal_struct.description = get_goal_description(peer_goal);

    proteus::TruceDecision decision = truce.negotiate(
        self,
        my_goal_struct,
        peer,
        peer_goal_struct
    );

    switch (decision) {
        case proteus::TruceDecision::MERGE:
            // Update your local goal to the merged one
            // Broadcast to swarm
            break;
        case proteus::TruceDecision::DEFER:
            // Follow the higher-rep agent's goal
            // Set flag to pause autonomous action
            break;
        case proteus::TruceDecision::ESCALATE:
            // Pause agent, wait for human intervention
            // Set a watchdog flag
            break;
        case proteus::TruceDecision::TIMEOUT:
            // Fallback – assume peer is dead, continue solo
            break;
    }
}
