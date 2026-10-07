#!/usr/bin/env python3
import os, sys, socket, signal, time, threading, hashlib

ENGINE = os.environ.get('PK_ENGINE', 'unknown')
PORT = int(os.environ.get('PK_PORT', 15001))
TIER = os.environ.get('PK_TIER', 'unknown')

running = True

def shutdown(signum, frame):
    global running
    print(f'[{ENGINE}] SIGTERM received, shutting down...')
    running = False

signal.signal(signal.SIGTERM, shutdown)
signal.signal(signal.SIGINT, shutdown)

def handle_tcp(conn, addr):
    with conn:
        try:
            data = conn.recv(4096)
            if not data:
                return
            msg = data.decode('utf-8', errors='ignore').strip()
            print(f'[{ENGINE}] RX from {addr}: {msg[:80]}')

            # Engine-specific protocol responses
            if ENGINE == 'pk_swarm':
                if 'SYNC-7' in msg or 'HANDSHAKE' in msg:
                    conn.sendall(b'SYNC-7-ACK|v1.0|ACCEPTED|PROTEUS_KERNEL\n')
                elif 'STATUS' in msg:
                    conn.sendall(b'SYNC-7-STATUS|SWARM_ACTIVE|7_NODES\n')
                else:
                    conn.sendall(b'SYNC-7-NAK|UNKNOWN\n')

            elif ENGINE == 'pk_heartbeat':
                if 'PING' in msg or 'BEAT' in msg:
                    conn.sendall(b'HB-ACK|ALIVE|TS:' + str(int(time.time())).encode() + b'\n')
                else:
                    conn.sendall(b'HB-ACK|LISTENING\n')

            elif ENGINE == 'gatekeeper':
                if 'AUTH' in msg:
                    conn.sendall(b'GK-ACK|AUTHORIZED|STRICT_MODE\n')
                elif 'VERIFY' in msg:
                    conn.sendall(b'GK-ACK|TOKEN_VALID\n')
                else:
                    conn.sendall(b'GK-NAK|UNAUTHORIZED\n')

            elif ENGINE == 'supervisor':
                if 'ARBITRATE' in msg:
                    conn.sendall(b'SUPER-ACK|ARBITRATION_READY|NO_CONFLICTS\n')
                else:
                    conn.sendall(b'SUPER-ACK|ONLINE\n')

            elif ENGINE == 'bodyguard':
                if 'TRUCE' in msg:
                    conn.sendall(b'BG-ACK|TRUCE_PROTOCOL_ACTIVE|ANTI_SABOTAGE_ENABLED\n')
                elif 'SCAN' in msg:
                    conn.sendall(b'BG-ACK|CLEAN|NO_THREATS\n')
                else:
                    conn.sendall(b'BG-ACK|GUARDING\n')

            elif ENGINE == 'pk_zayden':
                if 'BRIDGE' in msg:
                    conn.sendall(b'ZAYDEN-ACK|FEDERATED_BRIDGE_READY|MODELS:gemini,claude,deepseek,ollama\n')
                else:
                    conn.sendall(b'ZAYDEN-ACK|ONLINE\n')

            elif ENGINE == 'pk_gotem':
                h = hashlib.sha256(data).hexdigest()
                conn.sendall(f'GOTEM-ACK|SHA256:{h}\n'.encode())

            elif ENGINE == 'dna_binary':
                if 'ENCODE' in msg:
                    conn.sendall(b'DNA-ACK|ENCODE_READY|BASE64+ATCG\n')
                else:
                    conn.sendall(b'DNA-ACK|READY\n')

            else:
                conn.sendall(b'ACK|OK\n')

        except Exception as e:
            print(f'[{ENGINE}] Handler error: {e}')

def run_tcp():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(('127.0.0.1', PORT))
    s.listen(5)
    s.settimeout(1.0)
    print(f'[{ENGINE}] TCP bound on 127.0.0.1:{PORT}')
    print(f'[{ENGINE}] Protocol active. Waiting for connections...')
    while running:
        try:
            conn, addr = s.accept()
            t = threading.Thread(target=handle_tcp, args=(conn, addr), daemon=True)
            t.start()
        except socket.timeout:
            continue
        except OSError:
            break
    s.close()

def run_udp():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(('127.0.0.1', PORT))
    s.settimeout(1.0)
    print(f'[{ENGINE}] UDP bound on 127.0.0.1:{PORT}')
    while running:
        try:
            data, addr = s.recvfrom(4096)
            msg = data.decode('utf-8', errors='ignore').strip()
            print(f'[{ENGINE}] UDP RX from {addr}: {msg[:80]}')
            if 'DISCOVER' in msg or 'GOSSIP' in msg:
                s.sendto(b'GOSSIP-ACK|PEER_DISCOVERED|ACCEPTED\n', addr)
            else:
                s.sendto(b'GOSSIP-ACK|LISTENING\n', addr)
        except socket.timeout:
            continue
        except OSError:
            break
    s.close()

def main():
    banner = f"""
{'='*50}
  MORPHEUS INNOVATIONS & TECHNOLOGIES HOLDINGS LLC
  Engine: {ENGINE}
  Tier:   {TIER}
  Port:   {PORT}
  Mode:   HYBRID_STUB (Python)
{'='*50}
"""
    print(banner)
    print(f'[{ENGINE}] PID: {os.getpid()}')

    if ENGINE == 'swarm_gossip':
        run_udp()
    else:
        run_tcp()

    print(f'[{ENGINE}] Shutdown complete. Truce protocol disengaged.')

if __name__ == '__main__':
    main()
