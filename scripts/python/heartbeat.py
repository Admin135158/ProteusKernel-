#!/usr/bin/env python3
import socket
import threading
import time
import os

# Your cellular IP from ifconfig
MY_IP = "100.88.83.53"
HEARTBEAT_PORT = 9161

# Known peers from your config
KNOWN_PEERS = [
    "100.84.68.68",
    "100.93.246.209"
]

active_peers = set()

def send_heartbeat():
    """Send heartbeat to all known peers"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    
    while True:
        msg = f"HEARTBEAT:{MY_IP}:PROTEUS_ACTIVE"
        
        # Send to known peers
        for peer in KNOWN_PEERS:
            try:
                sock.sendto(msg.encode(), (peer, HEARTBEAT_PORT))
                print(f"[SEND] Heartbeat to {peer}")
            except Exception as e:
                print(f"[ERROR] Can't send to {peer}: {e}")
        
        # Try broadcast on all interfaces (might work if on WiFi)
        try:
            sock.sendto(msg.encode(), ('255.255.255.255', HEARTBEAT_PORT))
        except:
            pass
        
        time.sleep(5)

def listen_heartbeat():
    """Listen for incoming heartbeats"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('0.0.0.0', HEARTBEAT_PORT))
    
    print(f"[LISTEN] Waiting for heartbeats on port {HEARTBEAT_PORT}")
    
    while True:
        try:
            data, addr = sock.recvfrom(1024)
            msg = data.decode()
            
            if msg.startswith("HEARTBEAT:"):
                parts = msg.split(":")
                if len(parts) >= 3:
                    peer_ip = parts[1]
                    if peer_ip != MY_IP:
                        active_peers.add(peer_ip)
                        print(f"[PEER] Found {peer_ip} at {time.ctime()}")
                        
                        # Save to file
                        with open("active_peers.txt", "a") as f:
                            f.write(f"{peer_ip}\n")
        except Exception as e:
            print(f"[ERROR] {e}")

def command_listener():
    """Listen for commands on port 9162"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('0.0.0.0', 9162))
    
    while True:
        try:
            data, addr = sock.recvfrom(4096)
            msg = data.decode()
            
            if msg.startswith("CMD:"):
                cmd = msg[4:]
                print(f"[COMMAND] From {addr[0]}: {cmd}")
                
                # Execute command
                os.system(cmd + " &")
        except Exception as e:
            print(f"[ERROR] {e}")

if __name__ == "__main__":
    print("=" * 40)
    print("SYNC-7 HEARTBEAT NODE")
    print(f"My IP: {MY_IP}")
    print(f"Known peers: {KNOWN_PEERS}")
    print("=" * 40)
    
    # Start threads
    t1 = threading.Thread(target=send_heartbeat, daemon=True)
    t2 = threading.Thread(target=listen_heartbeat, daemon=True)
    t3 = threading.Thread(target=command_listener, daemon=True)
    
    t1.start()
    t2.start()
    t3.start()
    
    print("\n[RUNNING] Press Ctrl+C to stop\n")
    
    # Keep alive
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n[STOPPED]")
