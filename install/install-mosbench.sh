#!/bin/sh
# Script to deploy a ready to use some applications of the Mosbench
# Download the sources from the official site
# Apply patches to the config files
# Compile the sources into executable binary files
# Pack the whole directory into a payload (virtual disk)
set -e

IPATH="`realpath \"${0%/*}\"`"
PATH="$IPATH":"$PATH"
DATA="$IPATH/data"
CORES=`grep '^processor' /proc/cpuinfo | wc -l`

if [ "x`whoami`" = "xroot" ] ; then
    SUDO=
else
    SUDO=sudo
fi

MOSBENCH=https://pdos.csail.mit.edu/mosbench/mosbench.git
MOSBENCH_MD5SUM=dddc8d34b3a99293e417d23e6b5a8d2d

LIBMEMCACHED=https://launchpad.net/libmemcached/1.0/1.0.18/+download/libmemcached-1.0.18.tar.gz
LIBMEMCACHED_MD5SUM=b3958716b4e53ddc5992e6c49d97e819

PAYLOAD=mosbench-payload


if [ -e "$PAYLOAD" ] ; then
    echo "$0: payload '$PAYLOAD' already exists" >&2
    exit 1
fi


fakeroot="`mktemp -d install-mosbench-fakeroot.XXXXXX`"
prevpwd="`pwd`"
cd "$fakeroot"

download.sh --git "$MOSBENCH" mosbench $MOSBENCH_MD5SUM

download.sh "$LIBMEMCACHED" libmemcached-1.0.18.tar.gz $LIBMEMCACHED_MD5SUM
tar -xzf libmemcached-1.0.18.tar.gz

mkdir payload

mkdir payload/metis
(
    cd mosbench/metis
    patch app/pca.c "$DATA"/pca.c.patch
    make all -j $CORES
)
for app in matrix_mult wrmem kmeans wc wr pca ; do
    mv mosbench/metis/obj/app/$app payload/metis
done

mkdir payload/psearchy
(
    cd mosbench/psearchy
    make -C mkdb
    make -C query
)
mv mosbench/psearchy/mkdb/pedsort payload/psearchy
mv mosbench/psearchy/query/qe payload/psearchy

mkdir payload/memcached
(
    cd mosbench/memcached/memcached-1.4.5/
    ./configure
    for conf in configure config.status configure.ac ; do
	sed -ri 's/-Werror//g' $conf
    done
    make all -j $CORES
)
(
    cd libmemcached-1.0.18
    ./configure --enable-memaslap
    make all
)
mv mosbench/memcached/memcached-1.4.5/memcached payload/memcached
mv libmemcached-1.0.18/clients/memaslap payload/memcached
mv libmemcached-1.0.18/clients/memstat payload/memcached

mkdir payload/words
generate.pl -t $(( $CORES * 2 )) \
	    -w 100000 -m text -f 1000 -p payload/words/text-file-%d.txt
cat payload/words/text-file-*.txt > payload/words/text-file.txt

cd "$prevpwd"


cp -R "$DATA/base-payload" "$PAYLOAD"
echo ext4 > "$PAYLOAD/param/fs"
echo 10G > "$PAYLOAD/param/size"
rm -rf "$PAYLOAD/param/path"
mv "$fakeroot/payload" "$PAYLOAD/param/path"

rm -rf "$fakeroot"
