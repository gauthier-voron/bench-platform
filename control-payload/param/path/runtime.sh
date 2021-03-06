#!/bin/sh
# This script should be sourced and assume a run() function is defined.
# It also assume a TIME variable exist and indicate an amount of seconds.
# It will then execute run() a first time, then while there is time remaining,
# it will contnue to execute additional run(). An additional run can be stopped
# if the time is elapsed.

DEADLINE=`date '+%s'`
DEADLINE=$(( $DEADLINE + $TIME ))

ctrldir="`pwd`/control"
logdir="$ctrldir/log"
bindir="$ctrldir/bin"

export PATH="$bindir":"$PATH"


_run_application()
{
    suffix="`date '+%Y-%m-%d-%H-%M-%S'`-`hostname`.log"
    
    soft-network > "$logdir/soft-network-$suffix" &
    soft_network_pid=$!

    soft-disk --sync > "$logdir/soft-disk-$suffix" &
    soft_disk_pid=$!

    run

    kill $soft_disk_pid
    kill $soft_network_pid
    wait
}

_run_application       # Do the first, unbreakable run

if [ `date '+%s'` -lt $DEADLINE ] ; then
    set -m
    (
	while true ; do
	    _run_application          # Do additional runs
	done
    ) &
    loop=$!
    set +m

    duration=$(( $DEADLINE - `date '+%s'` ))
    sleep $duration
    kill -TERM -$loop
fi
