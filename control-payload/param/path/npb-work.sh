#!/bin/sh

pkg="$1" ; shift
time="$1" ; shift
active="$1" ; shift
cores="$1" ; shift


pinso=`pwd`/control/pin/lib/pin.so
chronoso=`pwd`/control/chronometer/lib/chronometer.so
activeso=`pwd`/control/activecond/lib/activecond.so
logbase=`pwd`/control/log

preload=""
if [ "x$active" = "xy" ] ; then
    preload="${preload}:${activeso}"
    export GOMP_SPINCOUNT=INFINITY
fi

if [ ! -d "control/log" ] ; then
    mkdir "control/log"
fi

cd npb

export OMP_NUM_THREADS=$cores

if [ "$#" -ne 0 ] ; then
    preload="${preload}:${pinso}"
    export PIN_RR="$*"
fi


run()
{
    suffix=`date '+%Y-%m-%d-%H-%M-%S'`-`hostname`

    export CHRONOMETER_OUTPUT="$logbase/chronometer-%d.log"

    ../control/iostat -d 1 > "../control/log/iostat-${suffix}.log" &
    iopid=$!

    free -s 1 > "../control/log/free-${suffix}.log" &
    freepid=$!

    (
	export LD_PRELOAD="${preload}"
	../control/rusage/rusage ./NPB3.3-OMP/bin/${pkg}.x \
	    | tee "../control/log/${pkg}-${suffix}.log"
    )

    kill $iopid
    kill $freepid
}


deadline=`date '+%s'`
deadline=$(( $deadline + $time ))


run


if [ `date '+%s'` -lt $deadline ] ; then
    set -m
    (
	while true ; do
	    run
	done
    ) &
    loop=$!
    set +m

    duration=$(( $deadline - `date '+%s'` ))
    sleep $duration
    kill -TERM -$loop
fi
