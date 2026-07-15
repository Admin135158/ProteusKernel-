#!/usr/bin/env python3
import socket
import subprocess
import sys

PORT = 9162

def send_command(cmd, target_ip="127.0.0.1"):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(cmd.encode(), (target_ip, PORT))
    data, _ = sock.recvfrom(4096)
    print(data.decode())
    sock.close()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: remote_control.py [command] [target_ip]")
        sys.exit(1)
    
    cmd = sys.argv[1]
    target = sys.argv[2] if len(sys.argv) > 2 else "127.0.0.1"
    send_command(cmd, target)
