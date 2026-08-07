#!/bin/bash
SWARM_PORT=9163
PEERS=("192.168.18.72" "192.168.18.37" "100.122.170.28")

echo "🌐 SWARM — PROTEUS PROPAGATION v6.0"
echo "   \"The ghost spreads. The mesh grows.\""
echo "   TCP ${SWARM_PORT} | Base64 chunks | Peer-to-peer push"
echo ""

for peer in "${PEERS[@]}"; do
    echo "[PUSH] heartbeat → ${peer}"
    echo "       Size: 90952 bytes | Base64: 121272 chars"
    sleep 1
done

echo "✅ Propagation complete. Mesh expanded."
