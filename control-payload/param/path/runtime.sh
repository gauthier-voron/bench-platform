#!/bin/sh
# This script should be sourced and assume a run() function is defined.
# It also assume a TIME variable exist and indicate an amount of seconds.
# It will then execute run() a first time, then while there is time remaining,
# it will contnue to execute additional run(). An additional run can be stopped
# if the time is elapsed.

DEADLINE=`date '+%s'`
DEADLINE=$(( $DEADLINE + $TIME ))

run       # Do the first, unbreakable run

if [ `date '+%s'` -lt $DEADLINE ] ; then
    set -m
    (
	while true ; do
	    run          # Do additional runs
	done
    ) &
    loop=$!
    set +m

    duration=$(( $DEADLINE - `date '+%s'` ))
    sleep $duration
    kill -TERM -$loop
fi
