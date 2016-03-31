#!/bin/sh
# Perform workload agnostic work to setup :
# - PKG     : the name of the package to use
# - TIME    : the minimum amount of time to work
# - CORES   : the amount of core to use
# - LOGBASE : where to put the logs
# - PRELOAD : the paths of the libraries to preload

PKG="$1" ; shift              # The package to use
TIME="$1" ; shift             # Minimum duration of the bench
active="$1" ; shift           # Use active locking
CORES="$1" ; shift            # Use a specified amount of cores
# Remaining parameters are the cores to use if pinning is needed

pinso=`pwd`/control/pin/lib/pin.so                     # thread pinning
activeso=`pwd`/control/activecond/lib/activecond.so    # active locking
LOGBASE=`pwd`/control/log                              # log destination


# What to preload =============================================================
# We use various preload hook to tune the behavior of the applications.
# Depending of the parameters, we choose here what to preload. We also set
# OpenMP parameters if needed.

PRELOAD=""

# Use active locking by preloading the pthread hooks and setting the OpenMP
# spincount to infinity
if [ "x$active" = "xy" ] ; then
    PRELOAD="${PRELOAD}:${activeso}"
    export GOMP_SPINCOUNT=INFINITY
fi

# Tell OpenMP runtime to use only the specified amount of cores
export OMP_NUM_THREADS=$CORES

# If pinning is needed, we preload the pthread hook to pin the thread in a
# round-robin fashion. Set the PIN_RR env variable to indicate on what core to
# pin the threads
if [ "$#" -ne 0 ] ; then
    PRELOAD="${PRELOAD}:${pinso}"
    export PIN_RR="$*"

    temp=`mktemp`
    for spec in $* ; do
	while [ "x$spec" != "x" ] ; do
	    range="${spec%%,*}"
	    start="${range%-*}"
	    end="${range#*-}"

	    seq $start $end

	    nspec="${spec#*,}"
	    if [ "$nspec" = "$spec" ] ; then
		spec=
	    else
		spec="$nspec"
	    fi
	done
    done > "$temp"

    accu=`sort -gu "$temp"`
    while [ `echo $accu | wc -w` -lt $CORES ] ; do
	accu="$accu $accu"
    done

    score=0
    for dcore in $accu ; do
    	if [ "x${PIN_MAP}" != "x" ] ; then
	    PIN_MAP="${PIN_MAP} "
	fi
	PIN_MAP="${PIN_MAP}${score}=${dcore}"

	score=$(( $score + 1 ))
	if [ $score -ge $CORES ] ; then
	    break
	fi
    done
    rm  "$temp"

    export PIN_MAP
fi

# Ensure the log destination exists
if [ ! -d "control/log" ] ; then
    mkdir "control/log"
fi
