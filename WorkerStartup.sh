# usage .WorkerStartup.sh [Master Address]

# set the default address of the machine the master server runs on
master_address=lenss-comp1

# check if we should override the default master_address
if [[ $# -gt 0 ]]; then master_address=$1; fi

echo "master is ${master_address}"

port="$(python -c 'import random; print(random.randint(10000,40000))')"
./fbsd "$port" $master_address
