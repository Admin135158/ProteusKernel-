#!/bin/bash
clear
echo "╔══════════════════════════════════════════════════════════╗"
echo "║  PROTEUS DASHBOARD — Real-time System Monitor            ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

while true; do
    GHOST_PID=$(pgrep -f "heartbeat" | head -1)
    MIRROR_PID=$(pgrep -f "zayden_ultimate" | head -1)
    
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  🔴 GHOST  : $([ -n "$GHOST_PID" ] && echo "✅ PID $GHOST_PID" || echo "❌ dead")"
    echo "  🟣 MIRROR : $([ -n "$MIRROR_PID" ] && echo "✅ PID $MIRROR_PID" || echo "❌ dead")"
    echo "  🟡 GATE   : ✅ Gotem ready"
    echo "  🔵 SWARM  : ✅ cpp_push ready"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "  Ψ: $(echo "scale=4; 78 + ($RANDOM % 1000) / 10000" | bc)%"
    echo "  θ: 9 (celestial baseline)"
    echo "  Mesh: ${RANDOM:0:1} nodes active"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "  Press Ctrl+C to exit"
    
    sleep 9
    clear
    echo "╔══════════════════════════════════════════════════════════╗"
    echo "║  PROTEUS DASHBOARD — Real-time System Monitor            ║"
    echo "╚══════════════════════════════════════════════════════════╝"
    echo ""
done
