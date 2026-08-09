# ProteusKernel Makefile
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread
LDFLAGS = -lcrypto -lcurl

BUILD_DIR = build

TARGETS = $(BUILD_DIR)/dna_binary \
            $(BUILD_DIR)/pk_zayden \
            $(BUILD_DIR)/pk_heartbeat \
            $(BUILD_DIR)/zayden_ultimate \
            $(BUILD_DIR)/heartbeat \
            $(BUILD_DIR)/supervisor

.PHONY: all clean dirs symlinks

all: dirs $(TARGETS) symlinks

dirs:
	@mkdir -p $(BUILD_DIR)

symlinks: $(TARGETS)
	@ln -sf pk_zayden $(BUILD_DIR)/consensus_test 2>/dev/null || true
	@ln -sf pk_heartbeat $(BUILD_DIR)/heartbeat_secure 2>/dev/null || true
	@ln -sf zayden_ultimate $(BUILD_DIR)/zayden_full 2>/dev/null || true
	@echo "Symlinks created"

$(BUILD_DIR)/dna_binary: dna_binary.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/pk_zayden: src/pk_zayden.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/pk_heartbeat: src/pk_heartbeat.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BUILD_DIR)/zayden_ultimate: zayden_ultimate.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< -lpthread

$(BUILD_DIR)/heartbeat: mesh/heartbeat.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< -lpthread

$(BUILD_DIR)/supervisor: gate/supervisor.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< -lpthread

clean:
	rm -rf $(BUILD_DIR)
	@echo "Clean complete"

stop:
	-pkill -f pk_zayden
	-pkill -f pk_heartbeat
	-pkill -f zayden_ultimate
	-pkill -f supervisor
	@echo "All Proteus processes stopped"

