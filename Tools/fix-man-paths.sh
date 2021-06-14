#!/bin/sh
# Fix man paths for ports installed into /usr
find $1 -name pkg-plist -exec sed -i_ -e 's/^man/share\/man/' -e 's/^%%MANPAGES%%man/%%MANPAGES%%share\/man/' {} \;

