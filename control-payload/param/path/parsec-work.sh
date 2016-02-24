#!/bin/sh

pkg="$1" ; shift
time="$1" ; shift
active="$1" ; shift
cores="$1" ; shift

if [ "x$pkg" = "xbodytrack" ] ; then
    type=gcc-openmp
else
    type=gcc
fi

root=`pwd`
datalog="log/amd64-linux.$type.pre"
pinso=$root/control/pin/lib/pin.so
chronoso=$root/control/chronometer/lib/chronometer.so
activeso=$root/control/activecond/lib/activecond.so
logbase=$root/control/log

preload=""
if [ "x$active" = "xy" ] ; then
    preload="${preload}:${activeso}"
    export GOMP_SPINCOUNT=INFINITY
fi

if [ "$#" -ne 0 ] ; then
    preload="${preload}:${pinso}"
    export PIN_RR="$*"
fi

if [ ! -d "$logbase" ] ; then
    mkdir "$logbase"
fi


thread=$cores

if [ "${pkg}" = "facesim" ] || [ "${pkg}" = "fluidanimate" ] ; then
    x=1
    while [ $thread -gt 1 ] ; do
	thread=$(( $thread / 2 ))
	x=$(( $x * 2 ))
    done
    thread=$x
fi


cd "parsec"
source ./env.sh || source env.sh
rm -rf "${datalog}"


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
	parsecmgmt -a run -p "${pkg}" -n "${thread}" -x pre -i native \
		   -c "$type" -s "$root/control/rusage/rusage"
    )

    kill $iopid
    kill $freepid
    mv "${datalog}/run"*".log" "$root/control/log/${pkg}-${suffix}.log"
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
