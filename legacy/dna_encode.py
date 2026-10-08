import sys

def to_dna(data: bytes) -> str:
    mapping = ["A", "C", "G", "T"]
    out = []
    for b in data:
        out.append(mapping[(b >> 6) & 0x3])
        out.append(mapping[(b >> 4) & 0x3])
        out.append(mapping[(b >> 2) & 0x3])
        out.append(mapping[b & 0x3])
    return "".join(out)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python dna_encode.py <string>")
        sys.exit(1)
    s = sys.argv[1].encode()
    print(to_dna(s))
