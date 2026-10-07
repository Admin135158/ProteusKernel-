#!/usr/bin/env python3
import json, os, sys, time, signal, errno, socket, subprocess
from pathlib import Path

BASE = Path.home() / 'ProteusKernel'
BIN_DIR = BASE / 'bin'
PID_DIR = BASE / 'run' / 'pids'
LOG_DIR = BASE / 'logs' / 'engines'
CONFIG = BASE / 'config' / 'ports.json'

PID_DIR.mkdir(parents=True, exist_ok=True)
LOG_DIR.mkdir(parents=True, exist_ok=True)

ENGINE_MANIFEST = {
    'pk_swarm':       ('cpp', 'pk_swarm',       ['--sync7'],           {'PK_MODE': 'sovereign'}),
    'pk_heartbeat':   ('cpp', 'pk_heartbeat',    ['--gossip'],          {'HB_INTERVAL': '1000'}),
    'gatekeeper':     ('cpp', 'gatekeeper',      ['--acl', 'strict'],   {}),
    'supervisor':     ('cpp', 'supervisor',      ['--arbitrate'],       {}),
    'bodyguard':      ('cpp', 'bodyguard',       ['--anti-sabotage'],   {'TRUCE_PROTOCOL': 'enabled'}),
    'swarm_gossip':   ('cpp', 'swarm_gossip',    ['--discovery', '--proto', 'udp'], {}),
    'pk_zayden':      ('cpp', 'pk_zayden',       ['--bridge'],          {'ZAYDEN_MODE': 'federated'}),
    'pk_gotem':       ('cpp', 'pk_gotem',        ['--hash', 'sha256'],  {}),
    'dna_binary':     ('cpp', 'dna_binary',      ['--encode'],          {}),
    'zayden_unified': ('python', 'zayden_unified.py', [],              {'MODELS': 'gemini,claude,deepseek,ollama'}),
    'chaos_engine':   ('python', 'chaos_engine.py',   ['--offline'],   {'CHAOS_MODE': 'lorenz', 'NETWORK': 'disabled'}),
    'kernel_bridge':  ('python', 'kernel_bridge.py',  [],              {}),
    'arbitration':    ('python', 'arbitration.py',    [],              {}),
    'initiation':     ('python', 'initiation.py',     [],              {}),
    'scs_main':       ('python', 'scs_main.py',       ['--dashboard'], {'HOLO_INVARIANT': 'enabled', 'GGSE_LAYERS': '5'}),
}

TIER_ORDER = ['core', 'bridge', 'utility', 'elmalo', 'orchestrator']

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

def tier_of(name):
    return load_ports()['registry'].get(name, {}).get('tier', 'unknown')

def build_cmdline(name, port):
    if name not in ENGINE_MANIFEST:
        return None
    etype, path, extra_args, _ = ENGINE_MANIFEST[name]

    # Try real binary first
    if etype == 'cpp':
        binary = str(BIN_DIR / path)
        if Path(binary).exists():
            return [binary] + extra_args + ['--port', str(port)]

    # Try real Python script
    if etype == 'python':
        script = str(BASE / path)
        if Path(script).exists():
            return [sys.executable, script, '--port', str(port)] + extra_args

    # FALLBACK: use hybrid stub
    stub = str(BASE / 'stub_engine.py')
    if Path(stub).exists():
        return [sys.executable, stub]

    return None

def start_engine(name):
    ports = load_ports()
    cfg = ports['registry'].get(name)
    if not cfg:
        print(f'[ERROR] Unknown engine: {name}')
        return False

    port = cfg['port']
    pid_file = PID_DIR / f'{name}.pid'

    if pid_file.exists():
        try:
            old = int(pid_file.read_text().strip())
            if pid_alive(old):
                print(f'[SKIP] {name} already running (PID {old})')
                return True
        except:
            pass

    cmdline = build_cmdline(name, port)
    if cmdline is None:
        print(f'[MISSING] {name}: no binary, script, or stub found')
        return False

    env = os.environ.copy()
    if name in ENGINE_MANIFEST:
        _, _, _, extra_env = ENGINE_MANIFEST[name]
        env.update(extra_env)
    env['PK_ENGINE'] = name
    env['PK_PORT'] = str(port)
    env['PK_TIER'] = tier_of(name)
    env['PK_BASE_DIR'] = str(BASE)

    log_file = LOG_DIR / f'{name}.log'
    try:
        proc = subprocess.Popen(
            cmdline,
            stdout=open(log_file, 'a'),
            stderr=subprocess.STDOUT,
            cwd=str(BASE),
            env=env,
            start_new_session=True
        )
        pid_file.write_text(str(proc.pid))
        mode = 'NATIVE' if (name in ENGINE_MANIFEST and ((ENGINE_MANIFEST[name][0] == 'cpp' and Path(BIN_DIR / ENGINE_MANIFEST[name][1]).exists()) or (ENGINE_MANIFEST[name][0] == 'python' and Path(BASE / ENGINE_MANIFEST[name][1]).exists()))) else 'STUB'
        print(f'[START] {name} -> PID {proc.pid} (port {port}) [{mode}]')
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

def start_tier(tier_name):
    ports = load_ports()
    engines = [n for n, c in ports['registry'].items() if c.get('tier') == tier_name]
    if not engines:
        print(f'[ERROR] No engines in tier: {tier_name}')
        return
    print(f'\n[TIER] Launching {tier_name} ({len(engines)} engines)')
    for name in engines:
        start_engine(name)
    time.sleep(1)

def start_all():
    print('[MORPHEUS] === STAGED LAUNCH ===')
    for tier in TIER_ORDER:
        start_tier(tier)
    print('[MORPHEUS] === ALL TIERS DEPLOYED ===')

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

def status():
    ports = load_ports()
    print(f"\n{'ENGINE':<20} {'TIER':<12} {'PORT':<8} {'PID':<8} {'STATUS':<8} {'MODE':<8}")
    print("-" * 65)
    for name, cfg in ports['registry'].items():
        port = cfg['port']
        tier = cfg.get('tier', '?')
        pid_file = PID_DIR / f'{name}.pid'
        pid_str = '-'
        status = 'DOWN'
        mode = 'MISSING'

        if name in ENGINE_MANIFEST:
            etype, path, _, _ = ENGINE_MANIFEST[name]
            if etype == 'cpp' and Path(BIN_DIR / path).exists():
                mode = 'NATIVE'
            elif etype == 'python' and Path(BASE / path).exists():
                mode = 'NATIVE'
            elif Path(BASE / 'stub_engine.py').exists():
                mode = 'STUB'

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
        print(f'{name:<20} {tier:<12} {port:<8} {pid_str:<8} {status:<8} {mode:<8}')

if __name__ == '__main__':
    cmd = sys.argv[1] if len(sys.argv) > 1 else 'status'
    if cmd == 'scan': scan_ports()
    elif cmd == 'resolve': resolve_ports()
    elif cmd == 'status': status()
    elif cmd == 'start':
        name = sys.argv[2]
        start_engine(name)
    elif cmd == 'tier':
        start_tier(sys.argv[2])
    elif cmd == 'all':
        start_all()
    elif cmd == 'stop': stop_engine(sys.argv[2])
    elif cmd == 'stopall': stop_all()
    else:
        print('Usage: termux_supervisor.py [scan|resolve|status|start NAME|tier TIER|all|stop NAME|stopall]')
