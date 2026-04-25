# Proteus Kernel — Master Makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -pthread
LDFLAGS = -pthread

# Directories
SRCDIR = src
INCDIR = include
BINDIR = bin
OBJDIR = obj

# Core engine sources
ENGINE_SRCS = $(SRCDIR)/engine/PROTEUS_ENGINE_V7.cpp \
              $(SRCDIR)/engine/proteus_final.cpp \
              $(SRCDIR)/engine/proteus_kernel_complete.cpp \
              $(SRCDIR)/engine/proteus_master.cpp \
              $(SRCDIR)/engine/heartbeat.cpp \
              $(SRCDIR)/engine/swarm.cpp \
              $(SRCDIR)/engine/shard.dna.cpp \
              $(SRCDIR)/engine/dna_encode.cpp

# Zayden sources
ZAYDEN_SRCS = $(SRCDIR)/zayden/zayden_ultimate.cpp \
              $(SRCDIR)/bridge/zayden_bridge.cpp

# All sources
ALL_SRCS = $(ENGINE_SRCS) $(ZAYDEN_SRCS)

# Targets
.PHONY: all core zayden clean dirs

all: dirs core zayden

dirs:
@mkdir -p $(BINDIR) $(OBJDIR)

core: dirs
$(CXX) $(CXXFLAGS) -I$(INCDIR) $(ENGINE_SRCS) -o $(BINDIR)/proteus_engine $(LDFLAGS)

zayden: dirs
$(CXX) $(CXXFLAGS) -I$(INCDIR) $(ALL_SRCS) -o $(BINDIR)/proteus_zayden $(LDFLAGS)

clean:
rm -rf $(OBJDIR) $(BINDIR)/*

test:
@echo "Engine sources:"
@ls -la $(SRCDIR)/engine/
@echo ""
@echo "Zayden sources:"
@ls -la $(SRCDIR)/zayden/
