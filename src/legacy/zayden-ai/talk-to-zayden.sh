#!/bin/bash
echo "🧬 Talking to Zayden (type 'quit' to exit)"
echo "----------------------------------------"
while true; do
    read -p "You: " msg
    if [ "$msg" = "quit" ]; then
        break
    fi
    echo "$msg" | nc -u localhost 9161
    echo "Message sent to Zayden"
done
