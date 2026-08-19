#!/usr/bin/env python3
from flask import Flask, request, jsonify
from flask_cors import CORS
import threading
import time
from biosim_engine import BiosimEngine
import json

app = Flask(__name__)
CORS(app)

engine = BiosimEngine()
simulation_thread = None
running = False

def simulation_loop():
    global running
    while running:
        if engine.is_running:
            engine.step()
        time.sleep(0.35)

@app.route('/api/health', methods=['GET'])
def health():
    return jsonify({"status": "ok", "simulation_loaded": True, "is_running": engine.is_running})

@app.route('/api/status', methods=['GET'])
def status():
    return jsonify(engine.get_status())

@app.route('/api/top-sequences', methods=['GET'])
def top_sequences():
    limit = int(request.args.get('limit', 8))
    sequences = engine.get_top_sequences(limit)
    # Convert to serializable list
    return jsonify([{
        'id': g['id'],
        'name': g.get('name', ''),
        'species': g.get('species', 'proteus'),
        'fitness': g['fitness'],
        'replication_fidelity': g['replication_fidelity'],
        'mutations': g['mutations'],
        'sequence': g['sequence'][:20] + '...'  # truncate for display
    } for g in sequences])

@app.route('/api/events', methods=['GET'])
def events():
    limit = int(request.args.get('limit', 20))
    return jsonify(engine.get_events(limit))

@app.route('/api/control/start', methods=['POST'])
def start_sim():
    global running, simulation_thread
    if not running:
        running = True
        simulation_thread = threading.Thread(target=simulation_loop, daemon=True)
        simulation_thread.start()
    engine.start()
    return jsonify({"status": "started", "generation": engine.generation})

@app.route('/api/control/pause', methods=['POST'])
def pause_sim():
    engine.pause()
    return jsonify({"status": "paused"})

@app.route('/api/save', methods=['POST'])
def save():
    engine.save_state()
    return jsonify({"status": "saved", "generation": engine.generation})

@app.route('/api/control/architect/purge', methods=['POST'])
def architect_purge():
    engine.architect_purge()
    return jsonify({"status": "purged"})

@app.route('/api/control/architect/synthesize', methods=['POST'])
def architect_synthesize():
    engine.architect_synthesize()
    return jsonify({"status": "synthesized"})

@app.route('/api/control/architect/elevate', methods=['POST'])
def architect_elevate():
    engine.architect_elevate()
    return jsonify({"status": "elevated"})

@app.route('/api/control/architect/meditate', methods=['POST'])
def architect_meditate():
    engine.architect_meditate()
    return jsonify({"status": "meditated"})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=False)
