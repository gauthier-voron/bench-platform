#!/bin/sh
# Script to deploy a ready to use the Parsec Benchmark
# Download the sources from the official site (v2.1)
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

ADDR="parsec.cs.princeton.edu/download/2.1"
BINARIES="$ADDR/binaries/parsec-2.1-amd64-linux.tar.gz"
BSUM=835f0dc9aa45ce7dac592cf5e80f9799
INPUTS="$ADDR/parsec-2.1-native.tar.gz"
ISUM=3186b36b86c9754b58e1d925a034340e
CORE="$ADDR/parsec-2.1-core.tar.gz"
CSUM=7da0d2b85eda583890b1d9716ab401d5
PAYLOAD=parsec-payload


if [ -e "$PAYLOAD" ] ; then
    echo "$0: payload '$PAYLOAD' already exists" >&2
    exit 1
fi


fakeroot="`mktemp -d install-parsec-fakeroot.XXXXXX`"
prevpwd="`pwd`"
cd "$fakeroot"

download.sh "$CORE" parsec-core.tar.gz $CSUM
download.sh "$BINARIES" parsec-binaries.tar.gz $BSUM
download.sh "$INPUTS" parsec-inputs.tar.gz $ISUM

tar -xzf parsec-core.tar.gz
tar -xzf parsec-binaries.tar.gz
tar -xzf parsec-inputs.tar.gz

cd "$prevpwd"


cp -R "$DATA/base-payload" "$PAYLOAD"
echo ext4 > "$PAYLOAD/param/fs"
echo 40G > "$PAYLOAD/param/size"
rm -rf "$PAYLOAD/param/path"
mv "$fakeroot/parsec-2.1" "$PAYLOAD/param/path"


$SUDO "`realpath \"$PAYLOAD\"`/method/pack"

rm -rf "$fakeroot"
