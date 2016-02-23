#!/bin/sh
# Script to deploy a ready to use control payload
# Compile the control utilities
# Pack the whole directory into a payload (virtual disk)
set -e


IPATH="`realpath \"${0%/*}\"`"

if [ "x`whoami`" = "xroot" ] ; then
    SUDO=
else
    SUDO=sudo
fi

PAYLOAD=control-payload


if [ ! -e "$PAYLOAD" ] ; then
    echo "$0: missing payload '$PAYLOAD'" >&2
    exit 1
fi


make all -C "$PAYLOAD/param/path"
$SUDO "`realpath \"$PAYLOAD\"`/method/pack"
