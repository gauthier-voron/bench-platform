#!/bin/sh

if [ $# -ne 6 ] ; then
    echo "$0: invalid argument count" >&2
    echo "syntax: $0 <bin> <lib> <wrk> <wak> <par> <cnt>" >&2
    exit 1
fi

bin="$1" ; shift      # binary for test
lib="$1" ; shift      # library to preload
wrk="$1" ; shift      # completion time in cycles
wak="$1" ; shift      # wake up contention
par="$1" ; shift      # percent of thread running in parallel
cnt="$1" ; shift      # delay between clocks access try

cpu_count=`grep 'processor' /proc/cpuinfo | wc -l`
thrds=$(( $cpu_count - 1 ))
sg_cycles=$cnt
cs_cycles=$wak
ns_cycles=$(( $cs_cycles * ( $par * $thrds ) / 100 ))
iter=$(( $wrk / ( $sg_cycles + $ns_cycles + $cs_cycles ) ))
list=`seq 0 $(( $cpu_count - 1 ))`

time -v $bin $thrds $iter $sg_cycles $cs_cycles $ns_cycles $list 2>&1 \
    | grep 'wall clock' \
    | sed -r 's/^.*([[:digit:]]+:[[:digit:]]+\.[[:digit:]]+)/\1/' \
    | xargs printf "%-10s"

(
    export LD_PRELOAD=$lib
    time -v $bin $thrds $iter $sg_cycles $cs_cycles $ns_cycles $list 2>&1 \
) \
    | grep 'wall clock' \
    | sed -r 's/^.*([[:digit:]]+:[[:digit:]]+\.[[:digit:]]+)/\1/' \
    | xargs printf "%-10s\n"
