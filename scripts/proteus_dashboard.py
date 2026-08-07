#!/usr/bin/env python3
"""
Proteus-Zayden Unified Dashboard
Shows all engines, running status, and Gotem ledger.
"""

import os
import json
import subprocess
import time
from datetime import datetime

REPO_DIR = os.path.dirname(os.path.abspath(__file__))

ENGINES = {
    "Ghost": ["heartbeat", "heartbeat_replicant", "heartbeat_secure"],
    "Mirror": ["zayden_ultimate", "zayden_gorf", "zayden_bridge"],
    "Gate": ["gatekeeper", "supervisor"],
    "Swarm": ["swarm", "swarm_push", "cpp_push"],
    "Core": ["proteus_master", "proteus_kernel", "proteus_engine_v7", "proteus_final", "proteus_fixed"],
    "Agent": ["agent", "mutated", "remote_control"],
}

def get_running():
    """Check which binaries are currently running."""
    try:
        out = subprocess.check_output(["ps", "-o", "pid,comm"], text=True)
        lines = out.strip().split("\n")[1:]
        running = {}
        for line in lines:
            parts = line.strip().split()
            if len(parts) >= 2:
                pid, comm = parts[0], parts[1]
                running[comm] = pid
        return running
    except Exception:
        return {}

def get_binaries():
    """Find all executable binaries in repo."""
    bins = {}
    for category, names in ENGINES.items():
        for name in names:
            path = os.path.join(REPO_DIR, name)
            if os.path.isfile(path) and os.access(path, os.X_OK):
                size = os.path.getsize(path)
                mtime = datetime.fromtimestamp(os.path.getmtime(path)).strftime("%m-%d %H:%M")
                bins[name] = {"category": category, "size": size, "mtime": mtime}
    return bins

def get_ledger():
    """Read Gotem ledger if it exists."""
    ledger_path = os.path.join(REPO_DIR, "gotem_ledger.json")
    if os.path.exists(ledger_path):
        try:
            with open(ledger_path) as f:
                return json.load(f)
        except Exception:
            pass
    return []

def get_spine():
    """Read HOLO spine if it exists."""
    spine_path = os.path.join(REPO_DIR, "holo_spine.json")
    if os.path.exists(spine_path):
        try:
            with open(spine_path) as f:
                return json.load(f)
        except Exception:
            pass
    return []

def format_size(n):
    if n > 1024*1024:
        return f"{n/(1024*1024):.1f}MB"
    if n > 1024:
        return f"{n/1024:.1f}KB"
    return f"{n}B"

def print_dashboard():
    running = get_running()
    binaries = get_binaries()
    ledger = get_ledger()
    spine = get_spine()

    print("=" * 60)
    print("  PROTEUS-ZAYDEN UNIFIED DASHBOARD")
    print(f"  {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print("=" * 60)

    for category, names in ENGINES.items():
        print(f"\n  [{category}]")
        for name in names:
            status = "RUNNING" if name in running else "STOPPED"
            pid = running.get(name, "-")
            if name in binaries:
                b = binaries[name]
                print(f"    {name:20s} {status:8s} pid:{pid:>6s}  {format_size(b['size']):>8s}  {b['mtime']}")
            else:
                print(f"    {name:20s} NOT BUILT")

    print(f"\n  [LEDGER]")
    print(f"    Gotem entries:    {len(ledger)}")
    print(f"    HOLO spine:       {len(spine)} entries")
    if spine:
        head = spine[-1]["spine"]["this_hash"][:24]
        print(f"    Head hash:        {head}...")
        print(f"    Chain valid:      {all(spine[i]['spine']['this_hash'] == spine[i+1]['spine']['prev_hash'] for i in range(len(spine)-1)) if len(spine) > 1 else 'N/A (single entry)'}")
    print(f"    Bridge file:      {'EXISTS' if os.path.exists('gotem_holo_bridge.py') else 'MISSING'}")

    print("\n" + "=" * 60)

    # Export HTML dashboard
    html_path = os.path.join(REPO_DIR, "dashboard.html")
    with open(html_path, "w") as f:
        f.write(f"""<!DOCTYPE html>
<html><head><meta charset="UTF-8"><title>Proteus Dashboard</title>
<style>
body {{ background:#030206; color:#e8e2d6; font-family:monospace; padding:40px; }}
h1 {{ color:#c9a84c; }}
table {{ border-collapse:collapse; width:100%; margin:20px 0; }}
th {{ border-bottom:2px solid #c9a84c; color:#c9a84c; padding:8px; text-align:left; }}
td {{ border-bottom:1px solid #333; padding:8px; }}
.running {{ color:#6ee7b7; }}
.stopped {{ color:#fca5a5; }}
.notbuilt {{ color:#5a566e; }}
</style></head><body>
<h1>PROTEUS-ZAYDEN DASHBOARD</h1>
<p>{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>
<table><tr><th>Engine</th><th>Category</th><th>Status</th><th>PID</th><th>Size</th><th>Built</th></tr>
""")
        for category, names in ENGINES.items():
            for name in names:
                status = "RUNNING" if name in running else "STOPPED" if name in binaries else "NOT BUILT"
                css = "running" if status == "RUNNING" else "stopped" if status == "STOPPED" else "notbuilt"
                pid = running.get(name, "-")
                size = format_size(binaries[name]["size"]) if name in binaries else "-"
                mtime = binaries[name]["mtime"] if name in binaries else "-"
                f.write(f'<tr><td>{name}</td><td>{category}</td><td class="{css}">{status}</td><td>{pid}</td><td>{size}</td><td>{mtime}</td></tr>\n')
        f.write(f"""</table>
<h2>Ledger</h2>
<p>Gotem entries: {len(ledger)} | HOLO spine: {len(spine)}</p>
<p>Head hash: {spine[-1]['spine']['this_hash'][:32] if spine else 'N/A'}...</p>
</body></html>""")
    print(f"  [HTML] dashboard.html generated")
    print(f"  Open it: termux-open dashboard.html")
    print("=" * 60)

if __name__ == "__main__":
    print_dashboard()
