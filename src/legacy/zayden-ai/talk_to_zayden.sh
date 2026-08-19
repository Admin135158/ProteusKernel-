#!/bin/bash

echo "💬 Talk to Zayden (type 'quit' to exit)"
echo "========================================"
echo "Commands: STATUS, MUTATE, BACKUP, PEERS, HELP"
echo ""

while true; do
    read -p "You: " msg
    if [ "$msg" = "quit" ]; then
        echo "Goodbye, Architect."
        break
    fi
    echo "TALK:$msg" | nc -u localhost 9162
    echo ""
done
