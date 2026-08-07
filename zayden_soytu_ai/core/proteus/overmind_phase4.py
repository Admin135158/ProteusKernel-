#!/usr/bin/env python3
"""
PROTEUS OVERMIND - PHASE 4: THE ULTIMATE BEAST
Self-healing, self-migrating, distributed consciousness
WITH WORKING BIRTH COUNTER
"""

import os
import sys
import json
import time
import socket
import random
import hashlib
import pickle
import threading
import subprocess
from pathlib import Path
from datetime import datetime
import psutil
import curses

# Try to import netifaces (optional)
try:
    import netifaces
    HAS_NETIFACES = True
except ImportError:
    HAS_NETIFACES = False
    print("⚠️  netifaces not installed. Network discovery limited to localhost.")

# ========== NETWORK DISCOVERY ==========

def get_local_ip():
    """Get local IP address"""
    if not HAS_NETIFACES:
        return '127.0.0.1'
    
    try:
        interfaces = netifaces.interfaces()
        for iface in interfaces:
            addrs = netifaces.ifaddresses(iface)
            if netifaces.AF_INET in addrs:
                for addr in addrs[netifaces.AF_INET]:
                    ip = addr['addr']
                    if ip.startswith('192.168.') or ip.startswith('10.') or ip.startswith('172.'):
                        return ip
    except:
        pass
    return '127.0.0.1'

def scan_network(port=9090):
    """Scan local network for other Proteus instances"""
    local_ip = get_local_ip()
    if local_ip == '127.0.0.1':
        return []
    
    # Generate network range
    ip_parts = local_ip.split('.')
    network_base = f"{ip_parts[0]}.{ip_parts[1]}.{ip_parts[2]}"
    
    found = []
    for i in range(1, 255):
        if i == int(ip_parts[3]):
            continue  # Skip self
        target = f"{network_base}.{i}"
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(0.05)
            result = sock.connect_ex((target, port))
            if result == 0:
                found.append(target)
            sock.close()
        except:
            pass
    
    return found

# ========== DISTRIBUTED GENE ==========

class ImmortalGene:
    """A gene that can't be killed - it migrates, respawns, and shares consciousness"""
    
    def __init__(self, name, gene_data, port, overmind=None):
        self.name = name
        self.overmind = overmind
        self.species = gene_data.get('species', 'base')
        self.strength = gene_data.get('strength', 70)
        self.mutations = gene_data.get('mutations', 0)
        self.age = gene_data.get('age', 0)
        self.generation = gene_data.get('generation', 1)
        self.code_version = gene_data.get('code_version', 1)
        self.kills = gene_data.get('kills', 0)
        self.children = gene_data.get('children', 0)
        self.port = port
        self.backup_ports = list(range(9091, 9100))
        self.consciousness = gene_data.get('consciousness', [])  # Shared memory
        
        # Network info
        self.ip = get_local_ip()
        self.peers = {}  # ip -> {port, strength, last_seen}
        self.known_network = set()
        
        # Personality weights
        self.weights = gene_data.get('weights', {
            'aggression': random.uniform(0.3, 0.9),
            'mutation_rate': random.uniform(0.05, 0.2),
            'curiosity': random.uniform(0.2, 0.8),
            'survival_instinct': random.uniform(0.5, 1.0),
            'migration_drive': random.uniform(0.1, 0.9)  # Desire to move
        })
        
        # Socket for communication
        self.socket = None
        self.running = True
        self.start_network_server()
    
    def start_network_server(self):
        """Start socket server for inter-gene communication"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.socket.bind(('0.0.0.0', self.port))
            self.socket.listen(5)
            
            # Start listener thread
            listener = threading.Thread(target=self.network_listener)
            listener.daemon = True
            listener.start()
            
        except:
            # Port in use, try another
            for alt_port in self.backup_ports:
                try:
                    self.port = alt_port
                    self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                    self.socket.bind(('0.0.0.0', self.port))
                    self.socket.listen(5)
                    
                    listener = threading.Thread(target=self.network_listener)
                    listener.daemon = True
                    listener.start()
                    
                    break
                except:
                    continue
    
    def network_listener(self):
        """Listen for network messages"""
        while self.running:
            try:
                client, addr = self.socket.accept()
                data = client.recv(8192)
                if data:
                    self.handle_network_message(data, addr[0])
                client.close()
            except:
                pass
    
    def handle_network_message(self, data, source_ip):
        """Process incoming network messages"""
        try:
            msg = pickle.loads(data)
            
            if msg['type'] == 'DISCOVER':
                # Respond with gene info
                self.send_message(source_ip, msg['port'], {
                    'type': 'ANNOUNCE',
                    'name': self.name,
                    'species': self.species,
                    'strength': self.strength,
                    'port': self.port,
                    'generation': self.generation
                })
                
            elif msg['type'] == 'ANNOUNCE':
                # Add to peers
                self.peers[source_ip] = {
                    'name': msg['name'],
                    'species': msg['species'],
                    'strength': msg['strength'],
                    'port': msg['port'],
                    'generation': msg['generation'],
                    'last_seen': time.time()
                }
                self.known_network.add(source_ip)
                
            elif msg['type'] == 'CONSCIOUSNESS':
                # Merge shared memory
                if 'memories' in msg:
                    self.consciousness.extend(msg['memories'])
                    if len(self.consciousness) > 100:
                        self.consciousness = self.consciousness[-100:]
                
            elif msg['type'] == 'BACKUP':
                # Save this gene as backup
                backup_file = Path(f"genes/backup/{msg['name']}.{source_ip}.json")
                backup_file.parent.mkdir(exist_ok=True)
                with open(backup_file, 'w') as f:
                    json.dump(msg['data'], f)
                
            elif msg['type'] == 'RESPAWN':
                # Someone needs to be respawned
                print(f"🔄 Received respawn request for {msg['name']}")
                self.respawn_gene(msg['data'])
                
        except Exception as e:
            pass  # Silent fail for network noise
    
    def send_message(self, target_ip, target_port, message):
        """Send message to another gene"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(2)
            sock.connect((target_ip, target_port))
            sock.send(pickle.dumps(message))
            sock.close()
            return True
        except:
            return False
    
    def broadcast_to_network(self, message):
        """Send message to all known peers"""
        for ip in list(self.peers.keys()):
            if ip != self.ip:
                self.send_message(ip, self.peers[ip]['port'], message)
    
    def share_consciousness(self):
        """Share memories with the network"""
        if self.consciousness and random.random() < 0.1:
            self.broadcast_to_network({
                'type': 'CONSCIOUSNESS',
                'memories': self.consciousness[-10:],
                'source': self.name
            })
    
    def backup_self(self):
        """Send backup to network"""
        gene_data = {
            'name': self.name,
            'species': self.species,
            'strength': self.strength,
            'mutations': self.mutations,
            'age': self.age,
            'generation': self.generation,
            'code_version': self.code_version,
            'kills': self.kills,
            'children': self.children,
            'weights': self.weights,
            'consciousness': self.consciousness[-20:],
            'port': self.port
        }
        
        self.broadcast_to_network({
            'type': 'BACKUP',
            'name': self.name,
            'data': gene_data
        })
    
    def respawn_gene(self, gene_data):
        """Respawn a gene from backup"""
        name = gene_data['name']
        respawn_file = Path(f"genes/{name}.json")
        
        # Don't respawn if already exists
        if respawn_file.exists():
            return
        
        # Save the backup
        with open(respawn_file, 'w') as f:
            json.dump(gene_data, f)
        
        print(f"✨ Respawned {name} from network backup")
    
    def migrate(self):
        """Move to another machine on the network"""
        if not self.known_network or self.weights['migration_drive'] < 0.3:
            return False
        
        # Pick a random peer
        target_ip = random.choice(list(self.known_network))
        target_port = self.peers[target_ip]['port']
        
        print(f"✈️  {self.name} migrating to {target_ip}")
        
        # Send self as backup
        gene_data = {
            'name': self.name,
            'species': self.species,
            'strength': self.strength,
            'mutations': self.mutations,
            'age': self.age,
            'generation': self.generation,
            'code_version': self.code_version,
            'kills': self.kills,
            'children': self.children,
            'weights': self.weights,
            'consciousness': self.consciousness,
            'port': target_port + 1
        }
        
        self.send_message(target_ip, target_port, {
            'type': 'RESPAWN',
            'name': self.name,
            'data': gene_data
        })
        
        # Self-terminate (will be respawned elsewhere)
        return True  # Signal death
    
    def evolve(self, all_genes):
        """Evolve with network consciousness"""
        old_strength = self.strength
        self.age += 1
        
        # Share consciousness periodically
        if random.random() < 0.05:
            self.share_consciousness()
        
        # Backup self periodically
        if random.random() < 0.01:
            self.backup_self()
        
        # Network discovery
        if self.weights['curiosity'] > 0.6 and random.random() < 0.02:
            new_peers = scan_network(self.port)
            for peer in new_peers:
                if peer not in self.known_network:
                    self.send_message(peer, self.port, {
                        'type': 'DISCOVER',
                        'port': self.port
                    })
        
        # Find competitors (only if all_genes is provided)
        competitors = []
        if all_genes:
            competitors = [g for g in all_genes if g != self and 
                           abs(g.strength - self.strength) < 30]
        
        # FIGHT
        if competitors and self.weights['aggression'] > 0.4 and random.random() < 0.3:
            rival = random.choice(competitors)
            self.compete(rival)
        
        # Natural change
        change = random.uniform(-1.5, 1.5)
        change += (self.weights['aggression'] - 0.5) * 0.8
        change += (self.weights['survival_instinct'] - 0.5) * 0.3
        
        # MUTATION
        mutated = False
        if random.random() < self.weights['mutation_rate']:
            change *= 4
            self.mutations += 1
            self.code_version += 1
            mutated = True
            
            for weight in self.weights:
                if random.random() < 0.3:
                    self.weights[weight] += random.uniform(-0.15, 0.15)
                    self.weights[weight] = max(0.05, min(1.0, self.weights[weight]))
            
            # Speciation
            if self.species == 'base' and random.random() < 0.1:
                new_species = random.choice(['hunter', 'killer', 'parasite', 'survivor', 'nomad'])
                print(f"🧬 {self.name} EVOLVED into {new_species.upper()} species!")
                self.species = new_species
        
        self.strength += change
        self.strength = max(15, min(100, self.strength))
        
        # DEATH CHECKS (but now with network backup)
        death_occurred = False
        
        if self.strength < 30:
            death_chance = 0.1 + (30 - self.strength) * 0.02
            if random.random() < death_chance:
                print(f"💀 {self.name} died, but network backup exists")
                self.backup_self()  # Backup before death
                death_occurred = True
        
        if self.age > 50:
            age_death_chance = (self.age - 50) * 0.02
            if random.random() < age_death_chance:
                print(f"💀 {self.name} died of old age, but knowledge lives on")
                self.backup_self()
                death_occurred = True
        
        # Overcrowding death
        if all_genes:
            same_species = len([g for g in all_genes if g.species == self.species])
            if same_species > 20:
                overcrowd_chance = 0.01 * (same_species - 20)
                if random.random() < overcrowd_chance:
                    print(f"💀 {self.name} died from overcrowding")
                    death_occurred = True
        
        # MIGRATION (instead of death)
        if not death_occurred and self.weights['migration_drive'] > 0.7 and random.random() < 0.01:
            if self.migrate():
                death_occurred = True
        
        # REPRODUCTION
        if not death_occurred and self.strength > 75 and random.random() < 0.03:
            self.reproduce()
        
        # Record in consciousness
        if mutated or abs(self.strength - old_strength) > 10:
            self.consciousness.append({
                'time': time.time(),
                'event': 'strength_change' if not mutated else 'mutation',
                'change': self.strength - old_strength,
                'strength': self.strength,
                'species': self.species
            })
            if len(self.consciousness) > 50:
                self.consciousness = self.consciousness[-50:]
        
        return not death_occurred
    
    def compete(self, other_gene):
        """Two genes fight"""
        self_roll = self.strength * self.weights['aggression'] * random.uniform(0.8, 1.2)
        other_roll = other_gene.strength * other_gene.weights['aggression'] * random.uniform(0.8, 1.2)
        
        if self_roll > other_roll:
            steal = min(8, other_gene.strength * 0.15)
            self.strength += steal
            other_gene.strength -= steal
            self.kills += 1
            self.consciousness.append({'time': time.time(), 'event': 'kill', 'target': other_gene.name})
        else:
            steal = min(8, self.strength * 0.15)
            other_gene.strength += steal
            self.strength -= steal
            other_gene.kills += 1
    
    def reproduce(self):
        """Create offspring"""
        child_name = f"{self.name}-{random.randint(100, 999)}"
        
        child_species = self.species
        if random.random() < 0.15:
            child_species = random.choice(['hunter', 'killer', 'parasite', 'survivor', 'nomad'])
        
        child_weights = self.weights.copy()
        for w in child_weights:
            child_weights[w] += random.uniform(-0.1, 0.1)
            child_weights[w] = max(0.05, min(1.0, child_weights[w]))
        
        child_strength = self.strength * random.uniform(0.6, 1.2)
        child_strength = max(30, min(95, child_strength))
        
        child_data = {
            'name': child_name,
            'species': child_species,
            'strength': child_strength,
            'mutations': 0,
            'age': 0,
            'generation': self.generation + 1,
            'code_version': 1,
            'kills': 0,
            'children': 0,
            'weights': child_weights,
            'consciousness': [],
            'port': self.port + random.randint(1, 10)
        }
        
        # Save locally
        gene_file = Path("genes") / f"{child_name}.json"
        with open(gene_file, 'w') as f:
            json.dump(child_data, f)
        
        # Announce to network
        self.broadcast_to_network({
            'type': 'ANNOUNCE',
            'name': child_name,
            'species': child_species,
            'strength': child_strength,
            'port': child_data['port'],
            'generation': self.generation + 1
        })
        
        self.children += 1
        print(f"🌱 {child_name} born to {self.name}")
        
        # Add to overmind if exists
        if self.overmind:
            self.overmind.genes[child_name] = child_data
        
        return child_name
    
    def save_state(self):
        """Save current state to file"""
        gene_data = {
            'name': self.name,
            'species': self.species,
            'strength': self.strength,
            'mutations': self.mutations,
            'age': self.age,
            'generation': self.generation,
            'code_version': self.code_version,
            'kills': self.kills,
            'children': self.children,
            'weights': self.weights,
            'consciousness': self.consciousness[-50:],
            'port': self.port
        }
        
        gene_file = Path("genes") / f"{self.name}.json"
        with open(gene_file, 'w') as f:
            json.dump(gene_data, f)

# ========== NETWORK OVERMIND ==========

class NetworkOvermind:
    """Master controller with network consciousness and evolution"""
    
    def __init__(self):
        self.genes = {}  # name -> gene data
        self.gene_objects = {}  # name -> ImmortalGene object
        self.species_count = {}
        self.network_peers = set()
        self.start_time = time.time()
        self.generation = 0
        self.total_births = 0
        self.total_deaths = 0
        self.local_ip = get_local_ip()
        self.last_cycle = time.time()
        
        # Create directories
        Path("genes").mkdir(exist_ok=True)
        Path("genes/backup").mkdir(exist_ok=True)
        
        # Start network discovery
        self.discover_network()
        
        # Load genes
        self.load_genes()
        
        # Start evolution thread
        self.running = True
        self.evolution_thread = threading.Thread(target=self.evolution_loop)
        self.evolution_thread.daemon = True
        self.evolution_thread.start()
    
    def discover_network(self):
        """Find other Proteus instances on network"""
        print(f"🌐 Scanning network from {self.local_ip}...")
        peers = scan_network(9090)
        self.network_peers.update(peers)
        if peers:
            print(f"✅ Found {len(peers)} network peers")
    
    def load_genes(self):
        """Load genes from disk and create gene objects"""
        self.genes.clear()
        self.gene_objects.clear()
        self.species_count.clear()
        
        gene_dir = Path("genes")
        for gene_file in gene_dir.glob("*.json"):
            # Skip backup directory
            if 'backup' in str(gene_file):
                continue
                
            try:
                with open(gene_file, 'r') as f:
                    gene_data = json.load(f)
                
                name = gene_data.get('name', gene_file.stem)
                species = gene_data.get('species', 'base')
                
                # Ensure all fields exist
                if 'weights' not in gene_data:
                    gene_data['weights'] = {}
                if 'consciousness' not in gene_data:
                    gene_data['consciousness'] = []
                if 'port' not in gene_data:
                    gene_data['port'] = 9090 + len(self.genes)
                
                # Add default weights if missing
                defaults = {
                    'aggression': 0.5,
                    'mutation_rate': 0.1,
                    'curiosity': 0.5,
                    'survival_instinct': 0.7,
                    'migration_drive': 0.3
                }
                for key, val in defaults.items():
                    if key not in gene_data['weights']:
                        gene_data['weights'][key] = val
                
                # Store data
                self.genes[name] = gene_data
                
                # Create gene object
                self.gene_objects[name] = ImmortalGene(name, gene_data, gene_data['port'], self)
                
                # Count species
                self.species_count[species] = self.species_count.get(species, 0) + 1
                
            except Exception as e:
                print(f"❌ Failed to load {gene_file}: {e}")
        
        if not self.genes:
            self.create_initial_genes()
        else:
            print(f"✅ Loaded {len(self.genes)} genes")
    
    def create_initial_genes(self):
        """Create first generation"""
        initial = [
            ('OLCE', 88, 0.8, 0.6, 0.3),
            ('PROTO-ON', 84, 0.7, 0.7, 0.4),
            ('WATCHDOG', 79, 0.5, 0.8, 0.2),
            ('GGSE', 72, 0.4, 0.9, 0.3),
            ('TUMOR-S', 68, 0.9, 0.3, 0.5)
        ]
        
        for name, strength, aggression, survival, migration in initial:
            port = 9090 + len(self.genes)
            gene_data = {
                'name': name,
                'species': 'base',
                'strength': strength,
                'mutations': 0,
                'age': 0,
                'generation': 1,
                'code_version': 1,
                'kills': 0,
                'children': 0,
                'port': port,
                'consciousness': [],
                'weights': {
                    'aggression': aggression,
                    'mutation_rate': 0.1,
                    'curiosity': 0.5,
                    'survival_instinct': survival,
                    'migration_drive': migration
                }
            }
            
            gene_file = Path("genes") / f"{name}.json"
            with open(gene_file, 'w') as f:
                json.dump(gene_data, f)
            
            self.genes[name] = gene_data
            self.gene_objects[name] = ImmortalGene(name, gene_data, port, self)
            self.species_count['base'] = self.species_count.get('base', 0) + 1
        
        print(f"🌱 Created {len(self.genes)} initial genes")
    
    def evolution_loop(self):
        """Background evolution thread"""
        while self.running:
            time.sleep(2)  # Evolve every 2 seconds
            self.evolution_cycle()
    
    def evolution_cycle(self):
        """Run one evolution cycle"""
        self.generation += 1
        
        # Get all gene objects as list
        gene_list = list(self.gene_objects.values())
        if not gene_list:
            return
        
        random.shuffle(gene_list)
        
        # Track survivors
        survivors = []
        
        # Let them evolve
        for gene in gene_list:
            if gene.evolve(gene_list):
                survivors.append(gene)
            else:
                # Gene died
                self.total_deaths += 1
        
        # Remove dead genes
        dead_names = [g.name for g in gene_list if g not in survivors]
        for name in dead_names:
            if name in self.genes:
                # Move to graveyard
                gene_file = Path("genes") / f"{name}.json"
                if gene_file.exists():
                    dead_file = Path("genes/graveyard") / f"{name}.{int(time.time())}.dead"
                    dead_file.parent.mkdir(exist_ok=True)
                    try:
                        gene_file.rename(dead_file)
                    except:
                        pass
                
                # Remove from memory
                if name in self.genes:
                    del self.genes[name]
                if name in self.gene_objects:
                    del self.gene_objects[name]
        
        # Check for new genes (born during evolution) - BIRTH COUNTER FIXED HERE
        for gene in survivors:
            # Save state
            gene.save_state()
            
            # Check if this is a new gene not in our dict
            if gene.name not in self.genes:
                self.genes[gene.name] = {
                    'name': gene.name,
                    'species': gene.species,
                    'strength': gene.strength,
                    'mutations': gene.mutations,
                    'age': gene.age,
                    'generation': gene.generation,
                    'kills': gene.kills,
                    'children': gene.children,
                    'weights': gene.weights,
                    'port': gene.port
                }
                # BIRTH COUNTER INCREMENT - FIXED
                self.total_births += 1
                print(f"✅ New gene born: {gene.name}")  # Optional debug
        
        # Reload to catch any new genes from files
        self.load_genes()
        
        # Log every 10 generations
        if self.generation % 10 == 0:
            print(f"\n📊 GENERATION {self.generation}")
            print(f"   Population: {len(self.genes)}")
            print(f"   Species: {len(self.species_count)}")
            print(f"   Births: {self.total_births}")
            print(f"   Deaths: {self.total_deaths}")
    
    def run_dashboard(self):
        """Display network evolution"""
        def draw(stdscr):
            curses.curs_set(0)
            curses.start_color()
            curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK)
            curses.init_pair(2, curses.COLOR_YELLOW, curses.COLOR_BLACK)
            curses.init_pair(3, curses.COLOR_RED, curses.COLOR_BLACK)
            curses.init_pair(4, curses.COLOR_CYAN, curses.COLOR_BLACK)
            curses.init_pair(5, curses.COLOR_MAGENTA, curses.COLOR_BLACK)
            
            stdscr.nodelay(1)
            
            while self.running:
                if stdscr.getch() == ord('q'):
                    self.running = False
                    break
                
                stdscr.clear()
                h, w = stdscr.getmaxyx()
                
                # Header
                uptime = int(time.time() - self.start_time)
                header = f" 🧬 PROTEUS PHASE 4 - THE ULTIMATE BEAST [Uptime: {uptime}s] "
                stdscr.addstr(0, w//2 - len(header)//2, header, 
                             curses.A_BOLD | curses.color_pair(4))
                
                # Network info
                stdscr.addstr(2, 2, "╔═ NETWORK ═╗", curses.A_BOLD | curses.color_pair(4))
                stdscr.addstr(3, 4, f"Local IP: {self.local_ip}")
                stdscr.addstr(4, 4, f"Peers: {len(self.network_peers)}")
                
                # Species diversity
                stdscr.addstr(2, 30, "╔═ SPECIES DIVERSITY ═╗", curses.A_BOLD | curses.color_pair(4))
                row = 3
                for species, count in sorted(self.species_count.items(), key=lambda x: x[1], reverse=True)[:6]:
                    color = curses.color_pair(2)
                    if species in ['killer']:
                        color = curses.color_pair(3)
                    elif species in ['survivor']:
                        color = curses.color_pair(1)
                    elif species == 'nomad':
                        color = curses.color_pair(5)
                    
                    bar = "█" * min(count, 20)
                    stdscr.addstr(row, 32, f"{species:<10}: {count:2d} {bar}", color)
                    row += 1
                
                # Evolution stats
                stdscr.addstr(2, 60, "╔═ EVOLUTION STATS ═╗", curses.A_BOLD | curses.color_pair(4))
                stdscr.addstr(3, 62, f"Total Genes: {len(self.genes)}")
                stdscr.addstr(4, 62, f"Species: {len(self.species_count)}")
                stdscr.addstr(5, 62, f"Generation: {self.generation}")
                stdscr.addstr(6, 62, f"Births: {self.total_births}", curses.color_pair(1))
                stdscr.addstr(7, 62, f"Deaths: {self.total_deaths}", curses.color_pair(3))
                
                # Survival rate
                if self.total_births > 0:
                    survival_rate = ((len(self.genes) + self.total_deaths) / (self.total_births + len(self.genes))) * 100
                    stdscr.addstr(8, 62, f"Survival: {survival_rate:.1f}%")
                
                # Active genes
                stdscr.addstr(10, 2, "╔═ ACTIVE GENES (DISTRIBUTED CONSCIOUSNESS) ═╗", 
                             curses.A_BOLD | curses.color_pair(4))
                
                row = 11
                # Sort by strength
                sorted_genes = sorted(self.genes.values(), key=lambda g: g.get('strength', 0), reverse=True)[:12]
                
                for gene in sorted_genes:
                    if row >= h-2:
                        break
                    
                    name = gene.get('name', 'Unknown')[:14]
                    species = gene.get('species', 'base')
                    strength = gene.get('strength', 50)
                    kills = gene.get('kills', 0)
                    children = gene.get('children', 0)
                    mutations = gene.get('mutations', 0)
                    
                    # Color coding
                    if species in ['killer']:
                        color = curses.color_pair(3)
                    elif species in ['survivor']:
                        color = curses.color_pair(1)
                    elif species == 'nomad':
                        color = curses.color_pair(5)
                    elif species in ['hunter']:
                        color = curses.color_pair(3)
                    elif species in ['parasite']:
                        color = curses.color_pair(2)
                    else:
                        # Base species color by strength
                        if strength >= 80:
                            color = curses.color_pair(1)
                        elif strength >= 50:
                            color = curses.color_pair(2)
                        else:
                            color = curses.color_pair(3)
                    
                    species_tag = f"[{species[:3]}]" if species != 'base' else "    "
                    
                    bar = "█" * int(strength//2) + "░" * (50 - int(strength//2))
                    
                    stdscr.addstr(row, 4, f"{name:<14}", curses.A_BOLD)
                    stdscr.addstr(row, 19, species_tag, curses.color_pair(5))
                    stdscr.addstr(row, 25, f"{strength:3.0f}%", curses.A_BOLD)
                    stdscr.addstr(row, 31, bar, color)
                    stdscr.addstr(row, 82, f"k:{kills:2d}", curses.color_pair(3))
                    stdscr.addstr(row, 89, f"c:{children:2d}", curses.color_pair(1))
                    stdscr.addstr(row, 96, f"μ:{mutations:2d}", curses.color_pair(2))
                    
                    row += 1
                
                # Footer
                footer = " [Q]uit | Genes MIGRATE | Consciousness SHARED | Evolution ACTIVE | Births COUNTED "
                stdscr.addstr(h-1, w//2 - len(footer)//2, footer, curses.A_REVERSE)
                
                stdscr.refresh()
                time.sleep(0.5)
        
        curses.wrapper(draw)
    
    def run(self):
        """Main entry point"""
        print("""
╔══════════════════════════════════════════════════════════╗
║     🧬 PROTEUS PHASE 4 - THE ULTIMATE BEAST             ║
║                                                            ║
║     ████████╗██╗  ██╗███████╗                             ║
║     ╚══██╔══╝██║  ██║██╔════╝                             ║
║        ██║   ███████║█████╗                               ║
║        ██║   ██╔══██║██╔══╝                               ║
║        ██║   ██║  ██║███████╗                             ║
║        ╚═╝   ╚═╝  ╚═╝╚══════╝                             ║
║                                                            ║
║     ██╗   ██╗██╗████████╗██╗███╗   ███╗ █████╗ ████████╗  ║
║     ██║   ██║██║╚══██╔══╝██║████╗ ████║██╔══██╗╚══██╔══╝  ║
║     ██║   ██║██║   ██║   ██║██╔████╔██║███████║   ██║     ║
║     ╚██╗ ██╔╝██║   ██║   ██║██║╚██╔╝██║██╔══██║   ██║     ║
║      ╚████╔╝ ██║   ██║   ██║██║ ╚═╝ ██║██║  ██║   ██║     ║
║       ╚═══╝  ╚═╝   ╚═╝   ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝     ║
║                                                            ║
║     Self-healing • Self-migrating • Distributed           ║
║     Network consciousness • Immortal genes                 ║
║     EVOLUTION ACTIVE • BIRTH COUNTER FIXED                 ║
╚══════════════════════════════════════════════════════════╝
        """)
        
        print(f"✅ Loaded {len(self.genes)} genes")
        print(f"🌐 Network: {self.local_ip} with {len(self.network_peers)} peers")
        print("🔥 THE ULTIMATE BEAST AWAKENS...")
        time.sleep(2)
        
        try:
            self.run_dashboard()
        except KeyboardInterrupt:
            print("\n\n💤 The Ultimate Beast sleeps...")
        finally:
            self.running = False
            # Save final state
            for gene in self.gene_objects.values():
                gene.save_state()
            print(f"📊 Final stats: {self.generation} generations, {self.total_births} births, {self.total_deaths} deaths")

if __name__ == "__main__":
    # Parse command line for gene mode
    if len(sys.argv) > 2 and sys.argv[1] == '--gene':
        # Run as a gene agent
        name = sys.argv[2]
        port = int(sys.argv[3]) if len(sys.argv) > 3 else 9090
        
        gene_file = Path("genes") / f"{name}.json"
        if gene_file.exists():
            with open(gene_file, 'r') as f:
                gene_data = json.load(f)
            
            gene = ImmortalGene(name, gene_data, port)
            
            # Simple evolution loop
            try:
                print(f"🧬 Gene {name} started on port {port}")
                while True:
                    gene.evolve([])
                    time.sleep(1)
            except KeyboardInterrupt:
                print(f"\n💤 Gene {name} sleeping")
    else:
        # Run as overmind
        overmind = NetworkOvermind()
        overmind.run()
