CXX ?= clang++
CXXFLAGS = -std=c++17 -Wall -O2 -pthread -Iinclude -I/data/data/com.termux/files/usr/include
LDFLAGS = -L/data/data/com.termux/files/usr/lib -lcrypto
BIN = bin

TARGETS = $(BIN)/arbitration $(BIN)/chaos $(BIN)/bridge $(BIN)/initiation $(BIN)/swarm_gossip $(BIN)/scs_main

all: $(BIN) $(TARGETS)

$(BIN):
	mkdir -p $(BIN)

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

clean:
	rm -f $(BIN)/arbitration $(BIN)/chaos $(BIN)/bridge $(BIN)/initiation $(BIN)/swarm_gossip $(BIN)/scs_main

.PHONY: all clean
