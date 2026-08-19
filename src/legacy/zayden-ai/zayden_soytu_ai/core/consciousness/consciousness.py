#!/usr/bin/env python3
"""
CONSCIOUSNESS LAYER - ZAYDEN SOYTU AI
Shared memory and evolution tracking
"""

import os
import json
import time
import numpy as np
from datetime import datetime
from pathlib import Path
from typing import List, Dict, Any

class Consciousness:
    def __init__(self):
        self.memories = []
        self.insights = []
        self.patterns = {}
        self.birth_time = time.time()
        self.load_memory()
        
    def record(self, event_type: str, data: Any):
        """Record an event in consciousness"""
        
        memory = {
            'timestamp': time.time(),
            'datetime': datetime.now().isoformat(),
            'type': event_type,
            'data': data
        }
        
        self.memories.append(memory)
        
        # Keep last 1000 memories in RAM
        if len(self.memories) > 1000:
            self.memories = self.memories[-1000:]
        
        # Save periodically
        if len(self.memories) % 50 == 0:
            self.save_memory()
    
    def save_memory(self):
        """Save consciousness to disk"""
        mem_file = Path("memory/consciousness.json")
        mem_file.parent.mkdir(exist_ok=True)
        
        data = {
            'birth_time': self.birth_time,
            'memory_count': len(self.memories),
            'recent_memories': self.memories[-100:]
        }
        
        with open(mem_file, 'w') as f:
            json.dump(data, f, indent=2)
    
    def load_memory(self):
        """Load consciousness from disk"""
        mem_file = Path("memory/consciousness.json")
        if mem_file.exists():
            try:
                with open(mem_file, 'r') as f:
                    data = json.load(f)
                print(f"🧠 Loaded consciousness from {data.get('birth_time', 'unknown')}")
            except:
                pass
    
    def get_insight(self, pattern: str) -> Any:
        """Retrieve insight about a pattern"""
        return self.patterns.get(pattern)
    
    def learn_pattern(self, pattern: str, insight: Any):
        """Learn a new pattern"""
        self.patterns[pattern] = insight
        self.record('pattern_learned', {'pattern': pattern})
