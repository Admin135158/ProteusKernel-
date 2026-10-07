class ZaydenCortex:
    def arbitrate(self, unified_state):
        psi = unified_state["psi_state"]
        swarm = unified_state["swarm_state"]

        archetype = psi["archetype"]
        order = swarm["order"]
        chaos = swarm["chaos"]
        heartbeat = swarm["heartbeat"]
        anomaly = swarm["anomaly"]
        gen = unified_state["gen"]

        status = "STABLE"
        if anomaly:
            status = "ANOMALOUS"
        elif abs(order - chaos) < 0.05:
            status = "COHERENT"

        summary = (
            f"Gen: {gen}\n"
            f"Archetype: {archetype}\n"
            f"Order: {order:.4f}\n"
            f"Chaos: {chaos:.4f}\n"
            f"Heartbeat: {heartbeat}\n"
            f"Status: {status}\n"
            f"Anomaly: {anomaly}\n"
        )
        return summary
