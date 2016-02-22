#!/bin/sh

bench="$1" ; shift
interl="$1" ; shift


mem=`free -b | head -n 3 | tail -n 1 \
     | sed -e 's/^.*:[[:space:]]*//' \
           -e 's/[[:space:]][[:space:]]*/ /g' \
     | cut -d' ' -f2`

if [ "x$interl" != "x" ] ; then
    interl=",mpol=interleave:$interl"
fi

mkdir tempfs
mount -t tmpfs tmpfs tempfs -o "size=$mem$interl"
rsync -aAHX "$bench/" "tempfs/"

if mountpoint -q "$bench" ; then
    umount "$bench/"
    rmdir "$bench"
else
    mv "$bench" "$bench.old"
fi
ln -s "tempfs" "$bench"

sync
