#!/bin/sh
# Script to deploy a ready to use HVM Xen Domain
# Download and untar the image of the root disk.
set -e


IPATH="`realpath \"${0%/*}\"`"
PATH="$IPATH":"$PATH"

DOWNLOAD=www.mnesic.fr/bigos/vm-xenhvm-linux3.9-replication+.tar.gz
MD5SUM=6ce8cee5c0aecc48a2573cb67974d9a9
VMBASE=vm-hvm-base


if [ -e "$VMBASE" ] ; then
    echo "$0: vm image '$VMBASE' already exists" >&2
    exit 1
fi


fakeroot="`mktemp -d install-vm-hvm-base-fakeroot.XXXXXX`"
prevpwd="`pwd`"
cd "$fakeroot"

download.sh "$DOWNLOAD" image.tar.gz $MD5SUM
tar -xzf image.tar.gz

cd "$prevpwd"

mv "$fakeroot/vm-xenhvm-linux3.9-replication+" "$VMBASE"
rm -rf "$fakeroot"
