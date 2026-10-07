#!/usr/bin/env python3
import socket, struct, hmac, hashlib, sys

SECRET = b'MORPHEUS_DEV_KEY_CHANGE_IN_PRODUCTION'

def encode(msg_type, payload=b''):
    plen = len(payload)
    header = b'MORP' + struct.pack('>BBBBI', 2, msg_type, 0, 0, plen)
    pre = header + b'\x00'*32 + payload
    sig = hmac.new(SECRET, pre, hashlib.sha256).digest()
    return header + sig + payload

def decode(data):
    if len(data) < 44 or data[:4] != b'MORP': return None
    ver, typ, flags, reserved, plen = struct.unpack('>BBBBI', data[4:12])
    if len(data) < 44+plen: return None
    payload = data[44:44+plen]
    their = data[12:44]
    pre = data[:12] + b'\x00'*32 + payload
    mine = hmac.new(SECRET, pre, hashlib.sha256).digest()
    if not hmac.compare_digest(their, mine): return None
    return {'type': typ, 'payload': payload}

def sendrecv(host, port, msg_type, payload=b''):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(3.0)
    s.connect((host, port))
    s.sendall(encode(msg_type, payload))
    # Read header
    hdr = b''
    while len(hdr) < 44:
        chunk = s.recv(44 - len(hdr))
        if not chunk: break
        hdr += chunk
    if len(hdr) < 44: s.close(); return None
    plen = struct.unpack('>I', hdr[8:12])[0]
    body = b''
    while len(body) < plen:
        chunk = s.recv(plen - len(body))
        if not chunk: break
        body += chunk
    s.close()
    return decode(hdr + body)

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: morp_client.py HOST PORT [HEARTBEAT|REGISTER|COMMAND|STATUS] [payload]')
        sys.exit(1)
    host, port = sys.argv[1], int(sys.argv[2])
    cmd = sys.argv[3].upper() if len(sys.argv) > 3 else 'HEARTBEAT'
    payload = sys.argv[4].encode() if len(sys.argv) > 4 else b''
    types = {'HEARTBEAT': 0x01, 'AUTH_CHALLENGE': 0x02, 'REGISTER': 0x04, 'COMMAND': 0x07, 'STATUS': 0x0B}
    t = types.get(cmd, 0x01)
    resp = sendrecv(host, port, t, payload)
    if resp:
        print(f'Type: 0x{resp["type"]:02x} | Payload: {resp["payload"].decode(errors="replace")}')
    else:
        print('INVALID_RESPONSE or HMAC_MISMATCH')
