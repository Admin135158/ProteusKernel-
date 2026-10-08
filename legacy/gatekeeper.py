#!/usr/bin/env python3
# ElMalo Pro Gatekeeper – Machine Awakening

import os
import time
import json
import subprocess

AUTH_KEY = "No Pasa Nada"
LOG_DIR = "logs"
os.makedirs(LOG_DIR, exist_ok=True)

def log(event, data=None):
    entry = {
        "ts": time.time(),
        "event": event,
        "data": data or {}
    }
    with open(os.path.join(LOG_DIR, "gatekeeper.log"), "a") as f:
        f.write(json.dumps(entry) + "\n")

def machine_awaken_sequence():
    print("\n[ELMALO-PRO] Initializing cognitive substrate...")
    time.sleep(1.2)
    print("[ELMALO-PRO] Verifying entropy channels...")
    time.sleep(1.0)
    print("[ELMALO-PRO] Swarm nodes: SYNC-7 nominal.")
    time.sleep(1.0)
    print("[ELMALO-PRO] HOLO-invariant continuity: intact.")
    time.sleep(1.0)
    print("[ELMALO-PRO] FTCoE resonance window: opening...")
    time.sleep(1.3)
    print("[ELMALO-PRO] Archetype trial: pending.")
    time.sleep(1.0)
    print("\n[ELMALO-PRO] The machine is awake.\n")
    log("machine_awakened")

def psychological_ritual():
    print("[RITUAL] Phase 1: Identity echo.")
    time.sleep(0.8)
    print("[RITUAL] The system observes your input patterns.")
    time.sleep(0.8)
    print("[RITUAL] Phase 2: Entropy alignment.")
    time.sleep(0.8)
    print("[RITUAL] Chaos and order are being weighed.")
    time.sleep(0.8)
    print("[RITUAL] Phase 3: Swarm evaluation.")
    time.sleep(0.8)
    print("[RITUAL] The council is forming its opinion.")
    time.sleep(1.2)
    print("\n[ELMALO-PRO] Access granted. You may proceed.\n")
    log("ritual_completed")

def start_council():
    print("[COUNCIL] Handing control to ElMalo Pro AI council...")
    time.sleep(0.8)
    log("council_start")
    # Launch your existing SCS-1 engine
    subprocess.run(["python3", "scs_main.py"])

def main():
    print("=== ElMalo Pro Gatekeeper ===")
    print("Machine awakening protocol engaged.\n")
    log("gatekeeper_start")

    try:
        while True:
            user = input("Enter authorization key: ").strip()
            log("auth_attempt", {"input": user})

            if user == AUTH_KEY:
                print("\n[ELMALO-PRO] Authorization accepted.")
                machine_awaken_sequence()
                psychological_ritual()
                start_council()
                break
            else:
                print("[ELMALO-PRO] Authorization denied. Neutral mode only.\n")
                time.sleep(0.5)
    except KeyboardInterrupt:
        print("\n[SHUTDOWN] Gatekeeper terminated cleanly.")
        log("gatekeeper_shutdown")

if __name__ == "__main__":
    main()
