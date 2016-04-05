#!/bin/sh
# Script to deploy a ready to use HVM Xen Domain
# Download and untar the image of the root disk.
set -e


IPATH="`realpath \"${0%/*}\"`"
PATH="$IPATH":"$PATH"

DOWNLOAD=www.mnesic.fr/bigos/vm-xenhvm-linux3.9-replication+.xvda.img.gz
MD5SUM=30befdc1fb56bd7c1b052e77d735c61f
VMIMAGE=vm-hvm-base/private/xvda.img


if [ -e "$VMIMAGE" ] ; then
    echo "$0: vm image '$VMIMAGE' already exists" >&2
    exit 1
fi


fakeroot="`mktemp -d install-vm-hvm-base-xvda-fakeroot.XXXXXX`"
prevpwd="`pwd`"
cd "$fakeroot"

download.sh "$DOWNLOAD" xvda.img.gz $MD5SUM
gunzip xvda.img.gz

cd "$prevpwd"

mkdir -p "${VMIMAGE%/*}"
mv "$fakeroot/xvda.img" "$VMIMAGE"
rm -rf "$fakeroot"
