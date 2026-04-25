#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

vector<string> get_peers() {
    vector<string> peers;
    ifstream f("peers.txt");
    string line;
    while (getline(f, line)) if (!line.empty()) peers.push_back(line);
    return peers;
}

void shard_file(const string& filename, int num_shards) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) { cerr << "[ERROR] Cannot open " << filename << endl; return; }
    vector<char> data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    vector<vector<char>> shards(num_shards);
    for (size_t i = 0; i < data.size(); i++) shards[i % num_shards].push_back(data[i]);
    for (int i = 0; i < num_shards; i++) {
        string shard_file = filename + ".shard." + to_string(i);
        ofstream out(shard_file, ios::binary);
        out.write(shards[i].data(), shards[i].size());
        out.close();
        cout << "[SHARD] Created " << shard_file << " (" << shards[i].size() << " bytes)" << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) { cout << "Usage: shard_dna <file> [num_shards]" << endl; return 1; }
    string file = argv[1];
    int num_shards = (argc > 2) ? atoi(argv[2]) : 4;
    auto peers = get_peers();
    if (peers.empty()) { cerr << "[ERROR] No peers found" << endl; return 1; }
    cout << "[DISTRIBUTE] Sharding " << file << " into " << num_shards << " pieces for " << peers.size() << " peers" << endl;
    shard_file(file, num_shards);
    cout << "[DISTRIBUTE] Shards ready. Use ./cpp_push to distribute." << endl;
    return 0;
}
