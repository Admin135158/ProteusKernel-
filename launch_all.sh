#!/data/data/com.termux/files/usr/bin/bash
# Launch all engines in ~/ProteusKernel

# Cleanup leftover processes
echo "[*] Killing leftover processes..."
pkill -f "pk_|gatekeeper|supervisor|bodyguard|swarm|heartbeat|dna_binary|zayden|scs_main|chaos_engine|kernel_bridge|arbitration|initiation|cpp_push|proteus" 2>/dev/null || true
sleep 1

# Create logs directory
mkdir -p logs

# Find all executables and .py scripts
echo "[*] Discovering engines..."
for f in *; do
    if [[ -x "$f" || "$f" == *.py ]]; then
        # Skip source files, hidden, and this script itself
        [[ "$f" == *.cpp || "$f" == *.h || "$f" == *.pyc || "$f" == "launch_all.sh" || "$f" == "monitor.py" ]] && continue
        # Skip if it's a directory
        [[ -d "$f" ]] && continue
        echo "    Starting: $f"
        # Run in background with nohup
        if [[ "$f" == *.py ]]; then
            nohup python3 "$f" >> "logs/${f}.log" 2>&1 &
        else
            nohup "./$f" >> "logs/${f}.log" 2>&1 &
        fi
    fi
done

echo "[*] All engines launched. Logs are in ~/ProteusKernel/logs/"
echo "[*] Use './monitor.py' to view logs and interact."
