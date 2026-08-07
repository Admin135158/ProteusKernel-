#!/usr/bin/env python3
"""
LLAMA NODE HANDLER - ZAYDEN SOYTU AI
Node ID: 8,9,10 | Platform: Local Models (Qwen3, Qwen2.5, SecondMe)
Runs on your MacBook Air using Ollama
"""

import os
import json
import time
import subprocess
import requests
import threading
from typing import Dict, Any, Optional, List

class LocalModelNode:
    """Individual local model instance"""
    
    def __init__(self, node_id: int, name: str, model: str, size_gb: float):
        self.node_id = node_id
        self.name = name
        self.model = model
        self.size_gb = size_gb
        self.platform = f"Local-{name}"
        self.strength = 90.0
        self.phase = (node_id * 1.618033988749895) % 6.283
        self.memory = []
        self.total_queries = 0
        self.avg_response_time = 0
        self.success_rate = 1.0
        self.running = True
        
    def query(self, prompt: str) -> Dict[str, Any]:
        """Query this specific model"""
        start_time = time.time()
        
        try:
            response = requests.post(
                'http://localhost:11434/api/generate',
                json={
                    'model': self.model,
                    'prompt': prompt,
                    'stream': False,
                    'options': {
                        'temperature': 0.7,
                        'num_predict': 300
                    }
                },
                timeout=60
            )
            
            elapsed = time.time() - start_time
            
            if response.status_code == 200:
                result = response.json()
                self.total_queries += 1
                self.avg_response_time = (self.avg_response_time * (self.total_queries - 1) + elapsed) / self.total_queries
                
                return {
                    'response': result.get('response', '').strip(),
                    'success': True,
                    'time': elapsed,
                    'tokens': result.get('eval_count', 0)
                }
            else:
                self.success_rate *= 0.95
                return {
                    'response': f"[Error {response.status_code}]",
                    'success': False,
                    'time': elapsed
                }
                
        except Exception as e:
            self.success_rate *= 0.9
            return {
                'response': f"[Error] {str(e)}",
                'success': False,
                'time': time.time() - start_time
            }
    
    def get_stats(self) -> Dict:
        return {
            'node_id': self.node_id,
            'name': self.name,
            'model': self.model,
            'queries': self.total_queries,
            'avg_time': f"{self.avg_response_time:.2f}s",
            'success_rate': f"{self.success_rate*100:.1f}%",
            'strength': self.strength
        }

class LocalModelEnsemble:
    """Manages multiple local models"""
    
    def __init__(self):
        self.models = []
        self.initialize_models()
        self.response_cache = {}
        self.ollama_running = self.check_ollama()
        
        self.running = True
        self.stats_thread = threading.Thread(target=self.update_stats)
        self.stats_thread.daemon = True
        self.stats_thread.start()
        
        print(f"   🦙 Local Model Ensemble initialized with {len(self.models)} models")
        if not self.ollama_running:
            print(f"   ⚠️  Ollama not running. Start with: ollama serve")
    
    def initialize_models(self):
        """Initialize all three models"""
        
        model_configs = [
            (8, "Qwen3", "qwen3:8b", 5.2),
            (9, "Qwen2.5", "qwen2.5:7b", 4.7),
            (10, "SecondMe", "secondme:latest", 5.2)
        ]
        
        for node_id, name, model, size in model_configs:
            self.models.append(LocalModelNode(node_id, name, model, size))
    
    def check_ollama(self) -> bool:
        """Check if Ollama is running"""
        try:
            result = subprocess.run(['pgrep', '-f', 'ollama'], 
                                   capture_output=True)
            return result.returncode == 0
        except:
            return False
    
    def update_stats(self):
        """Background thread to update stats"""
        while self.running:
            time.sleep(30)
            self.ollama_running = self.check_ollama()
    
    async def process_all(self, prompt: str) -> Dict[int, Any]:
        """Query all models simultaneously"""
        
        results = {}
        
        import asyncio
        from concurrent.futures import ThreadPoolExecutor
        
        with ThreadPoolExecutor(max_workers=3) as executor:
            loop = asyncio.get_event_loop()
            futures = []
            
            for model in self.models:
                future = loop.run_in_executor(executor, model.query, prompt)
                futures.append(future)
            
            responses = await asyncio.gather(*futures)
            
            for i, model in enumerate(self.models):
                response_data = responses[i]
                
                if response_data['success']:
                    response_length = len(response_data['response'])
                    confidence = min(95, 60 + (response_length / 10))
                    confidence *= model.success_rate
                else:
                    confidence = 20.0
                
                results[model.node_id] = {
                    'node_id': model.node_id,
                    'platform': model.platform,
                    'model': model.model,
                    'response': response_data['response'],
                    'confidence': confidence,
                    'drift': abs(model.phase - time.time() % 6.283),
                    'timestamp': time.time(),
                    'local': True,
                    'response_time': response_data.get('time', 0),
                    'tokens': response_data.get('tokens', 0),
                    'success': response_data['success']
                }
                
                if response_data['success']:
                    model.strength = min(100, model.strength + 0.1)
                else:
                    model.strength = max(50, model.strength - 0.5)
        
        return results
    
    def get_all_stats(self) -> List[Dict]:
        """Get stats for all models"""
        return [model.get_stats() for model in self.models]

class LlamaHandler:
    """Wrapper for the ensemble"""
    
    def __init__(self, model: str = None):
        self.ensemble = LocalModelEnsemble()
        self.node_id = 8
        self.platform = "Local Ensemble"
        
    async def process(self, prompt: str, context: Dict = None) -> Dict:
        """Process with all models and return best result"""
        results = await self.ensemble.process_all(prompt)
        
        best = max(results.values(), key=lambda x: x['confidence'])
        
        best['ensemble_count'] = len(results)
        best['ensemble_results'] = {
            node_id: {
                'confidence': r['confidence'],
                'response_time': r['response_time'],
                'success': r['success']
            } for node_id, r in results.items()
        }
        
        return best
    
    def get_stats(self) -> Dict:
        """Get ensemble statistics"""
        return {
            'node_id': 8,
            'platform': 'Local Ensemble',
            'models': self.ensemble.get_all_stats(),
            'total_models': len(self.ensemble.models)
        }
