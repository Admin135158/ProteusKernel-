#!/usr/bin/env python3
# ElMalo Pro – Sovereign Cognitive Substrate (SCS-1)

import json
import socket
import time
from chaos_engine import ElMalo
from kernel_bridge import ProteusKernelBridge
from arbitration import ZaydenCortex

def udp_send(port, payload):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(payload.encode(), ("127.0.0.1", port))

def main():
    malo = ElMalo()
    kernel = ProteusKernelBridge()
    cortex = ZaydenCortex()

    while True:
        psi_state = malo.synthesize_state()
        unified_state = kernel.process_state(psi_state)
        final_output = cortex.arbitrate(unified_state)

        udp_send(9164, json.dumps(unified_state))
        print("\n[FINAL OUTPUT]\n", final_output)

        time.sleep(1)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n[SHUTDOWN] ElMalo Pro terminated cleanly.")
