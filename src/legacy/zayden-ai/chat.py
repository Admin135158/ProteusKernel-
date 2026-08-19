#!/usr/bin/env python3
import socket
import sys

def send_udp(message, host='localhost', port=9162):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(message.encode(), (host, port))
    sock.settimeout(2)
    try:
        data, _ = sock.recvfrom(4096)
        print(data.decode())
    except socket.timeout:
        print("No response from Zayden")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        msg = "TALK:" + " ".join(sys.argv[1:])
        send_udp(msg)
    else:
        while True:
            user = input("You: ")
            if user.lower() == 'exit':
                break
            send_udp("TALK:" + user)
