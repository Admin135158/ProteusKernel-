#!/usr/bin/env python3
import sys
import hashlib
import base64

class DNAEncoder:
    def __init__(self):
        self.codon_map = {
            0: 'ACGT', 1: 'ACGA', 2: 'ACGC', 3: 'ACGG',
            4: 'ATGT', 5: 'ATGA', 6: 'ATGC', 7: 'ATGG',
            8: 'AGGT', 9: 'AGGA', 10: 'AGGC', 11: 'AGGG',
            12: 'TCGT', 13: 'TCGA', 14: 'TCGC', 15: 'TCGG'
        }
        self.reverse_map = {v: k for k, v in self.codon_map.items()}
    
    def encode_file(self, filename):
        with open(filename, 'rb') as f:
            data = f.read()
        return self.encode_bytes(data)
    
    def encode_bytes(self, data):
        result = ''
        for byte in data:
            result += self.codon_map[byte >> 4]
            result += self.codon_map[byte & 0x0F]
        return result
    
    def decode_to_bytes(self, dna):
        result = bytearray()
        for i in range(0, len(dna), 8):
            if i + 8 > len(dna): break
            codon1 = dna[i:i+4]
            codon2 = dna[i+4:i+8]
            if codon1 in self.reverse_map and codon2 in self.reverse_map:
                byte_val = (self.reverse_map[codon1] << 4) | self.reverse_map[codon2]
                result.append(byte_val)
        return bytes(result)
    
    def hash_dna(self, dna):
        return hashlib.sha256(dna.encode()).hexdigest()

if __name__ == '__main__':
    encoder = DNAEncoder()
    
    if len(sys.argv) < 2:
        print("Usage: dna_encode.py [file]")
        sys.exit(1)
    
    encoded = encoder.encode_file(sys.argv[1])
    dna_hash = encoder.hash_dna(encoded)
    
    print(f"Encoded DNA length: {len(encoded)} chars")
    print(f"DNA SHA256: {dna_hash}")
    print(f"First 100 chars: {encoded[:100]}...")
