#!/bin/sh

LIB="$1"
if [ "x$2" != "x" ] ; then
    BIN="$2"
else
    BIN=./
fi


check_config()
{
    out=`mktemp`
    cor=`mktemp`
    name="$1" ; shift

    set -m
    (
	export PIN_RR="$1"
	export LD_PRELOAD="$LIB"
    
	"$BIN/pthread" $2 > "$out"
    ) &
    pid=$!
    set +m

    sleep 11
    if ps $pid >/dev/null ; then
	kill -TERM -$pid
	echo "timeout config $name" >&2
    fi

    shift 2
    for elm in "$@" ; do
	echo $elm
    done > "$cor"

    if ! diff "$out" "$cor" >/dev/null ; then
	echo "failed config $name"
	echo "-- expected"
	cat "$cor"
	echo "-- but got"
	cat "$out"
	echo "--"
    fi >&2

    rm "$out" "$cor"
}


check_config "main 0"                0    ""    1
check_config "main 1"                1    ""    2
check_config "multi single"          0     4    1 1 1 1 1
check_config "multi multi"        "0 1"    4    1 2 1 2 1
check_config "multi choice"   "0,1 2,3"    4    3 c 3 c 3
check_config "multi range"      "0-2 3"    4    7 8 7 8 7
