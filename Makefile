CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -I/data/data/com.termux/files/usr/include
LDFLAGS = -L/data/data/com.termux/files/usr/lib -lssl -lcrypto -pthread
TARGETS = pk_heartbeat pk_zayden pk_gotem pk_swarm pk_push
SRCS = src/pk_heartbeat.cpp src/pk_zayden.cpp src/pk_gotem.cpp src/pk_swarm.cpp src/pk_push.cpp src/truce/truce_protocol.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGETS)

pk_heartbeat: src/pk_heartbeat.o
$(CXX) -o $@ $^ $(LDFLAGS)

pk_zayden: src/pk_zayden.o
$(CXX) -o $@ $^ $(LDFLAGS)

pk_gotem: src/pk_gotem.o
$(CXX) -o $@ $^ $(LDFLAGS)

pk_swarm: src/pk_swarm.o src/truce/truce_protocol.o
$(CXX) -o $@ $^ $(LDFLAGS)

pk_push: src/pk_push.o
$(CXX) -o $@ $^ $(LDFLAGS)

src/truce/truce_protocol.o: src/truce/truce_protocol.cpp src/truce/truce_protocol.h src/truce/truce_types.h
$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
rm -f $(TARGETS) src/*.o src/truce/*.o

.PHONY: all clean
