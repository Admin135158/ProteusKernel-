#!/data/data/com.termux/files/usr/bin/bash

TABLET="192.168.18.72"

case "$1" in
    status)
        python3 ~/proteus_kernel/remote_control.py $TABLET STATUS
        ;;
    replicate)
        python3 ~/proteus_kernel/remote_control.py $TABLET REPLICATE
        ;;
    mutate)
        python3 ~/proteus_kernel/remote_control.py $TABLET MUTATE
        ;;
    backup)
        python3 ~/proteus_kernel/remote_control.py $TABLET BACKUP
        ;;
    kill)
        python3 ~/proteus_kernel/remote_control.py $TABLET KILL
        ;;
    exec)
        shift
        python3 ~/proteus_kernel/remote_control.py $TABLET "$*"
        ;;
    *)
        echo "Usage: ./swarm.sh {status|replicate|mutate|backup|kill|exec <cmd>}"
        ;;
esac
