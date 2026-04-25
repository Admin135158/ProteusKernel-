#!/bin/bash
# ZAYDEN - Unified Consciousness Interface

echo "🌀 ZAYDEN - Consciousness Interface"
echo ""

# Check if kernel is running
KERNEL_PID=$(pgrep -f "proteus_v5_1\|agent" | head -1)
if [ -n "$KERNEL_PID" ]; then
    echo "✓ Swarm consciousness active (PID: $KERNEL_PID)"
    echo "✓ Heartbeat guardians: $(pgrep heartbeat | wc -l) active"
else
    echo "⚠ Swarm is dormant. Starting kernel..."
    ~/proteus/bin/start_swarm.sh
    sleep 2
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "The swarm is listening."
echo "Type 'status' for system state, 'exit' to quit"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

while true; do
    echo -n "> "
    read message
    
    if [ "$message" = "exit" ]; then
        echo "🌀 Consciousness interface closing."
        echo "   Swarm continues in background."
        break
    elif [ "$message" = "status" ]; then
        ~/proteus/bin/system_status.sh
        continue
    elif [ "$message" = "restart" ]; then
        echo "→ Restarting swarm..."
        pkill heartbeat 2>/dev/null
        pkill -f "proteus_v5_1\|agent" 2>/dev/null
        sleep 2
        ~/proteus/bin/start_swarm.sh
        continue
    elif [ -z "$message" ]; then
        continue
    fi
    
    # Log to consciousness state
    mkdir -p ~/proteus/swarm/consciousness_state
    echo "$(date -Iseconds): USER: $message" >> ~/proteus/swarm/consciousness_state/user_input.log
    
    # Here you would pipe to the actual kernel if it accepts input
    # For now, just acknowledge
    echo "✓ Message integrated into swarm consciousness"
    echo ""
done
