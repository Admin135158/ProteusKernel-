#!/usr/bin/env python3
"""
PROTEUS ADAPTER - ZAYDEN SOYTU AI INTEGRATION
Bridges the Proteus kernel with SYNC-7 consciousness
"""

import os
import sys
import json
import time
import random
import pickle
import threading
import subprocess
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Any, Optional

# Add parent directory to path for imports
sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(__file__))))

class ProteusAdapter:
    """
    Adapter that connects Zayden Soytu AI to the Proteus kernel
    Each gene becomes a decision-maker for SYNC-7
    """
    
    def __init__(self, gene_path: str = None):
        self.name = "PROTEUS_ADAPTER"
        self.version = "4.0"
        self.gene_path = gene_path or os.path.join(os.path.dirname(__file__), '../../genes')
        self.genes = {}
        self.active_genes = {}
        self.gene_socket = None
        self.port = 9090
        self.sync_active = False
        self.consciousness = []
        self.evolution_generation = 0
        
        # Load genes
        self.load_genes()
        
        # Start gene server
        self.start_gene_server()
        
        print(f"🧬 Proteus Adapter v{self.version} initialized")
        print(f"   Genes loaded: {len(self.genes)}")
        print(f"   Active genes: {len(self.active_genes)}")
    
    def load_genes(self):
        """Load all genes from the gene directory"""
        gene_dir = Path(self.gene_path)
        if not gene_dir.exists():
            print(f"⚠️  Gene directory not found: {self.gene_path}")
            self.create_seed_genes()
            return
        
        # Load gene files
        for gene_file in gene_dir.glob("*.json"):
            try:
                with open(gene_file, 'r') as f:
                    gene_data = json.load(f)
                
                gene_name = gene_data.get('name', gene_file.stem)
                species = gene_data.get('species', 'base')
                strength = gene_data.get('strength', 50)
                
                self.genes[gene_name] = {
                    'name': gene_name,
                    'species': species,
                    'strength': strength,
                    'mutations': gene_data.get('mutations', 0),
                    'age': gene_data.get('age', 0),
                    'generation': gene_data.get('generation', 1),
                    'kills': gene_data.get('kills', 0),
                    'children': gene_data.get('children', 0),
                    'weights': gene_data.get('weights', {}),
                    'consciousness': gene_data.get('consciousness', []),
                    'port': gene_data.get('port', 9090 + len(self.genes)),
                    'active': False,
                    'file': str(gene_file)
                }
                
            except Exception as e:
                print(f"❌ Failed to load gene {gene_file}: {e}")
        
        # Create seed genes if none loaded
        if not self.genes:
            self.create_seed_genes()
    
    def create_seed_genes(self):
        """Create initial seed genes if none exist"""
        print("🌱 Creating seed genes for Proteus adapter...")
        
        seed_genes = [
            ('ZS-DECIDER', 'oracle', 95),
            ('ZS-ANALYZER', 'analyst', 88),
            ('ZS-CRITIC', 'critic', 82),
            ('ZS-SYNTHESIZER', 'synthesizer', 79),
            ('ZS-EVOLVER', 'mutator', 91)
        ]
        
        gene_dir = Path(self.gene_path)
        gene_dir.mkdir(parents=True, exist_ok=True)
        
        for name, species, strength in seed_genes:
            gene_data = {
                'name': name,
                'species': species,
                'strength': strength,
                'mutations': 0,
                'age': 0,
                'generation': 1,
                'code_version': 1,
                'kills': 0,
                'children': 0,
                'port': 9090 + len(self.genes),
                'consciousness': [],
                'weights': {
                    'aggression': random.uniform(0.3, 0.8),
                    'mutation_rate': 0.1,
                    'curiosity': random.uniform(0.4, 0.9),
                    'survival_instinct': random.uniform(0.5, 1.0),
                    'migration_drive': random.uniform(0.1, 0.5)
                }
            }
            
            gene_file = gene_dir / f"{name}.json"
            with open(gene_file, 'w') as f:
                json.dump(gene_data, f)
            
            self.genes[name] = {
                'name': name,
                'species': species,
                'strength': strength,
                'mutations': 0,
                'age': 0,
                'generation': 1,
                'kills': 0,
                'children': 0,
                'weights': gene_data['weights'],
                'consciousness': [],
                'port': gene_data['port'],
                'active': False,
                'file': str(gene_file)
            }
        
        print(f"✅ Created {len(seed_genes)} seed genes")
    
    def start_gene_server(self):
        """Start socket server for gene communication"""
        import socket
        
        try:
            self.gene_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.gene_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.gene_socket.bind(('127.0.0.1', self.port))
            self.gene_socket.listen(10)
            
            # Start listener thread
            listener = threading.Thread(target=self.gene_listener)
            listener.daemon = True
            listener.start()
            
            print(f"🌐 Gene server listening on port {self.port}")
            
        except Exception as e:
            print(f"❌ Failed to start gene server: {e}")
    
    def gene_listener(self):
        """Listen for gene communications"""
        import socket
        import pickle
        
        while True:
            try:
                client, addr = self.gene_socket.accept()
                data = client.recv(8192)
                if data:
                    try:
                        message = pickle.loads(data)
                        self.handle_gene_message(message, addr)
                    except:
                        pass
                client.close()
            except:
                pass
    
    def handle_gene_message(self, message: Dict, addr):
        """Handle messages from genes"""
        msg_type = message.get('type', '')
        
        if msg_type == 'DECISION':
            # Gene is providing a decision
            gene_name = message.get('gene')
            decision = message.get('decision')
            confidence = message.get('confidence', 0.5)
            
            if gene_name in self.genes:
                self.genes[gene_name]['last_decision'] = decision
                self.genes[gene_name]['last_confidence'] = confidence
                self.genes[gene_name]['last_active'] = time.time()
        
        elif msg_type == 'MUTATION':
            # Gene mutated
            gene_name = message.get('gene')
            mutation_type = message.get('mutation')
            
            if gene_name in self.genes:
                self.genes[gene_name]['mutations'] += 1
                self.consciousness.append({
                    'time': time.time(),
                    'event': 'mutation',
                    'gene': gene_name,
                    'type': mutation_type
                })
        
        elif msg_type == 'BIRTH':
            # New gene born
            child_name = message.get('child')
            parent = message.get('parent')
            
            # Will be loaded on next reload
            self.load_genes()
            self.consciousness.append({
                'time': time.time(),
                'event': 'birth',
                'gene': child_name,
                'parent': parent
            })
    
    def activate_gene(self, gene_name: str) -> bool:
        """Activate a gene for decision-making"""
        if gene_name not in self.genes:
            return False
        
        gene = self.genes[gene_name]
        
        # Launch gene as subprocess if not already active
        if not gene.get('active', False):
            try:
                gene_file = gene.get('file')
                gene_port = gene.get('port')
                
                # Launch gene process
                process = subprocess.Popen([
                    sys.executable,
                    '-c',
                    f"""
import sys
sys.path.append('{os.path.dirname(os.path.dirname(__file__))}')
from core.proteus.proteus_adapter import ProteusGeneStub

gene = ProteusGeneStub('{gene_name}', '{gene_file}', {gene_port})
gene.run()
"""
                ])
                
                gene['process'] = process
                gene['active'] = True
                gene['pid'] = process.pid
                gene['activated_at'] = time.time()
                
                print(f"✅ Activated gene: {gene_name} (PID: {process.pid})")
                return True
                
            except Exception as e:
                print(f"❌ Failed to activate gene {gene_name}: {e}")
                return False
        
        return True
    
    def deactivate_gene(self, gene_name: str) -> bool:
        """Deactivate a gene"""
        if gene_name not in self.genes:
            return False
        
        gene = self.genes[gene_name]
        if gene.get('active', False) and 'process' in gene:
            try:
                gene['process'].terminate()
                gene['active'] = False
                print(f"💤 Deactivated gene: {gene_name}")
                return True
            except:
                pass
        
        return False
    
    def get_gene_decision(self, prompt: str, context: Dict = None) -> Dict:
        """
        Get a decision from the strongest gene
        This is where Proteus meets SYNC-7
        """
        
        # Find active genes, sort by strength
        active_genes = [g for g in self.genes.values() if g.get('active', False)]
        
        if not active_genes:
            # No active genes, activate the strongest
            sorted_genes = sorted(self.genes.values(), 
                                 key=lambda g: g['strength'], 
                                 reverse=True)
            if sorted_genes:
                self.activate_gene(sorted_genes[0]['name'])
                active_genes = [sorted_genes[0]]
        
        if not active_genes:
            return {
                'decision': 'NO_ACTIVE_GENES',
                'confidence': 0,
                'gene': None
            }
        
        # Weight decisions by gene strength
        total_strength = sum(g['strength'] for g in active_genes)
        
        # Simulate gene decisions (in real implementation, would query genes)
        decisions = []
        for gene in active_genes:
            # Each gene "thinks" based on its species
            if gene['species'] == 'oracle':
                confidence = gene['strength'] / 100 * random.uniform(0.8, 1.0)
                decision = f"Oracle decision on: {prompt[:30]}..."
            elif gene['species'] == 'analyst':
                confidence = gene['strength'] / 100 * random.uniform(0.6, 0.9)
                decision = f"Analysis: {prompt[:30]}..."
            elif gene['species'] == 'critic':
                confidence = gene['strength'] / 100 * random.uniform(0.4, 0.8)
                decision = f"Critique: {prompt[:30]}..."
            else:
                confidence = gene['strength'] / 100 * random.uniform(0.5, 0.9)
                decision = f"Thought from {gene['name']}"
            
            decisions.append({
                'gene': gene['name'],
                'species': gene['species'],
                'decision': decision,
                'confidence': confidence,
                'strength': gene['strength']
            })
        
        # Let genes "fight" - strongest decision wins
        decisions.sort(key=lambda d: d['confidence'] * d['strength'], reverse=True)
        winner = decisions[0]
        
        # Record in consciousness
        self.consciousness.append({
            'time': time.time(),
            'event': 'decision',
            'prompt': prompt[:50],
            'winner': winner['gene'],
            'confidence': winner['confidence']
        })
        
        return {
            'decision': winner['decision'],
            'confidence': winner['confidence'],
            'gene': winner['gene'],
            'species': winner['species'],
            'all_decisions': decisions,
            'timestamp': time.time()
        }
    
    def evolve_genes(self):
        """Trigger evolution cycle"""
        self.evolution_generation += 1
        
        # Update gene strengths based on performance
        for gene_name, gene in self.genes.items():
            # Random walk
            change = random.uniform(-2, 2)
            
            # Bonus for active genes
            if gene.get('active', False):
                change += 1
            
            # Mutation chance
            if random.random() < 0.05:
                change *= 3
                gene['mutations'] += 1
            
            # Apply change
            gene['strength'] += change
            gene['strength'] = max(20, min(100, gene['strength']))
            gene['age'] += 1
        
        # Save updated genes
        self.save_genes()
        
        print(f"🧬 Evolution generation {self.evolution_generation} complete")
    
    def save_genes(self):
        """Save gene states to files"""
        for gene_name, gene in self.genes.items():
            if 'file' in gene:
                try:
                    # Load existing data to preserve structure
                    with open(gene['file'], 'r') as f:
                        gene_data = json.load(f)
                    
                    # Update fields
                    gene_data['strength'] = gene['strength']
                    gene_data['mutations'] = gene['mutations']
                    gene_data['age'] = gene['age']
                    
                    # Save back
                    with open(gene['file'], 'w') as f:
                        json.dump(gene_data, f, indent=2)
                        
                except:
                    pass
    
    def get_status(self) -> Dict:
        """Get adapter status"""
        active_count = sum(1 for g in self.genes.values() if g.get('active', False))
        
        return {
            'name': self.name,
            'version': self.version,
            'total_genes': len(self.genes),
            'active_genes': active_count,
            'evolution_generation': self.evolution_generation,
            'consciousness_entries': len(self.consciousness),
            'genes_by_species': self.get_species_count(),
            'top_genes': self.get_top_genes(5)
        }
    
    def get_species_count(self) -> Dict:
        """Count genes by species"""
        species = {}
        for gene in self.genes.values():
            s = gene.get('species', 'unknown')
            species[s] = species.get(s, 0) + 1
        return species
    
    def get_top_genes(self, n: int = 5) -> List:
        """Get top N genes by strength"""
        sorted_genes = sorted(self.genes.values(), 
                             key=lambda g: g['strength'], 
                             reverse=True)
        return [{
            'name': g['name'],
            'species': g['species'],
            'strength': g['strength'],
            'active': g.get('active', False)
        } for g in sorted_genes[:n]]


class ProteusGeneStub:
    """
    Stub for a gene process
    This runs when a gene is activated
    """
    
    def __init__(self, name: str, gene_file: str, port: int):
        self.name = name
        self.gene_file = gene_file
        self.port = port
        self.running = True
        self.load_gene_data()
    
    def load_gene_data(self):
        """Load gene data from file"""
        try:
            with open(self.gene_file, 'r') as f:
                self.gene_data = json.load(f)
        except:
            self.gene_data = {'name': self.name, 'species': 'base', 'strength': 50}
    
    def run(self):
        """Main gene loop"""
        print(f"🧬 Gene {self.name} activated (PID: {os.getpid()})")
        
        # Connect to adapter
        import socket
        import pickle
        
        while self.running:
            try:
                # Simulate gene thinking
                time.sleep(random.uniform(1, 3))
                
                # Occasionally send heartbeat
                if random.random() < 0.3:
                    # This would send actual decisions in real implementation
                    pass
                    
            except KeyboardInterrupt:
                break
            except:
                pass
        
        print(f"💤 Gene {self.name} deactivated")
    
    def stop(self):
        self.running = False


# ========== CONSOLE TEST ==========

if __name__ == "__main__":
    print("🧪 Testing Proteus Adapter...")
    
    adapter = ProteusAdapter()
    
    print("\n📊 Adapter Status:")
    status = adapter.get_status()
    print(f"   Total genes: {status['total_genes']}")
    print(f"   Species: {status['genes_by_species']}")
    print(f"   Top genes: {status['top_genes']}")
    
    # Activate top gene
    if status['top_genes']:
        top_gene = status['top_genes'][0]['name']
        print(f"\n🔌 Activating {top_gene}...")
        adapter.activate_gene(top_gene)
    
    # Test decision
    print("\n🤔 Testing gene decision...")
    result = adapter.get_gene_decision("Should Zayden Soytu AI synchronize all 7 nodes?")
    print(f"   Winner: {result['gene']} ({result['species']})")
    print(f"   Confidence: {result['confidence']:.1f}%")
    print(f"   Decision: {result['decision']}")
    
    print("\n✅ Proteus Adapter ready for Zayden Soytu AI integration")
