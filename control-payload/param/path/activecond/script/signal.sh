#!/bin/sh

if [ $# -ne 6 ] ; then
    echo "$0: invalid argument count" >&2
    echo "syntax: $0 <bin> <lib> <wrk> <wat> <par> <cnt>" >&2
    exit 1
fi

bin="$1" ; shift      # binary for test
lib="$1" ; shift      # library to preload
wrk="$1" ; shift      # completion time in cycles
par="$1" ; shift      # percent of thread running in parallel
wat="$1" ; shift      # completion time in cycles
cnt="$1" ; shift      # delay between CS access try

cpu_count=`grep 'processor' /proc/cpuinfo | wc -l`
cs_cycles=$(( $cnt * $cpu_count ))
ps_cycles=$(( $cs_cycles * ( $par * $cpu_count ) / 100 ))
iter=$(( $wrk / ( $ps_cycles + $cnt ) ))
prodrate=$(( ( 100 - $wat ) / 2 ))
prod=$(( $cpu_count * $prodrate / 100 ))
cons=$(( $cpu_count - $prod ))
plist=`seq 0 $(( $prod - 1 ))`
clist=`seq $prod $(( $cpu_count - 1 ))`

time -v $bin $prod $cons $iter $cpu_count $cs_cycles $ps_cycles \
        $plist -- $clist 2>&1 \
    | grep 'wall clock' \
    | sed -r 's/^.*([[:digit:]]+:[[:digit:]]+\.[[:digit:]]+)/\1/' \
    | xargs printf "%-10s"

(
    export LD_PRELOAD=$lib
    time -v $bin $prod $cons $iter $cpu_count $cs_cycles $ps_cycles \
	    $plist -- $clist 2>&1
) \
    | grep 'wall clock' \
    | sed -r 's/^.*([[:digit:]]+:[[:digit:]]+\.[[:digit:]]+)/\1/' \
    | xargs printf "%-10s\n"
