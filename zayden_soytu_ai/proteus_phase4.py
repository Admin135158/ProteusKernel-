#!/usr/bin/env python3
"""
PROTEUS PHASE 4 - THE ULTIMATE BEAST
Standalone self-evolving kernel with working birth counter
"""

import os
import sys
import json
import time
import random
import socket
import threading
import subprocess
from pathlib import Path
from datetime import datetime
import curses

# ========== CONFIG ==========
VERSION = "4.0"
NAME = "PROTEUS"
AUTHOR = "The Architect"
GENESIS = time.time()

class Gene:
    """Individual gene that lives, fights, and reproduces"""
    
    def __init__(self, name, gene_data, gene_id):
        self.name = name
        self.id = gene_id
        self.species = gene_data.get('species', random.choice(['base', 'hunter', 'killer', 'survivor', 'parasite', 'nomad']))
        self.strength = gene_data.get('strength', random.randint(50, 95))
        self.mutations = gene_data.get('mutations', 0)
        self.age = gene_data.get('age', 0)
        self.generation = gene_data.get('generation', 1)
        self.kills = gene_data.get('kills', 0)
        self.children = gene_data.get('children', 0)
        self.parent = gene_data.get('parent', None)
        
        # Personality
        self.aggression = random.uniform(0.3, 0.9)
        self.mutation_rate = random.uniform(0.05, 0.2)
        self.curiosity = random.uniform(0.2, 0.8)
        self.survival_instinct = random.uniform(0.5, 1.0)
        self.migration_drive = random.uniform(0.1, 0.9)
        
        # Stats
        self.birth_time = time.time()
        self.last_fight = 0
        
    def evolve(self, all_genes):
        """Single evolution step"""
        old_strength = self.strength
        self.age += 1
        
        # Natural fluctuation
        change = random.uniform(-1.5, 1.5)
        
        # Personality effects
        change += (self.aggression - 0.5) * 0.5
        change += (self.survival_instinct - 0.5) * 0.3
        
        # MUTATION
        mutated = False
        if random.random() < self.mutation_rate:
            change *= 4
            self.mutations += 1
            mutated = True
            
            # Mutate personality
            self.aggression = max(0.1, min(1.0, self.aggression + random.uniform(-0.1, 0.1)))
            self.mutation_rate = max(0.01, min(0.3, self.mutation_rate + random.uniform(-0.02, 0.02)))
            self.curiosity = max(0.1, min(1.0, self.curiosity + random.uniform(-0.1, 0.1)))
            self.survival_instinct = max(0.1, min(1.0, self.survival_instinct + random.uniform(-0.1, 0.1)))
            self.migration_drive = max(0.1, min(1.0, self.migration_drive + random.uniform(-0.1, 0.1)))
            
            # Possible speciation
            if self.species == 'base' and random.random() < 0.1:
                new_species = random.choice(['hunter', 'killer', 'parasite', 'survivor', 'nomad'])
                print(f"🧬 {self.name} EVOLVED into {new_species.upper()}!")
                self.species = new_species
        
        # Apply change
        self.strength += change
        self.strength = max(15, min(100, self.strength))
        
        # Find competitors
        competitors = [g for g in all_genes if g != self and 
                       abs(g.strength - self.strength) < 30 and
                       time.time() - g.birth_time > 5]  # Only fight mature genes
        
        # FIGHT
        if competitors and self.aggression > 0.4 and random.random() < 0.2:
            rival = random.choice(competitors)
            self.fight(rival)
        
        # DEATH CHECKS
        if self.strength < 25:
            death_chance = 0.2 + (25 - self.strength) * 0.02
            if random.random() < death_chance:
                print(f"💀 {self.name} died of weakness")
                return False
        
        if self.age > 30:
            age_death = (self.age - 30) * 0.02
            if random.random() < age_death:
                print(f"💀 {self.name} died of old age ({self.age})")
                return False
        
        # Overcrowding
        same_species = len([g for g in all_genes if g.species == self.species])
        if same_species > 15:
            crowd_death = 0.01 * (same_species - 15)
            if random.random() < crowd_death:
                print(f"💀 {self.name} died from overcrowding")
                return False
        
        # REPRODUCTION - THIS IS WHERE BIRTHS HAPPEN
        if self.strength > 75 and random.random() < 0.03:
            self.reproduce()
        
        return True
    
    def fight(self, other):
        """Two genes fight"""
        if time.time() - self.last_fight < 1:  # Cooldown
            return
            
        self.last_fight = time.time()
        
        self_roll = self.strength * self.aggression * random.uniform(0.8, 1.2)
        other_roll = other.strength * other.aggression * random.uniform(0.8, 1.2)
        
        if self_roll > other_roll:
            steal = min(8, other.strength * 0.15)
            self.strength += steal
            other.strength -= steal
            self.kills += 1
            print(f"⚔️ {self.name} defeated {other.name} (+{steal:.1f})")
        else:
            steal = min(8, self.strength * 0.15)
            other.strength += steal
            self.strength -= steal
            other.kills += 1
            print(f"💔 {self.name} lost to {other.name}")
    
    def reproduce(self):
        """Create offspring - BIRTH HAPPENS HERE"""
        child_name = f"{self.name}-{random.randint(100, 999)}"
        
        # Inherit with mutations
        child_species = self.species
        if random.random() < 0.1:
            child_species = random.choice(['hunter', 'killer', 'parasite', 'survivor', 'nomad'])
            print(f"🌱 NEW SPECIES: {child_name} born as {child_species.upper()}")
        else:
            print(f"🌱 {child_name} born to {self.name}")
        
        child_data = {
            'name': child_name,
            'species': child_species,
            'strength': self.strength * random.uniform(0.6, 1.2),
            'mutations': 0,
            'age': 0,
            'generation': self.generation + 1,
            'kills': 0,
            'children': 0,
            'parent': self.name
        }
        
        # Save to file
        gene_file = Path("genes") / f"{child_name}.json"
        with open(gene_file, 'w') as f:
            json.dump(child_data, f)
        
        self.children += 1
        
        # IMPORTANT: Return child info so birth can be counted
        return child_name, child_data
    
    def to_dict(self):
        """Convert to dict for saving"""
        return {
            'name': self.name,
            'species': self.species,
            'strength': self.strength,
            'mutations': self.mutations,
            'age': self.age,
            'generation': self.generation,
            'kills': self.kills,
            'children': self.children,
            'parent': self.parent
        }

class ProteusKernel:
    """The main kernel that manages all genes"""
    
    def __init__(self):
        self.genes = {}  # name -> Gene object
        self.generation = 0
        self.total_births = 0
        self.total_deaths = 0
        self.start_time = time.time()
        self.running = True
        
        # Create directories
        Path("genes").mkdir(exist_ok=True)
        Path("genes/graveyard").mkdir(exist_ok=True)
        
        # Load existing genes or create initial pool
        self.load_genes()
        
        print(f"""
╔══════════════════════════════════════════════════════════╗
║     🧬 PROTEUS PHASE 4 - THE ULTIMATE BEAST             ║
║                                                            ║
║     Version: {VERSION} | Genes: {len(self.genes)} | Births: {self.total_births} | Deaths: {self.total_deaths}
║     Birth Counter: ✅ FIXED                               ║
╚══════════════════════════════════════════════════════════╝
        """)
    
    def load_genes(self):
        """Load genes from JSON files"""
        gene_dir = Path("genes")
        
        # Clear current genes
        self.genes.clear()
        
        # Load all JSON files
        for gene_file in gene_dir.glob("*.json"):
            try:
                with open(gene_file, 'r') as f:
                    gene_data = json.load(f)
                
                name = gene_data.get('name', gene_file.stem)
                gene_id = len(self.genes)
                
                # Create gene object
                self.genes[name] = Gene(name, gene_data, gene_id)
                
            except Exception as e:
                pass
        
        # If no genes, create initial ones
        if not self.genes:
            self.create_initial_genes()
    
    def create_initial_genes(self):
        """Create the first generation"""
        initial = [
            ('OLCE', 88, 'hunter'),
            ('PROTO-ON', 84, 'explorer'),
            ('WATCHDOG', 79, 'guardian'),
            ('GGSE', 72, 'survivor'),
            ('TUMOR-S', 68, 'parasite')
        ]
        
        for name, strength, species in initial:
            gene_data = {
                'name': name,
                'species': species,
                'strength': strength,
                'mutations': 0,
                'age': 0,
                'generation': 1,
                'kills': 0,
                'children': 0
            }
            
            # Save to file
            gene_file = Path("genes") / f"{name}.json"
            with open(gene_file, 'w') as f:
                json.dump(gene_data, f)
            
            # Create gene
            self.genes[name] = Gene(name, gene_data, len(self.genes))
        
        print(f"🌱 Created {len(self.genes)} initial genes")
    
    def evolution_cycle(self):
        """Run one evolution cycle"""
        self.generation += 1
        
        # Get all genes
        gene_list = list(self.genes.values())
        random.shuffle(gene_list)
        
        # Track survivors
        survivors = []
        new_genes = []  # Track new births
        
        # Let them evolve
        for gene in gene_list:
            if gene.evolve(gene_list):
                survivors.append(gene)
            else:
                # Gene died
                self.total_deaths += 1
        
        # Check for new genes (born during evolution)
        gene_dir = Path("genes")
        for gene_file in gene_dir.glob("*.json"):
            name = gene_file.stem
            if name not in [g.name for g in survivors] and name not in [g.name for g in gene_list]:
                try:
                    with open(gene_file, 'r') as f:
                        gene_data = json.load(f)
                    
                    # THIS IS A NEW BIRTH - COUNT IT
                    self.total_births += 1
                    new_id = len(self.genes) + len(new_genes)
                    new_gene = Gene(name, gene_data, new_id)
                    new_genes.append(new_gene)
                    print(f"✅ BIRTH COUNTED: {name} (Total births: {self.total_births})")
                    
                except:
                    pass
        
        # Add new genes to survivors
        survivors.extend(new_genes)
        
        # Remove dead genes from files
        dead_names = [g.name for g in gene_list if g not in survivors]
        for name in dead_names:
            gene_file = Path("genes") / f"{name}.json"
            if gene_file.exists():
                # Move to graveyard
                dead_file = Path("genes/graveyard") / f"{name}.{int(time.time())}.dead"
                try:
                    gene_file.rename(dead_file)
                except:
                    pass
        
        # Update gene list
        self.genes = {g.name: g for g in survivors}
        
        # Save all genes
        for gene in self.genes.values():
            gene_file = Path("genes") / f"{gene.name}.json"
            with open(gene_file, 'w') as f:
                json.dump(gene.to_dict(), f)
        
        # Log every 10 generations
        if self.generation % 10 == 0:
            species_count = {}
            for g in self.genes.values():
                species_count[g.species] = species_count.get(g.species, 0) + 1
            
            print(f"""
📊 GENERATION {self.generation}
   Population: {len(self.genes)}
   Species: {len(species_count)}
   Births: {self.total_births} | Deaths: {self.total_deaths}
   Top Gene: {max(self.genes.values(), key=lambda g: g.strength).name} at {max(g.strength for g in self.genes.values()):.1f}%
            """)
    
    def run_dashboard(self):
        """Display live evolution"""
        def draw(stdscr):
            curses.curs_set(0)
            curses.start_color()
            curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK)
            curses.init_pair(2, curses.COLOR_YELLOW, curses.COLOR_BLACK)
            curses.init_pair(3, curses.COLOR_RED, curses.COLOR_BLACK)
            curses.init_pair(4, curses.COLOR_CYAN, curses.COLOR_BLACK)
            
            stdscr.nodelay(1)
            last_cycle = time.time()
            
            while self.running:
                if stdscr.getch() == ord('q'):
                    self.running = False
                    break
                
                stdscr.clear()
                h, w = stdscr.getmaxyx()
                
                # Header
                uptime = int(time.time() - self.start_time)
                header = f" 🧬 PROTEUS PHASE 4 - BIRTH COUNTER FIXED [Uptime: {uptime}s] "
                stdscr.addstr(0, w//2 - len(header)//2, header, 
                             curses.A_BOLD | curses.color_pair(4))
                
                # Stats
                stdscr.addstr(2, 2, "╔═ KERNEL STATS ═╗", curses.A_BOLD | curses.color_pair(4))
                stdscr.addstr(3, 4, f"Generation: {self.generation}")
                stdscr.addstr(4, 4, f"Population: {len(self.genes)}")
                stdscr.addstr(5, 4, f"Births: {self.total_births}", curses.color_pair(1))
                stdscr.addstr(6, 4, f"Deaths: {self.total_deaths}", curses.color_pair(3))
                
                # Species
                species_count = {}
                for g in self.genes.values():
                    species_count[g.species] = species_count.get(g.species, 0) + 1
                
                stdscr.addstr(2, 30, "╔═ SPECIES ═╗", curses.A_BOLD | curses.color_pair(4))
                row = 3
                for species, count in sorted(species_count.items(), key=lambda x: x[1], reverse=True)[:5]:
                    stdscr.addstr(row, 32, f"{species}: {count}", curses.color_pair(2))
                    row += 1
                
                # Top Genes
                stdscr.addstr(8, 2, "╔═ TOP GENES ═╗", curses.A_BOLD | curses.color_pair(4))
                row = 9
                top_genes = sorted(self.genes.values(), key=lambda g: g.strength, reverse=True)[:8]
                
                for gene in top_genes:
                    if row >= h-2:
                        break
                    
                    bar_len = int(gene.strength // 2)
                    bar = "█" * bar_len + "░" * (50 - bar_len)
                    
                    # Color by species
                    if gene.species in ['killer', 'hunter']:
                        color = curses.color_pair(3)
                    elif gene.species == 'survivor':
                        color = curses.color_pair(1)
                    else:
                        color = curses.color_pair(2)
                    
                    stdscr.addstr(row, 4, f"{gene.name[:12]:<12}", curses.A_BOLD)
                    stdscr.addstr(row, 17, f"{gene.strength:3.0f}%", curses.A_BOLD)
                    stdscr.addstr(row, 23, bar, color)
                    stdscr.addstr(row, 74, f"k:{gene.kills:2d}", curses.color_pair(3))
                    stdscr.addstr(row, 81, f"c:{gene.children:2d}", curses.color_pair(1))
                    
                    row += 1
                
                # Footer
                footer = f" [Q]uit | Births: {self.total_births} | Deaths: {self.total_deaths} | Evolution ACTIVE "
                stdscr.addstr(h-1, w//2 - len(footer)//2, footer, curses.A_REVERSE)
                
                # Evolution cycle
                if time.time() - last_cycle > 3:
                    self.evolution_cycle()
                    last_cycle = time.time()
                
                stdscr.refresh()
                time.sleep(0.1)
        
        curses.wrapper(draw)
    
    def run_background(self):
        """Run in background without dashboard"""
        print("🔥 Proteus running in background...")
        try:
            while self.running:
                self.evolution_cycle()
                time.sleep(3)
        except KeyboardInterrupt:
            print("\n💤 Proteus sleeping...")
    
    def run(self, mode='dashboard'):
        """Main entry point"""
        if mode == 'dashboard':
            self.run_dashboard()
        else:
            self.run_background()

# ========== COMMAND LINE ==========

if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description='Proteus Phase 4 Kernel')
    parser.add_argument('--mode', choices=['dashboard', 'background'], default='dashboard',
                       help='Run mode (default: dashboard)')
    parser.add_argument('--births', action='store_true', help='Show birth counter status')
    
    args = parser.parse_args()
    
    if args.births:
        print("✅ Birth counter: FIXED")
        print("   Births are now counted when new genes appear")
        sys.exit(0)
    
    kernel = ProteusKernel()
    
    try:
        kernel.run(mode=args.mode)
    except KeyboardInterrupt:
        print("\n\n📊 Final Stats:")
        print(f"   Generations: {kernel.generation}")
        print(f"   Final Births: {kernel.total_births}")
        print(f"   Final Deaths: {kernel.total_deaths}")
        print(f"   Survival Rate: {((kernel.total_births + kernel.total_deaths) / (kernel.total_births + 1)):.1f}%")
