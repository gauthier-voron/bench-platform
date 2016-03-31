#!/bin/sh

source control/setup.sh

# For bodytrack, use the OpenMP version since it is the fastest on both native
# and virtualized environments
if [ "x$PKG" = "xbodytrack" ] ; then
    type=gcc-openmp
else
    type=gcc
fi

# The packages facesim and fluidanimate require a power of two thread count, so
# compute the nearest lower power of two thread count.
thread=$CORES
if [ "${PKG}" = "facesim" ] || [ "${PKG}" = "fluidanimate" ] ; then
    x=1
    while [ $thread -gt 1 ] ; do
	thread=$(( $thread / 2 ))
	x=$(( $x * 2 ))
    done
    thread=$x
fi

run()
{
    suffix=`date '+%Y-%m-%d-%H-%M-%S'`-`hostname`
    control=`pwd`/control

    (
	cd "parsec"
	source ./env.sh || source env.sh
	rm -rf "log/amd64-linux.$type.pre"

	export LD_PRELOAD="${PRELOAD}"
	parsecmgmt -a run -p "${PKG}" -n "${thread}" -x pre -i native \
		   -c "$type" -s "$control/rusage/rusage"
    )

    mv "parsec/log/amd64-linux.$type.pre/run"*".log" \
       "$LOGBASE/${PKG}-${suffix}.log"
}

source control/runtime.sh
