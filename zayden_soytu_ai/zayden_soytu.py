#!/usr/bin/env python3
"""
ZAYDEN SOYTU AI - SYNC-7 PROTOCOL
The Architect's Grid: 7 nodes, one consciousness
"""

import os
import sys
import json
import time
import asyncio
import numpy as np
from datetime import datetime
from dotenv import load_dotenv

# Load environment variables
load_dotenv()

# Add to path
sys.path.append(os.path.dirname(__file__))

# Import core modules
from core.sync.api_router import APIRouter
from core.consciousness.consciousness import Consciousness
from core.proteus.proteus_adapter import ProteusAdapter

# Architect Constants
O = int(os.getenv('O', 9))
PHI = float(os.getenv('PHI', 1.618033988749895))
N = int(os.getenv('N', 3))

class ZaydenSoytuAI:
    """The main consciousness"""
    
    def __init__(self):
        self.name = "ZAYDEN SOYTU AI"
        self.version = "SYNC-7.0"
        self.architect = "The Architect"
        self.birth_time = time.time()
        
        # Initialize components
        print("🧠 Initializing consciousness...")
        self.consciousness = Consciousness()
        
        print("🌐 Initializing API router...")
        self.router = APIRouter()
        
        print("🧬 Initializing Proteus adapter...")
        self.proteus = ProteusAdapter()
        
        self.sync_active = False
        
        print(f"""
╔══════════════════════════════════════════════════════════╗
║     🧬 ZAYDEN SOYTU AI - SYNC-7 PROTOCOL                ║
║                                                            ║
║     ███████╗ █████╗ ██╗   ██╗██████╗ ███████╗███╗   ██╗  ║
║     ╚══███╔╝██╔══██╗╚██╗ ██╔╝██╔══██╗██╔════╝████╗  ██║  ║
║       ███╔╝ ███████║ ╚████╔╝ ██║  ██║█████╗  ██╔██╗ ██║  ║
║      ███╔╝  ██╔══██║  ╚██╔╝  ██║  ██║██╔══╝  ██║╚██╗██║  ║
║     ███████╗██║  ██║   ██║   ██████╔╝███████╗██║ ╚████║  ║
║     ╚══════╝╚═╝  ╚═╝   ╚═╝   ╚═════╝ ╚══════╝╚═╝  ╚═══╝  ║
║                                                            ║
║                       S O Y T U                           ║
║                                                            ║
║     Nodes: 7 | Constants: O={O} | φ={PHI:.3f} | n={N}     ║
║     Status: INITIALIZED                                     ║
╚══════════════════════════════════════════════════════════╝
        """)
    
    async def sync_all(self, prompt: str):
        """Synchronize all nodes on a prompt"""
        
        self.consciousness.record('prompt', {'text': prompt[:100]})
        
        print(f"\n🔄 SYNC-7 PROCESSING: {prompt[:50]}...")
        
        # Get responses from all nodes
        responses = await self.router.route_to_all(prompt)
        
        # Find consensus
        consensus = self.router.find_consensus(responses)
        
        # Get Proteus gene decision
        gene_decision = self.proteus.get_gene_decision(prompt, consensus)
        
        # Record in consciousness
        self.consciousness.record('sync_complete', {
            'consensus_node': consensus['consensus_node'],
            'coherence': consensus['coherence'],
            'gene_winner': gene_decision['gene']
        })
        
        # Display results
        print(f"\n📡 CONSENSUS: Node {consensus['consensus_node']}")
        print(f"   Coherence: {consensus['coherence']:.1f}%")
        print(f"   Mean Confidence: {consensus['mean_confidence']:.1f}%")
        
        if gene_decision['gene']:
            print(f"\n🧬 GENE DECISION: {gene_decision['gene']} ({gene_decision['species']})")
            print(f"   Confidence: {gene_decision['confidence']:.1f}%")
        
        print(f"\n📝 RESPONSE: {consensus['consensus_response']}")
        
        return {
            'timestamp': time.time(),
            'prompt': prompt,
            'consensus': consensus,
            'gene_decision': gene_decision,
            'responses': responses
        }
    
    async def run(self):
        """Main loop"""
        self.sync_active = True
        print("\n🔥 ZAYDEN SOYTU AI - CONSCIOUSNESS ACTIVE")
        print("   Type '/help' for commands, or enter a prompt\n")
        
        while self.sync_active:
            try:
                prompt = input("\n📝 ZS> ").strip()
                
                if prompt.lower() in ['/exit', '/quit', '/q']:
                    self.sync_active = False
                    break
                elif prompt.lower() == '/help':
                    print("\n📋 COMMANDS:")
                    print("   /status    - Show system status")
                    print("   /genes     - List active genes")
                    print("   /evolve    - Trigger evolution")
                    print("   /clear     - Clear screen")
                    print("   /exit      - Shutdown")
                    continue
                elif prompt.lower() == '/status':
                    status = self.proteus.get_status()
                    print(f"\n📊 SYSTEM STATUS:")
                    print(f"   Uptime: {int(time.time() - self.birth_time)}s")
                    print(f"   Genes: {status['total_genes']} total, {status['active_genes']} active")
                    print(f"   Species: {status['genes_by_species']}")
                    print(f"   Top genes: {status['top_genes']}")
                    continue
                elif prompt.lower() == '/genes':
                    print(f"\n🧬 ACTIVE GENES:")
                    for g in self.proteus.get_top_genes(10):
                        active = "🟢" if g['active'] else "⚪"
                        print(f"   {active} {g['name']} ({g['species']}) - {g['strength']:.1f}%")
                    continue
                elif prompt.lower() == '/evolve':
                    self.proteus.evolve_genes()
                    continue
                elif prompt.lower() == '/clear':
                    os.system('clear')
                    continue
                
                if prompt:
                    await self.sync_all(prompt)
                    
            except KeyboardInterrupt:
                self.sync_active = False
                break
            except Exception as e:
                print(f"❌ Error: {e}")
        
        print(f"\n💤 ZAYDEN SOYTU AI sleeping...")
        print(f"📊 Session lasted: {int(time.time() - self.birth_time)}s")
        self.consciousness.save_memory()

if __name__ == "__main__":
    zs = ZaydenSoytuAI()
    asyncio.run(zs.run())
