#!/bin/sh

source control/setup.sh

MEM=`free -m | perl -wnl -e 's|^-/\+.*:\s+\d+\s+(\d+)|$1| and print'`

case "$PKG" in
    kmeans)
	command="./metis/kmeans 2048 768 524288 64 -p $CORES -q"
	;;
    pca)
	command="./metis/pca -R 4096 -C 8192 -M 8 -p $CORES -q"
	;;
    wrmem)
	command="./metis/wrmem -s 896 -p $CORES -q"
	;;
    matrix)
	command="./metis/matrix_mult l 8192 -p $CORES -q"
	;;
    wc)
	command="./metis/wc \"`pwd`/mosbench/words/wc-input.txt\" -p $CORES -q"
	;;
    wr)
	command="./metis/wr \"`pwd`/mosbench/words/wr-input.txt\" -p $CORES -q"
	;;
    psearchy)
	export LD_LIBRARY_PATH="`pwd`/mosbench/psearchy/lib:$LD_LIBRARY_PATH"
	for i in `seq 0 $CORES` ; do mkdir mosbench/psearchy/db$i ; done
	command="./psearchy/pedsort -t psearchy/db -c $CORES -m 1024 \
	       < words/pedsort-input.txt"
	;;
    memcached)
	export PATH="`pwd`/mosbench/memcached/bin:$PATH"
	export LD_LIBRARY_PATH="`pwd`/mosbench/memcached/lib:$LD_LIBRARY_PATH"
	command="memcached-bench.sh $CORES $MEM 20000000"
	;;
esac

run()
{
    suffix=`date '+%Y-%m-%d-%H-%M-%S'`-`hostname`
    control=`pwd`/control

    (
	cd mosbench
	command="'$control/rusage/rusage' $command \
                 | tee '$LOGBASE/${PKG}-${suffix}.log'"

	export LD_PRELOAD="${PRELOAD}"
	eval `echo $command`
    )
}

source control/runtime.sh
