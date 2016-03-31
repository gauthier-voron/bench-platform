#!/bin/sh
# Script to deploy a ready to use control payload
# Compile the control utilities
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

PIN=https://github.com/gauthier-voron/pin/archive/master.zip

PAYLOAD=control-payload


if [ ! -e "$PAYLOAD" ] ; then
    echo "$0: missing payload '$PAYLOAD'" >&2
    exit 1
fi


prevpwd="`pwd`"
cd "$PAYLOAD/param/path"

download.sh "$PIN" pin-master.zip
unzip pin-master.zip
rm -rf pin || true 2>/dev/null
mv pin-master pin
rm pin-master.zip
make all -C pin

make all

cd "$prevpwd"

$SUDO "`realpath \"$PAYLOAD\"`/method/pack"

