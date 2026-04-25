#!/data/data/com.termux/files/usr/bin/python
import sys
import os

def binary_to_dna(data):
    mapping = {'00':'A','01':'C','10':'G','11':'T'}
    binary = "".join(f"{b:08b}" for b in data)
    return "".join(mapping[binary[i:i+2]] for i in range(0, len(binary), 2))

def dna_to_binary(dna):
    mapping = {'A':'00','C':'01','G':'10','T':'11'}
    binary = "".join(mapping[c] for c in dna)
    return bytes(int(binary[i:i+8], 2) for i in range(0, len(binary), 8))

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python dna.py <file> [--decode]")
        sys.exit(1)
    
    if "--decode" in sys.argv:
        with open(sys.argv[1], 'r') as f:
            dna = f.read().strip()
        data = dna_to_binary(dna)
        with open("decoded_" + sys.argv[1].replace(".dna", ""), 'wb') as f:
            f.write(data)
        print(f"[DNA] Decoded to decoded_{sys.argv[1]}")
    else:
        with open(sys.argv[1], 'rb') as f:
            data = f.read()
        dna = binary_to_dna(data)
        output = sys.argv[1] + ".dna"
        with open(output, 'w') as f:
            f.write(dna)
        print(f"[DNA] Encoded to {output}")
