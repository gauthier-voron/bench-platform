#!/bin/sh

pkg="$1" ; shift
time="$1" ; shift

if [ "x$time" != "x" ] ; then
    sleep $time
fi
