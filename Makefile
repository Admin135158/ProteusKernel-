# SPDX-License-Identifier: Proprietary
# Copyright (c) 2026 Fernando De Jesus Garcia Gonzalez (The Architect)

CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -I$(PREFIX)/include
LDFLAGS = -L$(PREFIX)/lib -lssl -lcrypto -pthread

SRCDIR = src
TARGETS = pk_heartbeat pk_zayden pk_gotem pk_swarm pk_push

all: $(TARGETS)

pk_heartbeat: $(SRCDIR)/pk_heartbeat.cpp
$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

pk_zayden: $(SRCDIR)/pk_zayden.cpp
$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

pk_gotem: $(SRCDIR)/pk_gotem.cpp
$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

pk_swarm: $(SRCDIR)/pk_swarm.cpp
$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

pk_push: $(SRCDIR)/pk_push.cpp
$(CXX) $(CXXFLAGS) -o $@ $<

clean:
rm -f $(TARGETS) *.o

.PHONY: all clean
