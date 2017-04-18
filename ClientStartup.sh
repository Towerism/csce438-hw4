#!/bin/bash
# usage ./ClientStartup.sh <username> [Master Address]

master_address=$(cat default_master_address)
if [ $# -eq 0 ]; then
    echo "Usage: $0 <username> [Master Address]"
    exit 1
fi

# check if we should override the default master_address
if [ $# -gt 1 ]; then master_address=$2; fi

master_port=$(cat master_port)
connection="$master_address:$master_port"

./fbc $1 "$connection"
