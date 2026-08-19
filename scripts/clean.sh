#!/bin/bash
pkill -f "heartbeat" 2>/dev/null
pkill -f "zayden_ultimate" 2>/dev/null
pkill -f "proteus_" 2>/dev/null
rm -f heartbeat zayden_ultimate proteus_v5_1 proteus_engine_v7
rm -f proteus_master cpp_push swarm dna_encode shard_dna
rm -f proteus_final remote_control proteus_fixed proteus_kernel
rm -f proteus_kernel_complete mutated agent
echo "🧹 Clean complete."
