#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <bitset>

using namespace std;

const map<string, char> BIN_TO_DNA = {{"00", 'A'}, {"01", 'C'}, {"10", 'G'}, {"11", 'T'}};
const map<char, string> DNA_TO_BIN = {{'A', "00"}, {'C', "01"}, {'G', "10"}, {'T', "11"}};

string binary_to_dna(const vector<unsigned char>& data) {
    string dna, binary;
    for (unsigned char byte : data) binary += bitset<8>(byte).to_string();
    for (size_t i = 0; i + 1 < binary.length(); i += 2) {
        string pair = binary.substr(i, 2);
        auto it = BIN_TO_DNA.find(pair);
        if (it != BIN_TO_DNA.end()) dna += it->second;
    }
    return dna;
}

vector<unsigned char> dna_to_binary(const string& dna) {
    string binary;
    vector<unsigned char> result;
    for (char c : dna) {
        auto it = DNA_TO_BIN.find(toupper(c));
        if (it != DNA_TO_BIN.end()) binary += it->second;
    }
    for (size_t i = 0; i + 7 < binary.length(); i += 8) {
        result.push_back((unsigned char)bitset<8>(binary.substr(i, 8)).to_ulong());
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: dna_encode <file> [--decode]" << endl;
        return 1;
    }
    string filename = argv[1];
    bool decode = (argc >= 3 && string(argv[2]) == "--decode");
    
    if (decode) {
        ifstream file(filename);
        if (!file.is_open()) { cerr << "Cannot open " << filename << endl; return 1; }
        string dna, line;
        while (getline(file, line)) dna += line;
        file.close();
        auto binary = dna_to_binary(dna);
        string outfile = filename + ".bin";
        ofstream out(outfile, ios::binary);
        out.write((char*)binary.data(), binary.size());
        out.close();
        cout << "[DNA] Decoded " << dna.length() << " bp → " << binary.size() << " bytes" << endl;
    } else {
        ifstream file(filename, ios::binary);
        if (!file.is_open()) { cerr << "Cannot open " << filename << endl; return 1; }
        vector<unsigned char> data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();
        string dna = binary_to_dna(data);
        string outfile = filename + ".dna";
        ofstream out(outfile);
        out << dna;
        out.close();
        cout << "[DNA] Encoded " << data.size() << " bytes → " << dna.length() << " bp" << endl;
        cout << "[DNA] Preview: " << dna.substr(0, 100) << "..." << endl;
    }
    return 0;
}
