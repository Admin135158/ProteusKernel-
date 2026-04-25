#!/bin/bash
# PROTEUS SWARM LAUNCHER
# Starts the complete autonomous system

echo "🌀 Starting PROTEUS Swarm..."
echo ""

# Kill existing heartbeat instances (clean start)
echo "→ Resetting heartbeat guardians..."
pkill heartbeat 2>/dev/null
sleep 1

# Start the Proteus kernel
echo "→ Launching Proteus kernel..."
cd ~/proteus/evolution/v5.1
if [ -f ./proteus_v5_1 ]; then
    ./proteus_v5_1 --daemon --sync-port=7777 &
    KERNEL_PID=$!
    echo "  ✓ Kernel started (PID: $KERNEL_PID)"
elif [ -f ./agent ]; then
    ./agent --daemon --sync-port=7777 &
    KERNEL_PID=$!
    echo "  ✓ Kernel started (PID: $KERNEL_PID)"
else
    echo "  ✗ Kernel binary not found"
fi

sleep 2

# Start heartbeat guardians (2 instances for redundancy)
echo "→ Launching heartbeat guardians..."
cd ~
./heartbeat --watch-pid=$(pgrep -f "proteus_v5_1\|agent" | head -1) &
sleep 1
./heartbeat --watch-pid=$(pgrep -f "proteus_v5_1\|agent" | head -1) &
echo "  ✓ 2 heartbeat instances running"

# Create status file
mkdir -p ~/proteus/logs
echo "$(date -Iseconds): Swarm launched" >> ~/proteus/logs/swarm.log
echo "  ✓ Startup logged"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "SWARM ONLINE"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Talk to the swarm: ./zayden"
echo "Check status: swarm-status"
echo ""
