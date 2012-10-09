#!/bin/bash
#
# hack to find SUSE kernel flavour of the running kernel
read a b V d < /proc/version
V=${V##*-}

make -C /usr/src/linux-obj/x86_64/$V M=$PWD $@
