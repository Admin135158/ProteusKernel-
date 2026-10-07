#!/data/data/com.termux/files/usr/bin/bash
mkdir -p logs

# Core engines – adjust this list to what you actually have
ENGINES=(
    "pk_swarm"
    "pk_heartbeat"
    "gatekeeper"
    "supervisor"
    "bodyguard"
    "pk_zayden"
    "pk_gotem"
    "swarm_gossip"
    "zayden_unified"
)

echo "[*] Starting essential engines..."
for eng in "${ENGINES[@]}"; do
    if [[ -x "./$eng" ]]; then
        echo "    Starting: $eng"
        nohup "./$eng" >> "logs/${eng}.log" 2>&1 &
        sleep 1
    else
        echo "    [WARN] $eng not found or not executable"
    fi
done
echo "[*] Done. Use 'python monitor.py' to view logs."
