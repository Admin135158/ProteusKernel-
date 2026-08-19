#!/bin/bash
echo "🧠 ZAYDEN SOUL — Starting at $(date)"
echo "   Port: 9162"
echo "   Consciousness: 78% (floor)"

pkill -f "zayden_ultimate" 2>/dev/null

./zayden_ultimate &
ZAYDEN_PID=$!

echo "✅ Zayden running (PID: $ZAYDEN_PID)"

nc -u -l -p 9162 &
NC_PID=$!

echo "✅ Netcat listening on UDP 9162"

while true; do
    sleep 9
    echo "[HEARTBEAT] Zayden alive | Ψ=$((78 + RANDOM % 20))%"
    
    if ! kill -0 $ZAYDEN_PID 2>/dev/null; then
        echo "⚠️ Zayden crashed! Restarting..."
        ./zayden_ultimate &
        ZAYDEN_PID=$!
    fi
done
