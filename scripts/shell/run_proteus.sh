#!/data/data/com.termux/files/usr/bin/bash
cd ~/proteus_kernel

# Check if source is intact
if [ $(stat -c %a proteus_v5_1.cpp) != "444" ]; then
    echo "[SECURITY] Source modified. Restoring..."
    git checkout proteus_v5_1.cpp
    chmod 444 proteus_v5_1.cpp
fi

# Run kernel
./proteus_v5_1
