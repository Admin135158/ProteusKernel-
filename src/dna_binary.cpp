#include "morpheus_core.hpp"
using namespace morpheus;
class DNAEngine : public Engine {
public:
    DNAEngine() : Engine("dna_binary", 15010, getEnvSecret(), "data/dna_binary.journal") {}
    std::string encodeDNA(const std::string& in) {
        const char map[4] = {'A','T','C','G'};
        std::string out;
        for (unsigned char c : in) {
            out += map[(c>>6)&3]; out += map[(c>>4)&3];
            out += map[(c>>2)&3]; out += map[c&3];
        }
        return out;
    }
    void onFrame(int cfd, const Frame& f) override {
        if (f.type == HEARTBEAT) { sendFrame(cfd, mkFrame(RESPONSE, "DNA-ACK|ALIVE")); return; }
        std::string pl(f.payload.begin(), f.payload.end());
        if (f.type == COMMAND && pl.find("ENCODE") != std::string::npos) {
            std::string data = pl.substr(pl.find("ENCODE")+6);
            std::string enc = encodeDNA(data);
            sendFrame(cfd, mkFrame(RESPONSE, "DNA-ACK|ENCODED|BASE:ATCG|LENGTH:" + std::to_string(enc.size()) + "|DATA:" + enc));
        } else {
            sendFrame(cfd, mkFrame(RESPONSE, "DNA-ACK|READY"));
        }
    }
};
int main(int argc, char** argv) { DNAEngine e; e.start(argc, argv); return 0; }
