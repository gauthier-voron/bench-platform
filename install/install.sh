#!/bin/sh
# Install the necessary files for the benchmark platform to be usable
# Install the control-payload
# Install the null-payload
# Install the npb-payload
# Install the parsec-payload
# Install the vm-hvm-base

IPATH="`realpath \"${0%/*}\"`"
PATH="$IPATH":"$PATH"

install-control.sh
install-null.sh
install-npb.sh
install-parsec.sh
install-mosbench.sh
install-tpce.sh
install-vm-hvm-base.sh
