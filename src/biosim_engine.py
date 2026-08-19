#!/usr/bin/env python3
import json
import random
import time
import math
import os
from collections import defaultdict

class BiosimEngine:
    def __init__(self, state_file="biosim_state.json"):
        self.state_file = state_file
        self.generation = 0
        self.population = []
        self.max_population = 100
        self.graveyard = []
        self.event_log = []
        self.is_running = False
        self.total_births = 0
        self.total_degradations = 0
        self.total_replication_events = 0
        self.total_mutations = 0
        self.architect_power = 10
        self.load_state()
    
    def load_state(self):
        if os.path.exists(self.state_file):
            with open(self.state_file, 'r') as f:
                data = json.load(f)
                self.generation = data.get('generation', 0)
                self.population = data.get('population', [])
                self.graveyard = data.get('graveyard', [])
                self.event_log = data.get('event_log', [])
                self.total_births = data.get('total_births', 0)
                self.total_degradations = data.get('total_degradations', 0)
                self.total_replication_events = data.get('total_replication_events', 0)
                self.total_mutations = data.get('total_mutations', 0)
                self.architect_power = data.get('architect_power', 10)
        else:
            self.seed_initial_population()
    
    def seed_initial_population(self):
        # Create 10 random genes
        for i in range(10):
            gene = self.random_gene()
            self.population.append(gene)
        self.generation = 0
        self.event_log.append({"type": "genesis", "timestamp": time.time()})
    
    def random_gene(self):
        bases = ['A', 'C', 'G', 'T']
        sequence = ''.join(random.choices(bases, k=64))
        return {
            'id': f"gene_{int(time.time()*1000)}_{random.randint(0,1000)}",
            'sequence': sequence,
            'fitness': random.uniform(0.1, 0.8),
            'replication_fidelity': random.uniform(0.85, 0.99),
            'mutations': 0,
            'generation': self.generation,
            'species': 'proteus',
            'name': f"Seq-{random.randint(100,999)}"
        }
    
    def compute_fitness(self, gene):
        # Simple fitness: GC content + entropy
        seq = gene['sequence']
        gc = (seq.count('G') + seq.count('C')) / len(seq)
        entropy = -sum(seq.count(b)/len(seq) * math.log2(seq.count(b)/len(seq)) if seq.count(b) else 0 for b in set(seq))
        fitness = 0.5 * gc + 0.3 * entropy/6 + 0.2 * gene['replication_fidelity']
        return min(1.0, fitness)
    
    def mutate_gene(self, gene):
        new_seq = list(gene['sequence'])
        # One random mutation per gene
        pos = random.randint(0, len(new_seq)-1)
        new_base = random.choice(['A','C','G','T'])
        new_seq[pos] = new_base
        gene['sequence'] = ''.join(new_seq)
        gene['mutations'] += 1
        gene['replication_fidelity'] *= random.uniform(0.98, 1.02)
        gene['replication_fidelity'] = min(1.0, max(0.7, gene['replication_fidelity']))
        return gene
    
    def replicate(self):
        # Select top 50% to replicate, with mutation
        sorted_pop = sorted(self.population, key=lambda g: g['fitness'], reverse=True)
        survivors = sorted_pop[:int(len(sorted_pop)*0.5)]
        # Generate offspring
        new_population = survivors.copy()
        for gene in survivors:
            child = gene.copy()
            child['id'] = f"gene_{int(time.time()*1000)}_{random.randint(0,1000)}"
            child['generation'] = self.generation + 1
            # Mutate a random position
            self.mutate_gene(child)
            child['fitness'] = self.compute_fitness(child)
            new_population.append(child)
            self.total_replication_events += 1
            self.total_mutations += 1
            self.event_log.append({"type": "replication", "parent": gene['id'], "child": child['id'], "timestamp": time.time()})
        # Keep only top population
        if len(new_population) > self.max_population:
            new_population = sorted(new_population, key=lambda g: g['fitness'], reverse=True)[:self.max_population]
        self.population = new_population
        self.generation += 1
        self.total_births += len(self.population) - len(survivors)
    
    def degrade(self):
        # Remove bottom 10% if too many
        if len(self.population) > 20:
            sorted_pop = sorted(self.population, key=lambda g: g['fitness'])
            to_remove = int(len(sorted_pop)*0.1)
            removed = sorted_pop[:to_remove]
            self.population = sorted_pop[to_remove:]
            self.graveyard.extend(removed)
            self.total_degradations += len(removed)
            self.event_log.append({"type": "degradation", "count": len(removed), "timestamp": time.time()})
    
    def step(self):
        if not self.is_running:
            return
        # Compute fitness for all
        for gene in self.population:
            gene['fitness'] = self.compute_fitness(gene)
        self.replicate()
        self.degrade()
        self.event_log = self.event_log[-200:]  # keep log manageable
        self.save_state()
    
    def start(self):
        self.is_running = True
        self.event_log.append({"type": "start", "timestamp": time.time()})
    
    def pause(self):
        self.is_running = False
        self.event_log.append({"type": "pause", "timestamp": time.time()})
    
    def get_status(self):
        if not self.population:
            return {"generation": self.generation, "population": 0, "is_running": self.is_running}
        top = max(self.population, key=lambda g: g['fitness'])
        avg_fitness = sum(g['fitness'] for g in self.population) / len(self.population)
        avg_fidelity = sum(g['replication_fidelity'] for g in self.population) / len(self.population)
        return {
            "generation": self.generation,
            "population": len(self.population),
            "is_running": self.is_running,
            "top_fitness": top['fitness'] if self.population else 0,
            "mean_fitness": avg_fitness,
            "mean_fidelity": avg_fidelity,
            "total_births": self.total_births,
            "total_degradations": self.total_degradations,
            "total_replications": self.total_replication_events,
            "total_mutations": self.total_mutations,
            "architect_power": self.architect_power
        }
    
    def get_top_sequences(self, limit=8):
        sorted_pop = sorted(self.population, key=lambda g: g['fitness'], reverse=True)
        return sorted_pop[:limit]
    
    def get_events(self, limit=50):
        return self.event_log[-limit:]
    
    def save_state(self):
        data = {
            "generation": self.generation,
            "population": self.population,
            "graveyard": self.graveyard,
            "event_log": self.event_log,
            "total_births": self.total_births,
            "total_degradations": self.total_degradations,
            "total_replication_events": self.total_replication_events,
            "total_mutations": self.total_mutations,
            "architect_power": self.architect_power
        }
        with open(self.state_file, 'w') as f:
            json.dump(data, f, indent=2)
    
    def architect_purge(self):
        # Remove weakest 20%
        if len(self.population) < 5:
            return
        sorted_pop = sorted(self.population, key=lambda g: g['fitness'])
        to_remove = int(len(sorted_pop)*0.2)
        removed = sorted_pop[:to_remove]
        self.population = sorted_pop[to_remove:]
        self.graveyard.extend(removed)
        self.architect_power = max(0, self.architect_power - 1)
        self.event_log.append({"type": "architect_purge", "count": len(removed), "timestamp": time.time()})
        self.save_state()
    
    def architect_synthesize(self):
        # Create a new gene from two top parents
        if len(self.population) < 2:
            return
        top2 = sorted(self.population, key=lambda g: g['fitness'], reverse=True)[:2]
        parent1, parent2 = top2
        # Crossover
        seq1, seq2 = parent1['sequence'], parent2['sequence']
        cut = random.randint(0, len(seq1)-1)
        new_seq = seq1[:cut] + seq2[cut:]
        new_gene = {
            'id': f"gene_{int(time.time()*1000)}_{random.randint(0,1000)}",
            'sequence': new_seq,
            'fitness': 0.0,
            'replication_fidelity': (parent1['replication_fidelity'] + parent2['replication_fidelity'])/2,
            'mutations': 0,
            'generation': self.generation,
            'species': 'synthetic',
            'name': f"Synth-{random.randint(100,999)}"
        }
        new_gene['fitness'] = self.compute_fitness(new_gene)
        self.population.append(new_gene)
        self.architect_power = max(0, self.architect_power - 1)
        self.event_log.append({"type": "architect_synthesize", "timestamp": time.time()})
        self.save_state()
    
    def architect_elevate(self):
        # Boost top gene's fitness
        if not self.population:
            return
        top = max(self.population, key=lambda g: g['fitness'])
        top['fitness'] = min(1.0, top['fitness'] * 1.1)
        self.architect_power = max(0, self.architect_power - 1)
        self.event_log.append({"type": "architect_elevate", "timestamp": time.time()})
        self.save_state()
    
    def architect_meditate(self):
        self.architect_power = min(20, self.architect_power + 2)
        self.event_log.append({"type": "architect_meditate", "timestamp": time.time()})
        self.save_state()
