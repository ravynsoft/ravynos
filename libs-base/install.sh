#! /bin/sh

PREFIX=$1
MAKE=${2-make}

. $PREFIX/System/Library/Makefiles/GNUstep.sh

$MAKE GNUSTEP_INSTALLATION_DOMAIN=SYSTEM messages=yes install

exit 0
