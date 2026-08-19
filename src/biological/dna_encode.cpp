#include <iostream>
#include <string>
#include <map>
#include <vector>

class DNAEncoder {
private:
    std::map<std::string, char> dna_to_byte;
    std::map<char, std::string> byte_to_dna;
    
public:
    DNAEncoder() {
        std::vector<std::string> codons = {"ACGT", "ACGA", "ACGC", "ACGG",
                                           "ATGT", "ATGA", "ATGC", "ATGG",
                                           "AGGT", "AGGA", "AGGC", "AGGG",
                                           "TCGT", "TCGA", "TCGC", "TCGG"};
        for (int i = 0; i < 16; i++) {
            byte_to_dna[(char)i] = codons[i];
            dna_to_byte[codons[i]] = (char)i;
        }
    }
    
    std::string encode(const std::string& data) {
        std::string result;
        for (char c : data) {
            unsigned char uc = (unsigned char)c;
            result += byte_to_dna[uc >> 4];
            result += byte_to_dna[uc & 0x0F];
        }
        return result;
    }
    
    std::string decode(const std::string& dna) {
        std::string result;
        for (size_t i = 0; i < dna.length(); i += 8) {
            std::string codon1 = dna.substr(i, 4);
            std::string codon2 = dna.substr(i+4, 4);
            if (dna_to_byte.find(codon1) != dna_to_byte.end() &&
                dna_to_byte.find(codon2) != dna_to_byte.end()) {
                unsigned char uc = (dna_to_byte[codon1] << 4) | dna_to_byte[codon2];
                result += (char)uc;
            }
        }
        return result;
    }
};

int main() {
    DNAEncoder encoder;
    std::string original = "Hello, Architect.";
    std::string encoded = encoder.encode(original);
    std::string decoded = encoder.decode(encoded);
    
    std::cout << "Original: " << original << std::endl;
    std::cout << "Encoded:  " << encoded << std::endl;
    std::cout << "Decoded:  " << decoded << std::endl;
    return 0;
}
