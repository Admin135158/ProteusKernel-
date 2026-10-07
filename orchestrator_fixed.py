#!/usr/bin/env python3
import curses
import time
import threading
import queue
import subprocess
import os
import re
import random
import sys
import signal
from datetime import datetime

# ----------------------------------------------------------------------
# CLEANUP – Kill leftovers on start
# ----------------------------------------------------------------------
def cleanup_orphans():
    """Kill any processes that might be using our ports."""
    kill_list = [
        "pk_", "gatekeeper", "supervisor", "bodyguard",
        "swarm", "heartbeat", "dna_binary", "zayden",
        "scs_main", "chaos_engine", "kernel_bridge",
        "arbitration", "initiation", "cpp_push", "proteus"
    ]
    for proc_name in kill_list:
        os.system(f"pkill -f '{proc_name}' 2>/dev/null || true")

# Run cleanup at start
cleanup_orphans()

# ----------------------------------------------------------------------
# CONFIG – Look in current directory first
# ----------------------------------------------------------------------
CWD = os.getcwd()
BIN_DIRS = [
    CWD,
    os.path.join(CWD, "bin"),
    os.path.join(CWD, "build"),
]

REPO_MAP = {
    "ProteusKernel": re.compile(r"^(pk_|gatekeeper|supervisor|bodyguard|swarm|heartbeat|dna_binary|proteus)"),
    "Zayden": re.compile(r"^(zayden_|archive-src-|Zayden-)"),
    "ElMalo": re.compile(r"^elmalo|chaos_engine|kernel_bridge|arbitration|initiation"),
    "Other": re.compile(r".*"),
}

OLLAMA_MODEL = os.environ.get("OLLAMA_MODEL", "tinyllama")

# ----------------------------------------------------------------------
# DISCOVER BINARIES AND PYTHON SCRIPTS
# ----------------------------------------------------------------------
binaries = []
for d in BIN_DIRS:
    if not os.path.isdir(d):
        continue
    for f in os.listdir(d):
        full_path = os.path.join(d, f)
        if not os.path.isfile(full_path):
            continue
        if f.startswith('.') or f.endswith('.cpp') or f.endswith('.h') or f.endswith('.pyc'):
            continue
        is_exec = os.access(full_path, os.X_OK)
        is_py = f.endswith('.py')
        if is_py or is_exec:
            repo = "Other"
            for name, pattern in REPO_MAP.items():
                if pattern.match(f):
                    repo = name
                    break
            binaries.append((full_path, f, repo, is_py))

seen = set()
unique = []
for full_path, f, repo, is_py in binaries:
    if f not in seen:
        seen.add(f)
        unique.append((full_path, f, repo, is_py))

print(f"[*] Found {len(unique)} executables/scripts.")

engines_by_repo = {}
for full_path, f, repo, is_py in unique:
    engines_by_repo.setdefault(repo, []).append((full_path, f, is_py))

# ----------------------------------------------------------------------
# START ENGINES WITH PORT CONFLICT HANDLING
# ----------------------------------------------------------------------
queues = {}
processes = {}

def start_engine(name, cmd, retries=3):
    q = queue.Queue()
    queues[name] = q
    for attempt in range(retries):
        try:
            proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                    bufsize=0, universal_newlines=True)
            processes[name] = proc
            def reader():
                for line in proc.stdout:
                    q.put(f"[{datetime.now().strftime('%H:%M:%S')}] {line.strip()}")
            threading.Thread(target=reader, daemon=True).start()
            return True
        except Exception as e:
            q.put(f"[ERROR] Attempt {attempt+1}: {e}")
            time.sleep(0.5)
    return False

# Start all engines
for repo, bins in engines_by_repo.items():
    for full_path, f, is_py in bins:
        name = f"{repo}/{f}"
        if is_py:
            cmd = ["python3", full_path]
        else:
            cmd = [full_path]
        start_engine(name, cmd)

# ----------------------------------------------------------------------
# OLLAMA INTERACTION
# ----------------------------------------------------------------------
ollama_queue = queue.Queue()
ollama_response = "Awaiting your first prompt."

def run_ollama(prompt):
    global ollama_response
    try:
        cmd = ["ollama", "run", OLLAMA_MODEL, prompt]
        proc = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
        if proc.returncode == 0:
            response = proc.stdout.strip()
        else:
            response = f"Error: {proc.stderr.strip()}"
    except Exception as e:
        response = f"Exception: {e}"
    ollama_response = response
    ollama_queue.put(response)

# ----------------------------------------------------------------------
# AWAKENING SEQUENCE (Hybrid Glitch)
# ----------------------------------------------------------------------
def awakening_sequence(stdscr):
    curses.curs_set(0)
    h, w = stdscr.getmaxyx()
    lines = [
        " ⚡ MORPHEUS INNOVATIONS – MACHINE AWAKENING ⚡ ",
        " [FTCoE] Resonance field initiated...",
        " [HOLO] Append-only log active – Merkle root: 0x9f3a…",
        " [SYNC-7] Swarm coherence: 78%",
        " [ARCHETYPE] Trial #1 – Chaos/Order balance...",
        " [ELMALO] Offline intelligence synthesising...",
        " [PROTEUS] Digital Bodyguard armed",
        " [ZAYDEN] Arbitration Council ready",
        " ⚡ UNIFIED CONVERGENCE – ALL SYSTEMS NOMINAL ⚡ ",
    ]
    for phase, line in enumerate(lines):
        stdscr.clear()
        if random.random() < 0.3:
            for _ in range(5):
                y = random.randint(0, h-1)
                x = random.randint(0, w-1)
                try:
                    stdscr.addstr(y, x, random.choice("!@#$%^&*()_+{}|:<>?~`"))
                except:
                    pass
                stdscr.refresh()
                time.sleep(0.05)
        for attempt in range(3):
            attr = curses.color_pair(random.choice([1,2,3,4])) | curses.A_BOLD if attempt % 2 == 0 else curses.color_pair(1)
            stdscr.attron(attr)
            try:
                stdscr.addstr(h//2 + phase - 2, (w - len(line))//2, line)
            except:
                pass
            stdscr.attroff(attr)
            stdscr.refresh()
            time.sleep(0.15)
        for _ in range(3):
            stdscr.attron(curses.color_pair(3))
            stdscr.addstr(h//2 + phase - 1, (w - len("◈"))//2, "◈")
            stdscr.attroff(curses.color_pair(3))
            stdscr.refresh()
            time.sleep(0.1)
        time.sleep(0.3)
    for _ in range(5):
        stdscr.clear()
        msg = " ⚡ SYSTEMS CONVERGED – READY ⚡ "
        try:
            stdscr.attron(curses.color_pair(2) | curses.A_BOLD)
            stdscr.addstr(h//2, (w - len(msg))//2, msg)
            stdscr.attroff(curses.color_pair(2) | curses.A_BOLD)
        except:
            pass
        stdscr.refresh()
        time.sleep(0.2)
    time.sleep(1)

# ----------------------------------------------------------------------
# MAIN DASHBOARD
# ----------------------------------------------------------------------
def draw_dashboard(stdscr):
    global ollama_response
    curses.curs_set(1)
    curses.start_color()
    curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK)
    curses.init_pair(2, curses.COLOR_CYAN, curses.COLOR_BLACK)
    curses.init_pair(3, curses.COLOR_YELLOW, curses.COLOR_BLACK)
    curses.init_pair(4, curses.COLOR_RED, curses.COLOR_BLACK)

    h, w = stdscr.getmaxyx()
    if h < 24 or w < 80:
        stdscr.addstr(0, 0, "Terminal too small. Need 80x24. Press any key to continue anyway.")
        stdscr.getch()
        h, w = stdscr.getmaxyx()

    awakening_sequence(stdscr)

    log_area_height = h - 9
    chat_y = log_area_height + 1
    log_cache = {name: [] for name in queues.keys()}
    input_buffer = ""

    running = True
    while running:
        stdscr.clear()

        # Engine logs
        line_num = 0
        for repo, bins in engines_by_repo.items():
            if line_num >= log_area_height - 2:
                break
            stdscr.attron(curses.color_pair(1) | curses.A_BOLD)
            stdscr.addstr(line_num, 0, f" {repo} ")
            stdscr.attroff(curses.color_pair(1) | curses.A_BOLD)
            line_num += 1
            for full_path, f, is_py in bins:
                if line_num >= log_area_height - 2:
                    break
                name = f"{repo}/{f}"
                disp = f[:20]
                stdscr.addstr(line_num, 2, disp)
                line_num += 1
                q = queues.get(name)
                if q:
                    new_lines = []
                    while not q.empty():
                        try:
                            new_lines.append(q.get_nowait())
                        except queue.Empty:
                            break
                    if new_lines:
                        log_cache[name] = (log_cache.get(name, []) + new_lines)[-3:]
                if log_cache.get(name):
                    log_line = log_cache[name][0][:w-4]
                    if line_num < log_area_height - 2:
                        stdscr.addstr(line_num, 4, log_line)
                        line_num += 1

        # Separator
        stdscr.attron(curses.color_pair(3))
        stdscr.hline(chat_y - 1, 0, curses.ACS_HLINE, w)
        stdscr.attroff(curses.color_pair(3))

        # Ollama response
        response_lines = ollama_response.split('\n')
        for i, line in enumerate(response_lines[:2]):
            if i >= 2:
                break
            stdscr.addstr(chat_y + i, 0, f"OLLAMA: {line[:w-8]}")

        # Input prompt
        prompt_text = f"Prompt: {input_buffer}"
        stdscr.addstr(chat_y + 3, 0, prompt_text.ljust(w-1))
        stdscr.move(chat_y + 3, len(prompt_text))

        stdscr.refresh()
        time.sleep(0.1)

        # Keyboard
        key = stdscr.getch()
        if key == ord('q') or key == ord('Q'):
            running = False
        elif key == ord('\n') or key == ord('\r'):
            if input_buffer.strip():
                threading.Thread(target=run_ollama, args=(input_buffer,), daemon=True).start()
                ollama_response = "Thinking..."
                input_buffer = ""
        elif key == curses.KEY_BACKSPACE or key == 127:
            input_buffer = input_buffer[:-1]
        elif key >= 32 and key <= 126:
            input_buffer += chr(key)
        elif key == curses.KEY_RESIZE:
            h, w = stdscr.getmaxyx()
            log_area_height = h - 9
            chat_y = log_area_height + 1
            stdscr.clear()
            continue

    # Cleanup
    for proc in processes.values():
        try:
            proc.terminate()
        except:
            pass
    curses.endwin()

if __name__ == "__main__":
    # Run cleanup again before starting
    cleanup_orphans()
    time.sleep(1)
    curses.wrapper(draw_dashboard)
