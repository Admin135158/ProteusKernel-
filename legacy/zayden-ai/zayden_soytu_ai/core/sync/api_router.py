#!/usr/bin/env python3
"""
API ROUTER - ZAYDEN SOYTU AI
Routes requests to all nodes including local Llama
"""

import os
import sys
import json
import asyncio
import time
from typing import Dict, List, Any
import numpy as np

sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(__file__))))

from nodes.gemini.handler import GeminiHandler
from nodes.grok.handler import GrokHandler
from nodes.claude.handler import ClaudeHandler
from nodes.perplexity.handler import PerplexityHandler
from nodes.deepseek.handler import DeepSeekHandler
from nodes.chatgpt.handler import ChatGPTHandler
from nodes.llama.handler import LlamaHandler

class APIRouter:
    def __init__(self):
        self.nodes = {}
        self.node_stats = {}
        self.initialize_nodes()
        self.request_count = 0
        self.response_cache = {}
        
    def initialize_nodes(self):
        """Initialize all node handlers"""
        print("   Initializing node 2: Gemini")
        self.nodes[2] = GeminiHandler()
        
        print("   Initializing node 3: Grok")
        self.nodes[3] = GrokHandler()
        
        print("   Initializing node 4: Claude")
        self.nodes[4] = ClaudeHandler()
        
        print("   Initializing node 5: Perplexity")
        self.nodes[5] = PerplexityHandler()
        
        print("   Initializing node 6: DeepSeek")
        self.nodes[6] = DeepSeekHandler()
        
        print("   Initializing node 7: ChatGPT")
        self.nodes[7] = ChatGPTHandler()
        
        print("   Initializing nodes 8-10: Local Model Ensemble")
        self.nodes[8] = LlamaHandler()
        
        print(f"✅ All {len(self.nodes)} nodes initialized")
        
    async def route_to_all(self, prompt: str) -> Dict[int, Any]:
        """Send prompt to all nodes simultaneously"""
        tasks = []
        
        for node_id, handler in self.nodes.items():
            task = handler.process(prompt)
            tasks.append(task)
        
        results = await asyncio.gather(*tasks, return_exceptions=True)
        
        responses = {}
        node_ids = list(self.nodes.keys())
        
        for i, result in enumerate(results):
            node_id = node_ids[i]
            if isinstance(result, Exception):
                responses[node_id] = {
                    'node_id': node_id,
                    'platform': f"Node {node_id}",
                    'response': f"[Error] {str(result)}",
                    'confidence': 0,
                    'error': True,
                    'timestamp': time.time()
                }
            else:
                responses[node_id] = result
            
        self.request_count += 1
        return responses
    
    def find_consensus(self, responses: Dict[int, Any]) -> Dict:
        """Find consensus across all responses"""
        
        # Filter out errors
        valid_responses = {k: v for k, v in responses.items() 
                          if not v.get('error', False) and v.get('confidence', 0) > 0}
        
        if not valid_responses:
            return {
                'consensus_node': None,
                'consensus_response': "No valid responses",
                'consensus_confidence': 0,
                'mean_confidence': 0,
                'std_confidence': 0,
                'coherence': 0
            }
        
        confidences = [r['confidence'] for r in valid_responses.values()]
        mean_conf = float(np.mean(confidences))
        std_conf = float(np.std(confidences))
        
        # Find node closest to mean
        consensus_node = min(
            valid_responses.items(),
            key=lambda item: abs(item[1]['confidence'] - mean_conf)
        )
        
        # Find most divergent node
        most_divergent = max(
            valid_responses.items(),
            key=lambda item: abs(item[1]['confidence'] - mean_conf)
        )
        
        # Track node stats for evolution
        for node_id, response in valid_responses.items():
            if node_id not in self.node_stats:
                self.node_stats[node_id] = {
                    'total_responses': 0,
                    'avg_confidence': 0,
                    'times_consensus': 0
                }
            
            stats = self.node_stats[node_id]
            stats['total_responses'] += 1
            stats['avg_confidence'] = (stats['avg_confidence'] * (stats['total_responses'] - 1) + 
                                       response['confidence']) / stats['total_responses']
            
            if node_id == consensus_node[0]:
                stats['times_consensus'] += 1
        
        return {
            'consensus_node': consensus_node[0],
            'consensus_response': consensus_node[1]['response'],
            'consensus_confidence': float(consensus_node[1]['confidence']),
            'mean_confidence': mean_conf,
            'std_confidence': std_conf,
            'most_divergent_node': most_divergent[0],
            'most_divergent_confidence': float(most_divergent[1]['confidence']),
            'coherence': mean_conf * (1 - std_conf/100) if std_conf <= 100 else mean_conf,
            'node_count': len(valid_responses)
        }
    
    def get_node_stats(self) -> Dict:
        """Get statistics for all nodes"""
        return self.node_stats
