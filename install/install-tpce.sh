#!/bin/sh
# Script to deploy a ready to use TPC-E benchmark implementation (DBT-5)
# Download the sources from the official site
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

POSTGRESQL=https://ftp.postgresql.org/pub/source/v9.5.1/postgresql-9.5.1.tar.gz
POSTGRESQL_MD5SUM=615aa29d0c58c1968a2b97178f271ca3
DBT5=https://github.com/gauthier-voron/DBT-5/archive/master.zip

PAYLOAD=tpce-payload


if [ -e "$PAYLOAD" ] ; then
    echo "$0: payload '$PAYLOAD' already exists" >&2
    exit 1
fi


fakeroot="`mktemp -d install-tpce-fakeroot.XXXXXX`"
prevpwd="`pwd`"
cd "$fakeroot"

download.sh "$POSTGRESQL" postgresql-9.5.1.tar.gz "$POSTGRESQL_MD5SUM"
tar -xzf postgresql-9.5.1.tar.gz

download.sh "$DBT5" DBT-5-master.zip
unzip DBT-5-master.zip

mkdir payload

mkdir payload/postgresql
pginstall="`pwd`/payload/postgresql"
(
    cd postgresql-9.5.1
    ./configure --without-readline --prefix="$pginstall"
    make all -j $CORES
    make install
)

mkdir payload/dbt5
(
    cd DBT-5-master
    export CPATH="$pginstall/include":"$CPATH"
    export LD_LIBRARY_PATH="$pginstall/lib":"$LD_LIBRARY_PATH"
    export PATH="$pginstall/bin":"$PATH"
    cmake CMakeLists.txt -DDBMS=pgsql
    cd egen/prj
    make -f Makefile.pgsql -j $CORES
    cd ../../storedproc/pgsql/c
    make -j $CORES
    cd ../../..
    make -j $CORES
)
for file in DBT-5-master/bin/* ; do
    mv "$file" payload/dbt5
done
for file in DBT-5-master/scripts/dbt5-* ; do
    mv "$file" payload/dbt5
done
for file in DBT-5-master/scripts/pgsql/* ; do
    mv "$file" payload/dbt5
done
mv DBT-5-master/egen payload/dbt5

cd "$prevpwd"


cp -R "$DATA/base-payload" "$PAYLOAD"
echo ext4 > "$PAYLOAD/param/fs"
echo 10G > "$PAYLOAD/param/size"
rm -rf "$PAYLOAD/param/path"
mv "$fakeroot/payload" "$PAYLOAD/param/path"

rm -rf "$fakeroot"
