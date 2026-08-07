#!/usr/bin/env python3
"""
ZAYDEN SOYTU AI - WEB APPLICATION
Real web interface for your SYNC-7 kernel
"""

import os
import sys
import json
import time
import asyncio
import threading
from datetime import datetime
from flask import Flask, render_template, request, jsonify, send_from_directory
from flask_socketio import SocketIO, emit
from flask_cors import CORS

# Add to path
sys.path.append(os.path.dirname(__file__))

# Import Zayden core
from zayden_soytu import ZaydenSoytuAI

# Initialize Flask
app = Flask(__name__, 
            static_folder='static',
            template_folder='templates')
app.config['SECRET_KEY'] = 'zayden-soytu-secret'
CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*")

# Initialize Zayden kernel
print("🧠 Initializing Zayden Soytu AI Kernel...")
zayden = ZaydenSoytuAI()
kernel_thread = None
kernel_running = True

def run_kernel_loop():
    """Run the kernel in background"""
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    
    async def kernel_task():
        while kernel_running:
            # Let kernel evolve in background
            await asyncio.sleep(1)
            # Could add background processing here
    
    loop.run_until_complete(kernel_task())

# Start kernel in background thread
kernel_thread = threading.Thread(target=run_kernel_loop)
kernel_thread.daemon = True
kernel_thread.start()

# ========== WEB ROUTES ==========

@app.route('/')
def index():
    """Main web interface"""
    return render_template('index.html')

@app.route('/api/status', methods=['GET'])
def api_status():
    """Get system status"""
    status = {
        'name': zayden.name,
        'version': zayden.version,
        'uptime': int(time.time() - zayden.birth_time),
        'nodes': len(zayden.router.nodes) if hasattr(zayden, 'router') else 0,
        'genes': len(zayden.proteus.genes) if hasattr(zayden, 'proteus') else 0,
        'active_genes': sum(1 for g in zayden.proteus.genes.values() if g.get('active', False)) if hasattr(zayden, 'proteus') else 0,
        'species': zayden.proteus.get_species_count() if hasattr(zayden, 'proteus') else {},
        'timestamp': time.time()
    }
    return jsonify(status)

@app.route('/api/query', methods=['POST'])
def api_query():
    """Send a query to SYNC-7"""
    data = request.json
    prompt = data.get('prompt', '')
    
    if not prompt:
        return jsonify({'error': 'No prompt provided'}), 400
    
    # Run async query in thread
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    
    try:
        result = loop.run_until_complete(zayden.sync_all(prompt))
        
        # Emit via websocket
        socketio.emit('new_response', {
            'prompt': prompt,
            'response': result['consensus']['consensus_response'],
            'confidence': result['consensus']['coherence'],
            'timestamp': time.time()
        })
        
        return jsonify(result)
    except Exception as e:
        return jsonify({'error': str(e)}), 500
    finally:
        loop.close()

@app.route('/api/genes', methods=['GET'])
def api_genes():
    """Get gene information"""
    if not hasattr(zayden, 'proteus'):
        return jsonify({'error': 'Proteus not initialized'}), 500
    
    genes = []
    for name, gene in zayden.proteus.genes.items():
        genes.append({
            'name': name,
            'species': gene.get('species', 'unknown'),
            'strength': gene.get('strength', 0),
            'active': gene.get('active', False),
            'mutations': gene.get('mutations', 0),
            'kills': gene.get('kills', 0),
            'children': gene.get('children', 0)
        })
    
    return jsonify({
        'total': len(genes),
        'genes': sorted(genes, key=lambda x: x['strength'], reverse=True)
    })

@app.route('/api/evolve', methods=['POST'])
def api_evolve():
    """Trigger evolution"""
    if not hasattr(zayden, 'proteus'):
        return jsonify({'error': 'Proteus not initialized'}), 500
    
    zayden.proteus.evolve_genes()
    
    socketio.emit('evolution', {
        'generation': zayden.proteus.evolution_generation,
        'timestamp': time.time()
    })
    
    return jsonify({'success': True, 'generation': zayden.proteus.evolution_generation})

@app.route('/api/nodes', methods=['GET'])
def api_nodes():
    """Get node information"""
    if not hasattr(zayden, 'router'):
        return jsonify({'error': 'Router not initialized'}), 500
    
    nodes = []
    for node_id, handler in zayden.router.nodes.items():
        node_info = {
            'id': node_id,
            'platform': getattr(handler, 'platform', f'Node {node_id}'),
            'status': 'active'
        }
        
        # Add stats if available
        if hasattr(handler, 'get_stats'):
            try:
                stats = handler.get_stats()
                node_info['stats'] = stats
            except:
                pass
        
        nodes.append(node_info)
    
    return jsonify({
        'total': len(nodes),
        'nodes': nodes
    })

# ========== STATIC FILES ==========

@app.route('/static/<path:path>')
def serve_static(path):
    return send_from_directory('static', path)

# ========== MAIN ==========

if __name__ == '__main__':
    print(f"""
╔══════════════════════════════════════════════════════════╗
║     🌐 ZAYDEN SOYTU AI - WEB INTERFACE                  ║
║                                                            ║
║     Server: http://localhost:5000                         ║
║     Kernel: {zayden.name} v{zayden.version}                          ║
║     Status: RUNNING                                        ║
╚══════════════════════════════════════════════════════════╝
    """)
    
    socketio.run(app, host='0.0.0.0', port=5000, debug=True, allow_unsafe_werkzeug=True)
