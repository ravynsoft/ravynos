#!/bin/sh

pacman --noconfirm -Syu

prefix=
if [ -n "$MINGW_PACKAGE_PREFIX" ]; then
    prefix="${MINGW_PACKAGE_PREFIX}-"
fi
for module in libiconv python xz zlib "$@"; do
    pacman --noconfirm -S --needed ${prefix}$module
done

if [ ! -e libxml2-build/xmlconf ]; then
    mkdir -p libxml2-build
    wget https://www.w3.org/XML/Test/xmlts20080827.tar -O - |
       tar -x -C libxml2-build
fi
