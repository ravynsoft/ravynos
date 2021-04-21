#!/bin/sh
#
# Helper script to install additional packages needed to compile
# Helium on Helium image
#

pkg install gmake autoconf automake libtool pkgconf nasm \
	cmake gperf meson intltool bison docbook-xml llvm10 \
	talloc libmtdev gettext-tools py37-mako docbook-xsl

ln -sf /usr/local/bin/llvm-config10 /usr/bin
ln -sf /usr/local/bin/cmake /usr/bin
ln -sf /usr/local/bin/autoreconf-2.69 /usr/bin
ln -sf /usr/local/bin/autoreconf /usr/bin
