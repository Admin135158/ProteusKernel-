#!/bin/bash
echo "🔀 MERGING PROTEUS COMPONENTS"
echo "  - proteus_v5_1.cpp → merged.cpp"
echo "  - zayden_ultimate.cpp → merged.cpp"
echo "  - PROTEUS_ENGINE_V7.cpp → merged.cpp"

cat proteus_v5_1.cpp > merged.cpp
echo "// ---- ZAYDEN MERGE ----" >> merged.cpp
cat zayden_ultimate.cpp >> merged.cpp
echo "// ---- ENGINE V7 MERGE ----" >> merged.cpp
cat PROTEUS_ENGINE_V7.cpp >> merged.cpp

g++ -std=c++17 -pthread -o proteus_merged merged.cpp
echo "✅ Merge complete. Binary: ./proteus_merged"
