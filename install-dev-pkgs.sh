#!/bin/sh
#
# Helper script to install additional packages needed to compile
# Helium on Helium image
#

pkg install gmake autoconf automake libtool cmake gperf meson autopoint intltool \
	libtalloc libgdbm libmtdev libudev
