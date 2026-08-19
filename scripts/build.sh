#!/bin/bash
echo "🔨 Building PROTEUS Kernel..."
clang++ -std=c++17 -pthread -o heartbeat heartbeat.cpp
clang++ -std=c++17 -pthread -o zayden_ultimate zayden_ultimate.cpp
clang++ -std=c++17 -pthread -o proteus_v5_1 proteus_v5_1.cpp
clang++ -std=c++17 -pthread -o proteus_engine_v7 PROTEUS_ENGINE_V7.cpp
clang++ -std=c++17 -pthread -o proteus_master proteus_master.cpp
clang++ -std=c++17 -pthread -o cpp_push cpp_push.cpp
clang++ -std=c++17 -pthread -o swarm swarm.cpp
clang++ -std=c++17 -pthread -o dna_encode dna_encode.cpp
clang++ -std=c++17 -pthread -o shard_dna shard.dna.cpp
clang++ -std=c++17 -pthread -o proteus_final proteus_final.cpp
clang++ -std=c++17 -pthread -o remote_control remote_control.cpp
clang++ -std=c++17 -pthread -o proteus_fixed proteus_fixed.cpp
clang++ -std=c++17 -pthread -o proteus_kernel_bin proteus_kernel.cpp
clang++ -std=c++17 -pthread -o proteus_kernel_complete proteus_kernel_complete.cpp
clang++ -std=c++17 -pthread -o mutated mutated.cpp
clang++ -std=c++17 -pthread -o agent agent.cpp
echo "✅ All binaries built."
