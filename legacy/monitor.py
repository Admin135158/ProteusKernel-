#!/usr/bin/env python3
import os
import sys
import time
import subprocess
import glob
from datetime import datetime

GREEN = "\033[92m"
CYAN = "\033[96m"
YELLOW = "\033[93m"
RED = "\033[91m"
BOLD = "\033[1m"
RESET = "\033[0m"

def list_engines():
    logs = sorted(glob.glob("logs/*.log"))
    engines = [os.path.basename(f).replace('.log', '') for f in logs]
    return engines

def tail_log(engine, lines=20):
    logfile = f"logs/{engine}.log"
    if not os.path.exists(logfile):
        print(f"{RED}Log file for {engine} not found.{RESET}")
        return
    try:
        with open(logfile, 'r') as f:
            content = f.readlines()
        tail = content[-lines:] if len(content) > lines else content
        print(f"{BOLD}{CYAN}=== Last {len(tail)} lines of {engine} ==={RESET}")
        for line in tail:
            line = line.strip()
            if "ERROR" in line:
                color = RED
            elif "WARN" in line:
                color = YELLOW
            else:
                color = GREEN
            print(f"{color}{line}{RESET}")
    except Exception as e:
        print(f"{RED}Error: {e}{RESET}")

def stream_log(engine):
    logfile = f"logs/{engine}.log"
    if not os.path.exists(logfile):
        print(f"{RED}Log file not found.{RESET}")
        return
    try:
        print(f"{BOLD}{CYAN}Streaming {engine} (Ctrl+C to stop){RESET}")
        subprocess.run(["tail", "-f", logfile])
    except KeyboardInterrupt:
        print("\nStopped.")
    except Exception as e:
        print(f"{RED}Error: {e}{RESET}")

def start_engine(engine):
    """Start a single engine (must be in current directory)."""
    if not os.path.exists(f"./{engine}"):
        print(f"{RED}Engine '{engine}' not found in current dir.{RESET}")
        return
    # Check if already running
    result = subprocess.run(["pgrep", "-f", f"./{engine}"], capture_output=True)
    if result.returncode == 0:
        print(f"{YELLOW}{engine} is already running.{RESET}")
        return
    # Start it
    print(f"{GREEN}Starting {engine}...{RESET}")
    os.system(f"nohup ./{engine} >> logs/{engine}.log 2>&1 &")
    time.sleep(0.5)
    # Verify
    result = subprocess.run(["pgrep", "-f", f"./{engine}"], capture_output=True)
    if result.returncode == 0:
        print(f"{GREEN}{engine} started successfully.{RESET}")
    else:
        print(f"{RED}Failed to start {engine}.{RESET}")

def stop_engine(engine):
    """Stop a single engine by name."""
    print(f"{RED}Stopping {engine}...{RESET}")
    os.system(f"pkill -f './{engine}' 2>/dev/null || true")
    print(f"{GREEN}Done.{RESET}")

def stop_all():
    os.system("pkill -f 'pk_|gatekeeper|supervisor|bodyguard|swarm|heartbeat|dna_binary|zayden|proteus' 2>/dev/null || true")
    print(f"{GREEN}All engines stopped.{RESET}")

def query_ollama(prompt):
    model = os.environ.get("OLLAMA_MODEL", "tinyllama")
    print(f"{YELLOW}🤔 Querying {model}...{RESET}")
    try:
        result = subprocess.run(["ollama", "run", model, prompt],
                                capture_output=True, text=True, timeout=60)
        if result.returncode == 0:
            print(f"{GREEN}{BOLD}OLLAMA:{RESET} {result.stdout.strip()}")
        else:
            print(f"{RED}Error: {result.stderr.strip()}{RESET}")
    except subprocess.TimeoutExpired:
        print(f"{RED}Timeout (60s).{RESET}")
    except Exception as e:
        print(f"{RED}Exception: {e}{RESET}")

def main():
    while True:
        print(f"\n{BOLD}{CYAN}╔═══════════════════════════════════════╗{RESET}")
        print(f"{BOLD}{CYAN}║   MORPHEUS MONITOR                   ║{RESET}")
        print(f"{BOLD}{CYAN}╚═══════════════════════════════════════╝{RESET}")
        print("Commands:")
        print("  l              – list engines")
        print("  t <engine>     – tail last 20 lines")
        print("  s <engine>     – stream log")
        print("  start <engine> – start an engine (if not running)")
        print("  stop <engine>  – stop an engine")
        print("  stopall        – stop all engines")
        print("  o <prompt>     – query Ollama")
        print("  q              – quit")
        print()
        cmd = input(f"{GREEN}> {RESET}").strip()
        if not cmd:
            continue
        parts = cmd.split(maxsplit=1)
        action = parts[0].lower()
        arg = parts[1] if len(parts) > 1 else None

        if action == 'l':
            engines = list_engines()
            print(f"{BOLD}Engines found:{RESET}")
            for e in engines:
                print(f"  {e}")
        elif action == 't' and arg:
            tail_log(arg)
        elif action == 's' and arg:
            stream_log(arg)
        elif action == 'start' and arg:
            start_engine(arg)
        elif action == 'stop' and arg:
            stop_engine(arg)
        elif action == 'stopall':
            stop_all()
        elif action == 'o' and arg:
            query_ollama(arg)
        elif action == 'q':
            print("Exiting monitor.")
            break
        else:
            print(f"{RED}Invalid command.{RESET}")

if __name__ == "__main__":
    main()
