#!/usr/bin/env python3
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
