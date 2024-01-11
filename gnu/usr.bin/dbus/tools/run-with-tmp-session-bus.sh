#! /bin/sh

SCRIPTNAME="$0"
WRAPPED_SCRIPT="$1"
shift

if test -z "$DBUS_TEST_CONFIG_FILE"; then
    DBUS_TEST_CONFIG_FILE="$DBUS_TOP_BUILDDIR/test/data/valid-config-files/tmp-session.conf"
fi

die ()
{
    echo "$SCRIPTNAME: $*" >&2
    exit 1
}

if test -z "$DBUS_TOP_BUILDDIR" ; then
    die "Must set DBUS_TOP_BUILDDIR"
fi

if ! test -e "$DBUS_TOP_BUILDDIR"/bus/dbus-daemon ; then
    die "$DBUS_TOP_BUILDDIR/bus/dbus-daemon does not exist"
fi

PATH="$DBUS_TOP_BUILDDIR"/bus:$PATH
export PATH

## the libtool script found by the path search should already do this, but
LD_LIBRARY_PATH=$DBUS_TOP_BUILDDIR/dbus/.libs:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH
unset DBUS_SESSION_BUS_ADDRESS
unset DBUS_SESSION_BUS_PID

$DBUS_TOP_BUILDDIR/tools/dbus-run-session \
    --config-file="$DBUS_TEST_CONFIG_FILE" \
    --dbus-daemon="$DBUS_TOP_BUILDDIR/bus/dbus-daemon" \
    -- \
    "$WRAPPED_SCRIPT" "$@"
error=$?

# clean up
exit $error
