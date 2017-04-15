#!/bin/bash

# usage .MasterStartup.sh [spawnID]

creation_id=0

# check if we should override the default master_address
if [ $# -gt 0 ]; then creation_id=$1; fi
port="$(./RandomRange.sh 10000 40000)"

./fbmasterRep "$port" $creation_id &> /dev/null &
