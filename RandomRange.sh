#!/bin/sh

# usage ./RandomRange.sh <min> <max>

min=$1
max=$2

python -c "import random; print(random.randint($min,$max))"
