#!/usr/bin/env python3
import sys
import hashlib

class DNAEncoder:
    def __init__(self):
        self.codons = ['ACGT', 'ACGA', 'ACGC', 'ACGG', 'ATGT', 'ATGA', 'ATGC', 'ATGG',
                       'AGGT', 'AGGA', 'AGGC', 'AGGG', 'TCGT', 'TCGA', 'TCGC', 'TCGG']
        self.byte_to_dna = {i: self.codons[i] for i in range(16)}
        self.dna_to_byte = {self.codons[i]: i for i in range(16)}
    
    def encode(self, data):
        result = ''
        for byte in data.encode('utf-8'):
            result += self.byte_to_dna[byte >> 4]
            result += self.byte_to_dna[byte & 0x0F]
        return result
    
    def decode(self, dna):
        result = ''
        for i in range(0, len(dna), 8):
            if i + 8 > len(dna): break
            codon1, codon2 = dna[i:i+4], dna[i+4:i+8]
            if codon1 in self.dna_to_byte and codon2 in self.dna_to_byte:
                byte_val = (self.dna_to_byte[codon1] << 4) | self.dna_to_byte[codon2]
                result += chr(byte_val)
        return result

if __name__ == '__main__':
    encoder = DNAEncoder()
    data = sys.argv[1] if len(sys.argv) > 1 else "PROTEUS_KERNEL"
    encoded = encoder.encode(data)
    decoded = encoder.decode(encoded)
    print(f"Original: {data}")
    print(f"Encoded:  {encoded}")
    print(f"Decoded:  {decoded}")
