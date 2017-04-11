# csce438-hw4
An improved chat room server and client inspired by twitter feeds

# Building
Run `make`

# Running
The Master server location must be visible to all components of the system. By
default master is assumed to be located on the server `lenss-comp1`. This can be
changed by modifying the file `default_master_address`. The worker and client
startup scripts enable overriding `default_master_address` on a one-time basis.

## Scripts
We have several scripts to make it easier to start the various system
components.

### MasterStartup.sh
This script can be run on any machine. It takes no arguments. Just run it to
spin up the master server.

```
Usage: MasterStartup.sh
```

### WorkerStartup.sh
This script can be run on any machine. It takes one optional argument which will
override the default master address.

```
Usage: MasterStartup.sh [Master Address]
```

### ClientStartup.sh
This script can be run on any machine. It takes one required and one optional
argument which will override the default master address.

```
Usage: MasterStartup.sh <User Name> [Master Address]
```

## Running on Different Machines
Run each command on a different machine (obviously master should be started on lenss-comp1):
```
 MasterStartup.sh
```
```
$ WorkerStartup.sh
```
```
$ ClientStartup.sh MyUser
```

## Running on Different Machines (override default master address)
In this example we'll run master on a machine whose LAN address is 144.144.144.144.
Run each command on a different machine:
```
$ MasterStartup.sh
```
```
$ WorkerStartup.sh 144.144.144.144
```
```
$ ClientStartup.sh MyUser 144.144.144.144
```

# To Implement:
## master:
- currently has nothing for handling replication.
- currently cannot handle if a new server comes online
- At least insofar as duplicating old files
- currently cannot handle rebalancing connection points between servers
