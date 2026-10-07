#!/usr/bin/env python3
import os
import sys
import time
import random
import threading
import queue
import subprocess
import re
from datetime import datetime

# ANSI colors
GREEN = "\033[92m"
CYAN = "\033[96m"
YELLOW = "\033[93m"
RED = "\033[91m"
BOLD = "\033[1m"
RESET = "\033[0m"

# ----------------------------------------------------------------------
# 1. CLEANUP
# ----------------------------------------------------------------------
def cleanup():
    for name in ["pk_", "gatekeeper", "supervisor", "bodyguard", "swarm", "heartbeat", "dna_binary", "zayden", "scs_main", "chaos_engine", "kernel_bridge", "arbitration", "initiation", "cpp_push", "proteus"]:
        os.system(f"pkill -f '{name}' 2>/dev/null || true")
cleanup()

# ----------------------------------------------------------------------
# 2. DISCOVER BINARIES IN CURRENT DIR
# ----------------------------------------------------------------------
CWD = os.getcwd()
binaries = []
for f in os.listdir(CWD):
    full = os.path.join(CWD, f)
    if not os.path.isfile(full):
        continue
    if f.startswith('.') or f.endswith('.cpp') or f.endswith('.h') or f.endswith('.pyc'):
        continue
    if os.access(full, os.X_OK) or f.endswith('.py'):
        binaries.append(full)

print(f"{BOLD}{GREEN}[*] Found {len(binaries)} executables/scripts.{RESET}")

# ----------------------------------------------------------------------
# 3. START ENGINES
# ----------------------------------------------------------------------
queues = {}
processes = {}

def start_engine(cmd, name):
    q = queue.Queue()
    queues[name] = q
    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                bufsize=0, universal_newlines=True)
        processes[name] = proc
        def reader():
            for line in proc.stdout:
                q.put(line.strip())
        threading.Thread(target=reader, daemon=True).start()
        return True
    except Exception as e:
        q.put(f"[ERROR] {e}")
        return False

for exe in binaries:
    name = os.path.basename(exe)
    if name.endswith('.py'):
        cmd = ["python3", exe]
    else:
        cmd = [exe]
    start_engine(cmd, name)

print(f"{GREEN}[*] Launched {len(processes)} engines.{RESET}")

# ----------------------------------------------------------------------
# 4. OLLAMA COMMAND HANDLER
# ----------------------------------------------------------------------
def run_ollama(prompt):
    try:
        result = subprocess.run(["ollama", "run", "tinyllama", prompt], capture_output=True, text=True, timeout=30)
        return result.stdout.strip() or result.stderr.strip()
    except Exception as e:
        return f"Error: {e}"

# ----------------------------------------------------------------------
# 5. MAIN LOOP – PRINT LOGS + COMMAND PROMPT
# ----------------------------------------------------------------------
def print_banner():
    banner = f"""
{BOLD}{CYAN}  ╔═══════════════════════════════════════════════╗
  ║   ⚡ MORPHEUS INNOVATIONS – LIVE DEMO ⚡   ║
  ║       SOVEREIGN AI INFRASTRUCTURE          ║
  ╚═══════════════════════════════════════════════╝{RESET}
"""
    print(banner)
    print(f"{YELLOW}Type 'ollama <your question>' to query local AI.")
    print(f"Type 'quit' to exit.{RESET}\n")

print_banner()

# We'll run the log printer in a thread, and handle input in main thread.
# But input blocks, so we'll use a non-blocking approach: we'll print logs in a thread,
# and use a separate thread to read input with a timeout.
# Simpler: just use a loop that prints logs and checks if there's input available (via select).
# But on Termux, select on stdin may not work well. We'll use a simple approach:
# We'll print logs in a thread, and in main thread we'll ask for input after each log batch.
# Not ideal, but stable.

# We'll use a queue for user commands, and read input in a separate thread.
command_queue = queue.Queue()

def input_thread():
    while True:
        try:
            cmd = input(f"{BOLD}{GREEN}>>> {RESET}")
            command_queue.put(cmd)
        except:
            break

threading.Thread(target=input_thread, daemon=True).start()

# Log display loop
log_cache = {name: [] for name in queues.keys()}
running = True
while running:
    # Print any new logs from all engines
    any_new = False
    for name, q in queues.items():
        while not q.empty():
            line = q.get()
            # Truncate if too long
            if len(line) > 100:
                line = line[:97] + "..."
            # Color based on content
            if "ERROR" in line or "error" in line:
                color = RED
            elif "WARN" in line or "warn" in line:
                color = YELLOW
            else:
                color = CYAN
            print(f"{color}[{name[:12]:<12}] {line}{RESET}")
            any_new = True
    if any_new:
        # If we printed logs, flush and show prompt again
        sys.stdout.flush()
        # The input thread will handle the prompt.

    # Check for commands
    try:
        cmd = command_queue.get_nowait()
        if cmd.lower() == 'quit':
            running = False
            break
        elif cmd.lower().startswith('ollama '):
            prompt = cmd[7:].strip()
            print(f"{YELLOW}🤔 Thinking...{RESET}")
            response = run_ollama(prompt)
            print(f"{GREEN}{BOLD}OLLAMA:{RESET} {response}")
        else:
            print(f"{RED}Unknown command. Use 'ollama <text>' or 'quit'.{RESET}")
    except queue.Empty:
        pass

    time.sleep(0.2)

# Cleanup
print(f"{RED}Shutting down...{RESET}")
for proc in processes.values():
    try:
        proc.terminate()
    except:
        pass
print(f"{GREEN}Done.{RESET}")
