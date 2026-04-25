import subprocess
import socket
import threading
import time
from pathlib import Path

class ProteusBridge:
    PORT = 9161
    
    def __init__(self):
        self.peers = set()
        self.proteus_binary = Path.home() / "proteus_kernel" / "proteus_kernel"
        self.process = None
        
    def get_local_ip(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            s.connect(('8.8.8.8', 80))
            ip = s.getsockname()[0]
        except:
            ip = '127.0.0.1'
        finally:
            s.close()
        return ip
    
    def start_proteus(self):
        if self.proteus_binary.exists():
            self.process = subprocess.Popen([str(self.proteus_binary)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            return True
        return False
    
    def discover_peers(self, timeout=3):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.settimeout(timeout)
        sock.sendto(b"PROTEUS_DISCOVER", ('<broadcast>', self.PORT))
        peers = []
        try:
            while True:
                data, addr = sock.recvfrom(1024)
                if data.decode().startswith("PROTEUS_ACTIVE:"):
                    ip = data.decode().split(":")[1]
                    if ip != self.get_local_ip():
                        peers.append(ip)
        except socket.timeout:
            pass
        sock.close()
        return peers
    
    def sync(self, message):
        for peer in self.peers:
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                sock.sendto(message.encode(), (peer, self.PORT))
                sock.close()
            except:
                pass

bridge = ProteusBridge()
