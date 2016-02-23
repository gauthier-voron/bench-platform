#!/bin/sh
# Script to deploy a ready to use null payload
# Pack the whole directory into a payload (virtual disk)
set -e


IPATH="`realpath \"${0%/*}\"`"

if [ "x`whoami`" = "xroot" ] ; then
    SUDO=
else
    SUDO=sudo
fi

PAYLOAD=null-payload


if [ ! -e "$PAYLOAD" ] ; then
    echo "$0: missing payload '$PAYLOAD'" >&2
    exit 1
fi


$SUDO "`realpath \"$PAYLOAD\"`/method/pack"
