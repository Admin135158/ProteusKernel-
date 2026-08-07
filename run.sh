#!/bin/bash

echo "=================================================="
echo "   PROTEUS KERNEL + ZAYDEN-AI DISTRIBUTED MESH   "
echo "=================================================="

# 1. Compile C++ Core Engine
echo "[1/3] Compiling PROTEUS_ENGINE_V7.cpp..."
g++ -O3 -std=c++17 PROTEUS_ENGINE_V7.cpp -o proteus_engine -lcurl 2>/dev/null || g++ -O3 -std=c++17 PROTEUS_ENGINE_V7.cpp -o proteus_engine

if [ ! -f "proteus_engine" ]; then
    echo "[!] Warning: Engine build skipped or missing custom main(). Using REST backend directly."
fi

# 2. Start Express Server
echo "[2/3] Booting REST API & Zayden AI Bridge on port 8080..."
node server.js > server.log 2>&1 &
SERVER_PID=$!

# 3. Serve Dashboard
echo "[3/3] Hosting HTML Dashboard on port 8000..."
python3 -m http.server 8000 > http.log 2>&1 &
HTTP_PID=$!

sleep 2
echo ""
echo ">>> SYSTEM READY <<<"
echo "  -> Dashboard UI : http://localhost:8000/dashboard.html"
echo "  -> Backend API  : http://localhost:8080/api/status"
echo ""
echo "Press CTRL+C to stop all background processes."

trap "kill $SERVER_PID $HTTP_PID 2>/dev/null; echo '[+] Shutdown complete.'; exit" INT TERM EXIT
wait
