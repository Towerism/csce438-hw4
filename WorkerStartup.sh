#!/bin/sh
# usage .WorkerStartup.sh [Master Address]

master_address=$(cat default_master_address)

# check if we should override the default master_address
if [ $# -gt 0 ]; then master_address=$1; fi

port="$(python -c 'import random; print(random.randint(10000,40000))')"
./fbsd "$port" $master_address  &
