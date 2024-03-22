#!/bin/sh

set -ex

_XORG_SCRIPT="/xorg-script"
_FLAG_FILE="/xorg-started"

echo "touch ${_FLAG_FILE}; sleep 100000" > "${_XORG_SCRIPT}"
if [ "x$1" != "x" ]; then
    export LD_LIBRARY_PATH="${1}/lib"
    export LIBGL_DRIVERS_PATH="${1}/lib/dri"
fi
xinit /bin/sh "${_XORG_SCRIPT}" -- /usr/bin/Xorg vt45 -noreset -s 0 -dpms -logfile /Xorg.0.log &

# Wait for xorg to be ready for connections.
for _ in 1 2 3 4 5; do
    if [ -e "${_FLAG_FILE}" ]; then
        break
    fi
    sleep 5
done
