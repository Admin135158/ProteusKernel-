#!/bin/bash

echo "🧬 Launching Zayden Ultimate..."
echo ""

# Kill any existing instances
pkill -f zayden_ultimate 2>/dev/null

# Run Zayden
./zayden_ultimate
