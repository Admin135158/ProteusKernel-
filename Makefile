CXX ?= clang++
OPENSSL_PREFIX := /usr/local/opt/openssl@4
CXXFLAGS = -std=c++17 -Wall -O2 -pthread -Iinclude -I$(OPENSSL_PREFIX)/include
LDFLAGS = -L$(OPENSSL_PREFIX)/lib -lcrypto
BIN = bin

TARGETS = $(BIN)/gatekeeper $(BIN)/arbitration $(BIN)/chaos $(BIN)/bridge $(BIN)/initiation $(BIN)/swarm_gossip $(BIN)/scs_main $(BIN)/dna_binary

all: $(BIN) $(TARGETS)

$(BIN):
	mkdir -p $(BIN)

$(BIN)/gatekeeper: src/gatekeeper.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/gatekeeper.cpp $(LDFLAGS)

$(BIN)/arbitration: src/arbitration.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/arbitration.cpp $(LDFLAGS)

$(BIN)/chaos: src/chaos.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/chaos.cpp $(LDFLAGS)

$(BIN)/bridge: src/bridge.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/bridge.cpp $(LDFLAGS)

$(BIN)/initiation: src/initiation.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/initiation.cpp $(LDFLAGS)

$(BIN)/swarm_gossip: src/swarm_gossip.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/swarm_gossip.cpp $(LDFLAGS)

$(BIN)/scs_main: src/scs_main.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/scs_main.cpp $(LDFLAGS)

$(BIN)/dna_binary: src/dna_binary.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/dna_binary.cpp $(LDFLAGS)

clean:
	rm -f $(BIN)/gatekeeper $(BIN)/arbitration $(BIN)/chaos $(BIN)/bridge $(BIN)/initiation $(BIN)/swarm_gossip $(BIN)/scs_main $(BIN)/dna_binary

.PHONY: all clean
