#!/bin/sh

if [ $# -ne 5 ] ; then
    echo "$0: invalid argument count" >&2
    echo "syntax: $0 <bin> <lib> <wrk> <par> <cnt>" >&2
    exit 1
fi

bin="$1" ; shift      # binary for test
lib="$1" ; shift      # library to preload
wrk="$1" ; shift      # completion time in cycles
par="$1" ; shift      # percent of thread running in parallel
cnt="$1" ; shift      # delay between CS access try

cpu_count=`grep 'processor' /proc/cpuinfo | wc -l`
cs_cycles=$(( $cnt * $cpu_count ))
ps_cycles=$(( $cs_cycles * ( $par * $cpu_count ) / 100 ))
iter=$(( $wrk / ( $ps_cycles + $cnt ) ))
list=`seq 0 $(( $cpu_count - 1 ))`

time -v $bin $cpu_count $iter $cs_cycles $ps_cycles $list 2>&1 \
    | grep 'wall clock' \
    | sed -r 's/^.*([[:digit:]]+:[[:digit:]]+\.[[:digit:]]+)/\1/' \
    | xargs printf "%-10s"

(
    export LD_PRELOAD=$lib
    time -v $bin $cpu_count $iter $cs_cycles $ps_cycles $list 2>&1
) \
    | grep 'wall clock' \
    | sed -r 's/^.*([[:digit:]]+:[[:digit:]]+\.[[:digit:]]+)/\1/' \
    | xargs printf "%-10s\n"
