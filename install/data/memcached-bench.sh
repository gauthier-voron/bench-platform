#!/bin/sh

CORES="$1" ; shift
MEMORY="$1" ; shift
LOAD="$1" ; shift

SERVER_CORES=$(( $CORES / 2 ))
CLIENT_CORES=$(( $CORES - $SERVER_CORES ))


nwords()
{
    n="$1" ; shift
    echo "$@" | tr ' ' '\n'  | head -n $n | tr '\n' ' '
}

lwords()
{
    n="$1" ; shift
    echo "$@" | tr ' ' '\n'  | tail -n +$(( $n + 1 )) | tr '\n' ' '
}

if [ "x$PIN_RR" != "x" ] ; then
    while [ `echo $PIN_RR | wc -w` -lt $CORES ] ; do
	PIN_RR="$PIN_RR $PIN_RR"
    done
    SERVER_PIN_RR="`nwords $SERVER_CORES $PIN_RR`"
    CLIENT_PIN_RR="`lwords $SERVER_CORES $PIN_RR`"
    unset PIN_RR
fi


export PIN_RR="$SERVER_PIN_RR"
memcached -t $SERVER_CORES -m $MEMORY -u daemon &
server=$!


export PIN_RR="$CLIENT_PIN_RR"
memaslap -s 127.0.0.1:11211 -T $CLIENT_CORES -c $CLIENT_CORES \
	 -t 1s -x $LOAD -S 1h &
client=$!


wait $client
kill $server
