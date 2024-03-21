#!/bin/sh

# Copyright © 2016 Collabora Ltd.
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

set -e

export DBUS_DEBUG_OUTPUT=1
echo "# dbus-daemon binary: ${DBUS_TEST_DAEMON:=dbus-daemon}"
echo "# dbus-send binary: ${DBUS_TEST_DBUS_SEND:=dbus-send}"

if test -n "$DBUS_TEST_DATA"; then
    echo "# test data: $DBUS_TEST_DATA"
    config="--config-file=$DBUS_TEST_DATA/valid-config-files/session.conf"
elif test -n "$DBUS_TEST_DATADIR"; then
    echo "# datadir: $DBUS_TEST_DATADIR"
    config="--config-file=$DBUS_TEST_DATADIR/dbus-1/session.conf"
else
    echo "# using standard session bus configuration"
    config="--session"
fi

if ! workdir="$(mktemp -d)"; then
    echo "1..0 # SKIP - mktemp -d doesn't work"
    exit 0
fi

echo "1..3"

unset DBUS_SESSION_BUS_ADDRESS
unset DBUS_SESSION_BUS_PID
rm -f "$workdir"/address "$workdir"/pid

${DBUS_TEST_DAEMON} --fork --print-address=8 --print-pid=9 "$config" \
    8>"$workdir/address" 9>"$workdir/pid"

export DBUS_SESSION_BUS_ADDRESS="$(cat "$workdir"/address)"
test -n "$DBUS_SESSION_BUS_ADDRESS"
DBUS_SESSION_BUS_PID="$(cat "$workdir"/pid)"
kill -0 "$DBUS_SESSION_BUS_PID"

${DBUS_TEST_DBUS_SEND} --session --dest=org.freedesktop.DBus \
    --type=method_call --print-reply / org.freedesktop.DBus.ListNames >&2

kill "$DBUS_SESSION_BUS_PID"

echo "ok 1 - normal dbus-daemon"

unset DBUS_SESSION_BUS_ADDRESS
unset DBUS_SESSION_BUS_PID
rm -f "$workdir"/address "$workdir"/pid

${DBUS_TEST_DAEMON} --fork --print-address=8 --print-pid=9 "$config" \
    8>"$workdir/address" 9>"$workdir/pid" <&-

export DBUS_SESSION_BUS_ADDRESS="$(cat "$workdir"/address)"
test -n "$DBUS_SESSION_BUS_ADDRESS"
DBUS_SESSION_BUS_PID="$(cat "$workdir"/pid)"
kill -0 "$DBUS_SESSION_BUS_PID"

${DBUS_TEST_DBUS_SEND} --session --dest=org.freedesktop.DBus \
    --type=method_call --print-reply / org.freedesktop.DBus.ListNames >&2

kill "$DBUS_SESSION_BUS_PID"

echo "ok 2 - dbus-daemon with stdin closed"

unset DBUS_SESSION_BUS_ADDRESS
unset DBUS_SESSION_BUS_PID
rm -f "$workdir"/address "$workdir"/pid

${DBUS_TEST_DAEMON} --fork --print-address=8 --print-pid=9 "$config" \
    8>"$workdir/address" 9>"$workdir/pid" <&- >&- 2>&-

export DBUS_SESSION_BUS_ADDRESS="$(cat "$workdir"/address)"
test -n "$DBUS_SESSION_BUS_ADDRESS"
DBUS_SESSION_BUS_PID="$(cat "$workdir"/pid)"
kill -0 "$DBUS_SESSION_BUS_PID"

${DBUS_TEST_DBUS_SEND} --session --dest=org.freedesktop.DBus \
    --type=method_call --print-reply / org.freedesktop.DBus.ListNames >&2

kill "$DBUS_SESSION_BUS_PID"

echo "ok 3 - dbus-daemon with stdin, stdout, stderr closed"

rm -r "$workdir"
