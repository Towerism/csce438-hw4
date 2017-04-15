#!/bin/sh

# usage .MasterStartup.sh [spawnID]

creation_id=0

# check if we should override the default creation_id
if [ $# -gt 0 ]; then creation_id=$1; fi

./fbmaster $creation_id
