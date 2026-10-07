#!/bin/bash
cd ~/ProteusKernel
source venv/bin/activate 2>/dev/null || true

case "${1:-status}" in
    status)    python src/supervisor_v2.py status ;;
    scan)      python src/supervisor_v2.py scan ;;
    start)     python src/supervisor_v2.py start "$2" ;;
    tier)      python src/supervisor_v2.py tier "$2" ;;
    all)       python src/supervisor_v2.py all ;;
    stop)      python src/supervisor_v2.py stop "$2" ;;
    stopall)   python src/supervisor_v2.py stopall ;;
    monitor)   python src/supervisor_v2.py monitor ;;
    client)
        if [ -z "$2" ]; then
            echo "Usage: $0 client PORT [HEARTBEAT|COMMAND|STATUS] [payload]"
            echo "Example: $0 client 15001 HEARTBEAT"
            echo "Example: $0 client 15001 COMMAND 'AUTH'"
        else
            python src/morp_client.py 127.0.0.1 "$2" "${3:-HEARTBEAT}" "${4:-}"
        fi
        ;;
    dash|dashboard) python src/termux_dashboard.py ;;
    clean)     python src/supervisor_v2.py stopall; rm -f run/pids/*.pid; echo "[OK] Clean slate" ;;
    build)
        echo "[BUILD] Compiling MORP-v2 C++ engines..."
        for e in pk_swarm pk_heartbeat gatekeeper supervisor bodyguard swarm_gossip pk_zayden pk_gotem dna_binary; do
            echo "  -> $e"
            clang++ -std=c++17 -O2 -o bin/$e src/$e.cpp -pthread 2>&1 | grep -i error || true
        done
        echo "[BUILD] Done."
        ;;
    *) echo "Usage: $0 [status|scan|start NAME|tier TIER|all|stop NAME|stopall|monitor|client PORT CMD|dashboard|clean|build]" ;;
esac
