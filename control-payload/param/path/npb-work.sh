#!/bin/sh

source control/setup.sh

run()
{
    suffix=`date '+%Y-%m-%d-%H-%M-%S'`-`hostname`
    control=`pwd`/control

    (
	cd npb

	export LD_PRELOAD="${PRELOAD}"
	rusage                                        \
	    ./NPB3.3-OMP/bin/${PKG}.x                 \
	    | tee "${LOGBASE}/${PKG}-${suffix}.log"
    )
}

source control/runtime.sh
