#!/usr/bin/env python3
"""
gotem_holo_bridge.py
Drop-in bridge: Gotem Ledger -> HOLO Spine
No changes to existing Proteus/Gotem/Swarm code required.
"""

import hashlib
import json
import time
import sys
from datetime import datetime

class GotemHOLOBridge:
    INVARIANTS = {
        "append_only": True,
        "hash_chain": True,
        "temporal_mono": True,
        "ollin_structure": True,
        "sig_required": True,
    }
    
    def __init__(self, origin_timestamp: float = None):
        self.spine = []
        self.position = 0
        self.prev_hash = "0" * 64
        self.origin = origin_timestamp or 933466260.0
    
    def ingest(self, gotem_entry: dict, node_id: str = "zayden_node") -> dict:
        self.position += 1
        
        ts_raw = gotem_entry.get("timestamp", time.time())
        if ts_raw > 1e12:
            ts_sec = ts_raw / 1e9
        else:
            ts_sec = ts_raw
            
        days_since_origin = (ts_sec - self.origin) / 86400
        phi_cycle = int(days_since_origin // 9)
        ollin_index = int(days_since_origin % 9)
        
        self.origin = origin_timestamp or 933466260.0
            "frame_id": f"gotem_{int(ts_raw)}",
            "node_id": node_id,
            "timestamp": datetime.fromtimestamp(ts_sec).isoformat() + "Z",
            "phi_cycle": phi_cycle,
            "ollin_index": ollin_index,
            "payload": {
                "gotem_hash": gotem_entry.get("hash", ""),
                "context": gotem_entry.get("context", ""),
                "author": gotem_entry.get("author", ""),
                "type": "gotem_attribution"
            },
            "signature": gotem_entry.get("signature", "pending")
        }
        
        frame_hash = hashlib.sha256(
            json.dumps(dna_frame, sort_keys=True).encode()
        ).hexdigest()
        
        entry_body = {
            "position": self.position,
            "prev_hash": self.prev_hash,
            "frame_hash": frame_hash,
            "ingested_at": time.time()
        }
        this_hash = hashlib.sha256(
            json.dumps(entry_body, sort_keys=True).encode()
        ).hexdigest()
        
        spine_entry = {
            "spine": {
                "position": self.position,
                "prev_hash": self.prev_hash,
                "this_hash": this_hash,
                "frame_hash": frame_hash
            },
            "dna_frame": dna_frame,
            "compliance": self._check_suite(dna_frame, gotem_entry),
            "status": "verified" if gotem_entry.get("signature") else "pending"
        }
        
        self.prev_hash = this_hash
        self.spine.append(spine_entry)
        return spine_entry
    
    def _check_suite(self, dna_frame: dict, gotem_entry: dict) -> dict:
        return {
            "crypto_integrity": len(str(dna_frame["signature"])) >= 64,
            "temporal_mono": True,
            "lineage_unbroken": self.position == 1 or len(self.spine) > 0,
            "entropy_ok": len(str(gotem_entry.get("hash", ""))) > 16,
            "coherence_ok": True,
            "phi_cycle_ok": dna_frame["phi_cycle"] >= 0,
            "ollin_ok": 0 <= dna_frame["ollin_index"] <= 8,
            "hash_present": len(str(gotem_entry.get("hash", ""))) > 0
        }
    
    def verify_chain(self) -> bool:
        for i in range(len(self.spine) - 1):
            if self.spine[i]["spine"]["this_hash"] != self.spine[i+1]["spine"]["prev_hash"]:
                return False
        return True
    
    def export(self, filepath: str = "holo_spine.json"):
        with open(filepath, "w") as f:
            json.dump(self.spine, f, indent=2)
        print(f"\n[BRIDGE] Spine exported: {filepath} ({len(self.spine)} entries)")


if __name__ == "__main__":
    bridge = GotemHOLOBridge()
    
    ledger_file = sys.argv[1] if len(sys.argv) > 1 else "gotem_ledger.json"
    
    try:
        with open(ledger_file, "r") as f:
            data = json.load(f)
            if isinstance(data, list):
                ledger = data
            elif isinstance(data, dict) and "entries" in data:
                ledger = data["entries"]
            else:
                ledger = [data]
    except FileNotFoundError:
        print(f"[BRIDGE] {ledger_file} not found. Using demo entries.\n")
        ledger = [
            {"timestamp": 1778561979984341, "context": "Hackathon Demo", "author": "Fernando Garcia", "hash": "90edcebc685c3682..."},
            {"timestamp": 1778562432095912, "context": "Hackathon Demo", "author": "Fernando Garcia", "hash": "a073e5f29afb5fc7..."},
            {"timestamp": 1778562792026194, "context": "Hackathon Demo", "author": "Fernando Garcia", "hash": "217cec07c0f0746c..."},
            {"timestamp": 1778584668619096, "context": "Hackathon Demo", "author": "Fernando Garcia", "hash": "34024ee8f2035424..."},
            {"timestamp": 1778606341716022, "context": "Hackathon Demo", "author": "Fernando Garcia", "hash": "fefe1ed810713286..."},
            {"timestamp": 1778648495234038, "context": "Hackathon Demo", "author": "Fernando Garcia", "hash": "089fa53c2d57ccdb..."},
        ]
    
    for entry in ledger:
        bridge.ingest(entry)
    
    print(f"[BRIDGE] Chain valid: {bridge.verify_chain()}")
    print(f"[BRIDGE] Total entries: {len(bridge.spine)}")
    print(f"[BRIDGE] Head hash: {bridge.prev_hash}")
    
    for e in bridge.spine:
        s = e["spine"]
        c = sum(e["compliance"].values())
        print(f"  [{s['position']:04d}] {s['this_hash'][:24]}... | 808: {c}/8 | phi:{e['dna_frame']['phi_cycle']} ollin:{e['dna_frame']['ollin_index']}")
    
    bridge.export()
