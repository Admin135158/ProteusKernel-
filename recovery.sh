#!/bin/bash
# morpheus_recovery.sh
# ONE RUN. Fixes everything. Zero compiled extensions.

set -e
cd ~
echo "[RECOVERY] Morpheus sovereign infrastructure recovery starting..."

# 1. Kill dpkg locks from the failed package install
echo "[1] Killing package manager locks..."
rm -f /data/data/com.termux/files/usr/var/lib/dpkg/lock-frontend 2>/dev/null || true
rm -f /data/data/com.termux/files/usr/var/lib/dpkg/lock 2>/dev/null || true

# 2. Ensure directory structure
echo "[2] Creating directory tree..."
mkdir -p ~/ProteusKernel/{bin,logs,run/pids,config,src}
mkdir -p ~/ProteusKernel/logs/{engines,audit,health}

# 3. Destroy and recreate the broken venv
cd ~/ProteusKernel
echo "[3] Creating fresh Python virtual environment..."
rm -rf venv
python -m venv venv
source venv/bin/activate

# 4. Install ONLY pure-Python packages (NO compilation, NO rust, NO C extensions)
echo "[4] Installing dependencies (pure Python only)..."
pip install --upgrade pip
pip install rich --no-cache-dir

# 5. Create ports.json using Python (safe from bash interpretation)
echo "[5] Writing port registry..."
python3 << 'PYEOF'
import json
from pathlib import Path
BASE = Path.home() / 'ProteusKernel'
ports = {
    "registry": {
        "pk_swarm":      {"port": 15001, "proto": "tcp", "tier": "core", "critical": True},
        "pk_heartbeat":  {"port": 15002, "proto": "tcp", "tier": "core", "critical": True},
        "gatekeeper":    {"port": 15003, "proto": "tcp", "tier": "core", "critical": True},
        "supervisor":    {"port": 15004, "proto": "tcp", "tier": "core", "critical": True},
        "bodyguard":     {"port": 15005, "proto": "tcp", "tier": "core", "critical": True},
        "pk_zayden":     {"port": 15006, "proto": "tcp", "tier": "bridge", "critical": False},
        "pk_gotem":      {"port": 15007, "proto": "tcp", "tier": "bridge", "critical": False},
        "swarm_gossip":  {"port": 15008, "proto": "udp", "tier": "core", "critical": True},
        "zayden_unified":{"port": 15009, "proto": "tcp", "tier": "bridge", "critical": False},
        "dna_binary":    {"port": 15010, "proto": "tcp", "tier": "utility", "critical": False},
        "chaos_engine":  {"port": 15011, "proto": "tcp", "tier": "elmalo", "critical": False},
        "kernel_bridge": {"port": 15012, "proto": "tcp", "tier": "elmalo", "critical": False},
        "arbitration":   {"port": 15013, "proto": "tcp", "tier": "elmalo", "critical": False},
        "initiation":    {"port": 15014, "proto": "tcp", "tier": "elmalo", "critical": False},
        "scs_main":      {"port": 15015, "proto": "tcp", "tier": "orchestrator", "critical": True}
    },
    "dynamic_range": {"min": 20000, "max": 25000},
    "version": "1.0.0"
}
with open(BASE / 'config' / 'ports.json', 'w') as f:
    json.dump(ports, f, indent=2)
print("[OK] ports.json created")
PYEOF

# 6. Create the supervisor (stdlib ONLY, parses /proc directly, NO psutil)
echo "[6] Writing termux_supervisor.py..."
python3 << 'PYEOF'
code = r'''#!/usr/bin/env python3
import json, os, sys, time, signal, errno, socket, subprocess
from pathlib import Path
from datetime import datetime

BASE = Path.home() / 'ProteusKernel'
PID_DIR = BASE / 'run' / 'pids'
LOG_DIR = BASE / 'logs' / 'engines'
CONFIG = BASE / 'config' / 'ports.json'

PID_DIR.mkdir(parents=True, exist_ok=True)
LOG_DIR.mkdir(parents=True, exist_ok=True)

def load_ports():
    with open(CONFIG) as f:
        return json.load(f)

def port_free(port, proto='tcp'):
    try:
        kind = socket.SOCK_STREAM if proto == 'tcp' else socket.SOCK_DGRAM
        s = socket.socket(socket.AF_INET, kind)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(('127.0.0.1', port))
        s.close()
        return True
    except OSError as e:
        if e.errno in (errno.EADDRINUSE, errno.EACCES):
            return False
        raise

def pid_alive(pid):
    return os.path.exists(f'/proc/{pid}')

def proc_state(pid):
    try:
        with open(f'/proc/{pid}/stat') as f:
            return f.read().split()[2]
    except:
        return '?'

def scan_ports():
    ports = load_ports()
    print(f"{'ENGINE':<20} {'PORT':<8} {'PROTO':<6} {'STATUS':<10}")
    print("-" * 50)
    for name, cfg in ports['registry'].items():
        port = cfg['port']
        free = port_free(port, cfg['proto'])
        status = 'FREE' if free else 'IN USE'
        print(f"{name:<20} {port:<8} {cfg['proto']:<6} {status:<10}")

def resolve_ports():
    print('[MORPHEUS] Resolving port conflicts...')
    ports = load_ports()
    migrated = []
    for name, cfg in list(ports['registry'].items()):
        port = cfg['port']
        if not port_free(port, cfg['proto']):
            for pid_file in PID_DIR.glob('*.pid'):
                try:
                    pid = int(pid_file.read_text().strip())
                    if pid_alive(pid):
                        with open(f'/proc/{pid}/cmdline') as f:
                            if str(port) in f.read().replace('\x00', ' '):
                                print(f'[KILL] Stale {name} (PID {pid})')
                                os.kill(pid, signal.SIGTERM)
                                time.sleep(0.5)
                except:
                    pass
            if not port_free(port, cfg['proto']):
                dyn = ports['dynamic_range']
                for p in range(dyn['min'], dyn['max']):
                    if port_free(p, cfg['proto']):
                        print(f'[MIGRATE] {name}: {port} -> {p}')
                        ports['registry'][name]['port'] = p
                        ports['registry'][name]['original_port'] = port
                        migrated.append((name, port, p))
                        break
    if migrated:
        with open(CONFIG, 'w') as f:
            json.dump(ports, f, indent=2)
        print(f'[OK] {len(migrated)} ports migrated')
    else:
        print('[OK] All ports clean')

def start_engine(name, cmdline, port, env=None):
    pid_file = PID_DIR / f'{name}.pid'
    if pid_file.exists():
        try:
            old = int(pid_file.read_text().strip())
            if pid_alive(old):
                print(f'[SKIP] {name} already running (PID {old})')
                return True
        except:
            pass
    environment = os.environ.copy()
    if env: environment.update(env)
    environment['PK_ENGINE'] = name
    environment['PK_PORT'] = str(port)
    log_file = LOG_DIR / f'{name}.log'
    try:
        proc = subprocess.Popen(
            cmdline,
            stdout=open(log_file, 'a'),
            stderr=subprocess.STDOUT,
            cwd=str(BASE),
            env=environment,
            start_new_session=True
        )
        pid_file.write_text(str(proc.pid))
        print(f'[START] {name} -> PID {proc.pid} (port {port})')
        time.sleep(0.3)
        return True
    except Exception as e:
        print(f'[ERROR] {name}: {e}')
        return False

def stop_engine(name):
    pid_file = PID_DIR / f'{name}.pid'
    if pid_file.exists():
        try:
            pid = int(pid_file.read_text().strip())
            if pid_alive(pid):
                os.kill(pid, signal.SIGTERM)
                time.sleep(0.5)
                if pid_alive(pid):
                    os.kill(pid, signal.SIGKILL)
        except:
            pass
        pid_file.unlink()
    print(f'[STOP] {name}')

def stop_all():
    for pid_file in list(PID_DIR.glob('*.pid')):
        stop_engine(pid_file.stem)

def status():
    ports = load_ports()
    print(f"\n{'ENGINE':<20} {'PORT':<8} {'PID':<8} {'STATUS':<8}")
    print("-" * 50)
    for name, cfg in ports['registry'].items():
        port = cfg['port']
        pid_file = PID_DIR / f'{name}.pid'
        pid_str = '-'
        status = 'DOWN'
        if pid_file.exists():
            try:
                pid = int(pid_file.read_text().strip())
                if pid_alive(pid):
                    state = proc_state(pid)
                    status = 'UP' if state != 'Z' else 'ZOMBIE'
                    pid_str = str(pid)
                else:
                    status = 'DEAD'
            except:
                status = 'ERROR'
        print(f'{name:<20} {port:<8} {pid_str:<8} {status:<8}')

if __name__ == '__main__':
    cmd = sys.argv[1] if len(sys.argv) > 1 else 'status'
    if cmd == 'scan': scan_ports()
    elif cmd == 'resolve': resolve_ports()
    elif cmd == 'status': status()
    elif cmd == 'start':
        name = sys.argv[2]
        port = int(sys.argv[3]) if len(sys.argv) > 3 else 15001
        start_engine(name, [sys.executable, '-c', f'import time; print("{name} on {port}"); time.sleep(99999)'], port)
    elif cmd == 'stop': stop_engine(sys.argv[2])
    elif cmd == 'stopall': stop_all()
    else:
        print('Usage: termux_supervisor.py [scan|resolve|status|start NAME PORT|stop NAME|stopall]')
'''
with open(Path.home() / 'ProteusKernel' / 'src' / 'termux_supervisor.py', 'w') as f:
    f.write(code)
print("[OK] termux_supervisor.py created")
PYEOF

chmod +x ~/ProteusKernel/src/termux_supervisor.py

# 7. Create the Rich dashboard (NO psutil, parses /proc directly)
echo "[7] Writing termux_dashboard.py..."
python3 << 'PYEOF'
code = r'''#!/usr/bin/env python3
import json, os, time
from pathlib import Path
from datetime import datetime
from rich.console import Console
from rich.table import Table
from rich.panel import Panel
from rich.live import Live
from rich.text import Text
from rich.layout import Layout

BASE = Path.home() / 'ProteusKernel'
PID_DIR = BASE / 'run' / 'pids'
CONFIG = BASE / 'config' / 'ports.json'

console = Console()

def pid_alive(pid):
    return os.path.exists(f'/proc/{pid}')

def proc_state(pid):
    try:
        with open(f'/proc/{pid}/stat') as f:
            return f.read().split()[2]
    except:
        return '?'

def load_ports():
    with open(CONFIG) as f:
        return json.load(f)

class Dashboard:
    def __init__(self):
        self.layout = Layout()
        self.layout.split_column(
            Layout(name='header', size=3),
            Layout(name='main', ratio=1),
            Layout(name='footer', size=3)
        )

    def make_header(self):
        t = Text()
        t.append(' MORPHEUS ', style='bold white on blue')
        t.append(' Sovereign AI Infrastructure ', style='bold cyan')
        t.append(f' {datetime.now().strftime("%H:%M:%S")} ', style='dim')
        return Panel(t, border_style='blue')

    def make_table(self):
        table = Table(border_style='cyan', expand=True)
        table.add_column('Engine', style='bold', min_width=15)
        table.add_column('Port', justify='right', min_width=6)
        table.add_column('Status', justify='center', min_width=8)
        table.add_column('PID', justify='right', min_width=6)
        
        ports = load_ports()
        up = down = 0
        
        for name, cfg in ports['registry'].items():
            port = cfg['port']
            pid_file = PID_DIR / f'{name}.pid'
            status = '[red]DOWN[/red]'
            pid_str = '-'
            
            if pid_file.exists():
                try:
                    pid = int(pid_file.read_text().strip())
                    if pid_alive(pid):
                        state = proc_state(pid)
                        if state == 'Z':
                            status = '[yellow]ZOMBIE[/yellow]'
                        else:
                            status = '[green]UP[/green]'
                            up += 1
                        pid_str = str(pid)
                    else:
                        down += 1
                except:
                    down += 1
            else:
                down += 1
            
            table.add_row(name, str(port), status, pid_str)
        
        color = 'green' if down == 0 else 'yellow' if up > 0 else 'red'
        return Panel(table, title=f'Engines: {up} UP / {down} DOWN', border_style=color)

    def make_footer(self):
        t = Text()
        t.append(' Ctrl+C to exit ', style='bold red on black')
        return Panel(t, border_style='dim')

    def refresh(self):
        self.layout['header'].update(self.make_header())
        self.layout['main'].update(self.make_table())
        self.layout['footer'].update(self.make_footer())
        return self.layout

    def run(self):
        with Live(self.refresh(), refresh_per_second=2, screen=True) as live:
            while True:
                time.sleep(0.5)
                live.update(self.refresh())

if __name__ == '__main__':
    try:
        Dashboard().run()
    except KeyboardInterrupt:
        console.print('\n[bold red]Dashboard closed.[/bold red]')
'''
with open(Path.home() / 'ProteusKernel' / 'src' / 'termux_dashboard.py', 'w') as f:
    f.write(code)
print("[OK] termux_dashboard.py created")
PYEOF

chmod +x ~/ProteusKernel/src/termux_dashboard.py

# 8. Create the unified launcher
echo "[8] Writing launch_stable.sh..."
cat > ~/ProteusKernel/launch_stable.sh << 'SHEOF'
#!/bin/bash
cd ~/ProteusKernel
source venv/bin/activate 2>/dev/null || true

case "${1:-status}" in
    status)    python src/termux_supervisor.py status ;;
    scan)      python src/termux_supervisor.py scan ;;
    resolve)   python src/termux_supervisor.py resolve ;;
    start)     python src/termux_supervisor.py start "$2" "$3" ;;
    stop)      python src/termux_supervisor.py stop "$2" ;;
    stopall)   python src/termux_supervisor.py stopall ;;
    dash|dashboard) python src/termux_dashboard.py ;;
    clean)     python src/termux_supervisor.py stopall; rm -f run/pids/*.pid; echo "[OK] Clean slate" ;;
    *)         echo "Usage: $0 [status|scan|resolve|start NAME PORT|stop NAME|stopall|dashboard|clean]" ;;
esac
SHEOF

chmod +x ~/ProteusKernel/launch_stable.sh

echo ""
echo "========================================"
echo "[MORPHEUS] RECOVERY COMPLETE"
echo "========================================"
echo ""
echo "Your sovereign infrastructure is ready."
echo ""
echo "Commands:"
echo "  cd ~/ProteusKernel"
echo "  ./launch_stable.sh status      # Engine status table"
echo "  ./launch_stable.sh scan        # Port scan"
echo "  ./launch_stable.sh resolve     # Fix all port conflicts"
echo "  ./launch_stable.sh dashboard   # Rich interactive TUI"
echo "  ./launch_stable.sh clean       # Nuclear reset"
echo ""
echo "To start an engine for testing:"
echo "  ./launch_stable.sh start pk_swarm 15001"
echo "  ./launch_stable.sh stop pk_swarm"
echo ""

