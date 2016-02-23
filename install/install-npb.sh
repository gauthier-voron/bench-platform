#!/bin/sh
# Script to deploy a ready to use the Nas Parallel Benchmark
# Download the sources from the official site (v3.3.1)
# Apply patches to the config files
# Compile the sources into executable binary files
# Pack the whole directory into a payload (virtual disk)
set -e


IPATH="`realpath \"${0%/*}\"`"
PATH="$IPATH":"$PATH"
DATA="$IPATH/data"

if [ "x`whoami`" = "xroot" ] ; then
    SUDO=
else
    SUDO=sudo
fi

DOWNLOAD=www.nas.nasa.gov/assets/npb/NPB3.3.1.tar.gz
MD5SUM=8e5ec2c819480759725df67833619911
PAYLOAD=npb-payload


if [ -e "$PAYLOAD" ] ; then
    echo "$0: payload '$PAYLOAD' already exists" >&2
    exit 1
fi


fakeroot="`mktemp -d install-npb-fakeroot.XXXXXX`"
prevpwd="`pwd`"
cd "$fakeroot"

download.sh "$DOWNLOAD" npb.tar.gz $MD5SUM
tar -xzf npb.tar.gz

cp "$DATA/NPB3.3.1-OMP-make.def" "NPB3.3.1/NPB3.3-OMP/config/make.def"

make BT CLASS=C -C "NPB3.3.1/NPB3.3-OMP"
make CG CLASS=C -C "NPB3.3.1/NPB3.3-OMP"
make DC CLASS=B -C "NPB3.3.1/NPB3.3-OMP"
make EP CLASS=D -C "NPB3.3.1/NPB3.3-OMP"
make FT CLASS=C -C "NPB3.3.1/NPB3.3-OMP"
make LU CLASS=C -C "NPB3.3.1/NPB3.3-OMP"
make MG CLASS=D -C "NPB3.3.1/NPB3.3-OMP"
make SP CLASS=C -C "NPB3.3.1/NPB3.3-OMP"
make UA CLASS=C -C "NPB3.3.1/NPB3.3-OMP"

cd "$prevpwd"


cp -R "$DATA/base-payload" "$PAYLOAD"
echo ext4 > "$PAYLOAD/param/fs"
echo 40G > "$PAYLOAD/param/size"
rmdir "$PAYLOAD/param/path"
mv "$fakeroot/NPB3.3.1" "$PAYLOAD/param/path"


$SUDO "`realpath \"$PAYLOAD\"`/method/pack"

rm -rf "$fakeroot"
