#!/data/data/com.termux/files/usr/bin/python3
import sys
import os

def binary_to_dna(data):
    """Convert binary data to DNA base pairs (A/C/G/T)"""
    mapping = {'00':'A', '01':'C', '10':'G', '11':'T'}
    binary = ''.join(f'{b:08b}' for b in data)
    return ''.join(mapping[binary[i:i+2]] for i in range(0, len(binary), 2))

def dna_to_binary(dna):
    """Convert DNA base pairs back to binary"""
    mapping = {'A':'00', 'C':'01', 'G':'10', 'T':'11'}
    binary = ''.join(mapping[c] for c in dna)
    return bytes(int(binary[i:i+8], 2) for i in range(0, len(binary), 8))

if len(sys.argv) < 2:
    print("Usage: python dna_encode.py <file> [--decode]")
    print("")
    print("Examples:")
    print("  python dna_encode.py proteus_kernel           # Encode binary to DNA")
    print("  python dna_encode.py kernel.dna --decode      # Decode DNA back to binary")
    sys.exit(1)

filepath = sys.argv[1]
output_base = os.path.basename(filepath)

if '--decode' in sys.argv:
    # Decode DNA back to binary
    with open(filepath, 'r') as f:
        dna = f.read().strip()
    data = dna_to_binary(dna)
    output = f"/sdcard/ProteusVault/dna/decoded_{output_base.replace('.dna', '')}"
    with open(output, 'wb') as f:
        f.write(data)
    print(f"[DNA] Decoded to: {output}")
else:
    # Encode binary to DNA
    if not os.path.exists(filepath):
        print(f"[ERROR] File not found: {filepath}")
        sys.exit(1)
    
    with open(filepath, 'rb') as f:
        data = f.read()
    
    dna = binary_to_dna(data)
    
    # Save to SD card vault
    output = f"/sdcard/ProteusVault/dna/{output_base}.dna"
    with open(output, 'w') as f:
        f.write(dna)
    
    print(f"[DNA] Encoded to: {output}")
    print(f"[DNA] Original size: {len(data)} bytes")
    print(f"[DNA] DNA length: {len(dna)} base pairs")
    print(f"[DNA] Preview: {dna[:100]}...")
