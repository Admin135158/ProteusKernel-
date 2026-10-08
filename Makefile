CXX ?= clang++

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  OPENSSL_PREFIX := $(shell brew --prefix openssl)
else
  OPENSSL_PREFIX := /data/data/com.termux/files/usr
endif

CXXFLAGS = -std=c++17 -Wall -O2 -pthread -Iinclude -I$(OPENSSL_PREFIX)/include
LDFLAGS  = -L$(OPENSSL_PREFIX)/lib -lcrypto

BIN = bin

TARGETS = $(BIN)/pk_swarm $(BIN)/pk_heartbeat $(BIN)/gatekeeper \
          $(BIN)/pk_gotem $(BIN)/pk_zayden \
          $(BIN)/swarm_gossip $(BIN)/dna_binary $(BIN)/chaos \
          $(BIN)/bridge $(BIN)/arbitration $(BIN)/initiation $(BIN)/scs_main

all: $(BIN) $(TARGETS)

$(BIN):
	mkdir -p $(BIN)

$(BIN)/pk_swarm: src/pk_swarm.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/pk_swarm.cpp $(LDFLAGS)

$(BIN)/pk_heartbeat: src/pk_heartbeat.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/pk_heartbeat.cpp $(LDFLAGS)

$(BIN)/gatekeeper: src/gatekeeper.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/gatekeeper.cpp $(LDFLAGS)

$(BIN)/pk_gotem: src/pk_gotem.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/pk_gotem.cpp $(LDFLAGS)

$(BIN)/pk_zayden: src/pk_zayden.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/pk_zayden.cpp $(LDFLAGS)

$(BIN)/swarm_gossip: src/swarm_gossip.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/swarm_gossip.cpp $(LDFLAGS)

$(BIN)/dna_binary: src/dna_binary.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/dna_binary.cpp $(LDFLAGS)

$(BIN)/chaos: src/chaos.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/chaos.cpp $(LDFLAGS)

$(BIN)/bridge: src/bridge.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/bridge.cpp $(LDFLAGS)

$(BIN)/arbitration: src/arbitration.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/arbitration.cpp $(LDFLAGS)

$(BIN)/initiation: src/initiation.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/initiation.cpp $(LDFLAGS)

$(BIN)/scs_main: src/scs_main.cpp include/morp.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/scs_main.cpp $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
