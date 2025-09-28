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
set -x

export DBUS_DEBUG_OUTPUT=1
echo "# dbus-daemon binary: ${DBUS_TEST_DAEMON:=dbus-daemon}"
echo "# dbus-launch binary: ${DBUS_TEST_DBUS_LAUNCH:=dbus-launch}"
echo "# dbus-monitor binary: ${DBUS_TEST_DBUS_LAUNCH:=dbus-monitor}"
echo "# dbus-send binary: ${DBUS_TEST_DBUS_SEND:=dbus-send}"
echo "# dbus-uuidgen binary: ${DBUS_TEST_DBUS_UUIDGEN:=dbus-uuidgen}"

if test -n "$DBUS_TEST_DATA"; then
    echo "# test data: $DBUS_TEST_DATA"
    bus_config="--config-file=$DBUS_TEST_DATA/valid-config-files/session.conf"
    launch_config="$bus_config"
elif test -n "$DBUS_TEST_DATADIR"; then
    echo "# datadir: $DBUS_TEST_DATADIR"
    bus_config="--config-file=$DBUS_TEST_DATADIR/dbus-1/session.conf"
    launch_config="$bus_config"
else
    echo "# using standard session bus configuration"
    bus_config="--session"
    # dbus-launch doesn't accept --session so add a harmless command-line
    # argument
    launch_config="--sh-syntax"
fi

if ! "${DBUS_TEST_DBUS_UUIDGEN}" --get >/dev/null; then
    if test -n "$DBUS_TEST_UNINSTALLED"; then
        echo "1..0 # SKIP - Unable to test dbus-launch without a machine ID"
        exit 0
    else
        echo "Bail out! dbus not correctly installed: no machine ID"
        exit 1
    fi
fi

if ! workdir="$(mktemp -d)"; then
    echo "1..0 # SKIP - mktemp -d doesn't work"
    exit 0
fi

if ! xvfb-run --auto-servernum true; then
    echo "1..0 # SKIP - xvfb-run doesn't work"
    exit 0
fi

test_num=0
x_session_pid=
unset DISPLAY
unset XAUTHORITY

start_xvfb () {
    rm -f "$workdir/display"
    rm -f "$workdir/ready"
    rm -f "$workdir/x-session-pid"
    rm -f "$workdir/xauthority"
    rm -f "$workdir/xvfb-done"

    # Run an X session in the background for up to 5 minutes.
    (
        set +e
        xvfb-run --auto-servernum \
            sh -c 'echo "$$" > "$1/x-session-pid"; echo "$XAUTHORITY" > "$1/xauthority"; echo "$DISPLAY" > "$1/display"; touch "$1/ready"; exec sleep 300' \
            sh "$workdir"
        touch "$workdir/xvfb-done"
    ) &

    tries=0
    while ! [ -e "$workdir/ready" ]; do
        if [ $tries -ge 10 ]; then
            echo "# Failed to start xvfb-run within $tries seconds"
            exit 1
        fi
        tries=$(($tries + 1))
        sleep 1
    done

    x_session_pid="$(cat "$workdir/x-session-pid")"
    export DISPLAY="$(cat "$workdir/display")"
    export XAUTHORITY="$(cat "$workdir/xauthority")"
    kill -0 "$x_session_pid"
}

test_disconnection () {
    rm -f "$workdir/disconnected"
    (
        set +e
        ${DBUS_TEST_DBUS_MONITOR} > "$workdir/dbus-monitor.log" 2>&1
        touch "$workdir/disconnected"
    ) &

    kill "$x_session_pid"

    tries=0
    while ! [ -e "$workdir/disconnected" ]; do
        if [ $tries -ge 10 ]; then
            echo "# dbus-monitor was not disconnected within $tries seconds" >&2
            exit 1
        fi
        tries=$(($tries + 1))
        sleep 1
    done

    tries=0
    while ! [ -e "$workdir/xvfb-done" ]; do
        if [ $tries -ge 10 ]; then
            echo "# xvfb-run did not exit within $tries seconds" >&2
            exit 1
        fi
        tries=$(($tries + 1))
        sleep 1
    done
}

test_exit_with_x11 () {
    arg="$1"
    unset DBUS_SESSION_BUS_ADDRESS
    unset DBUS_SESSION_BUS_PID
    unset DBUS_SESSION_BUS_WINDOWID

    start_xvfb
    eval "$($DBUS_TEST_DBUS_LAUNCH --sh-syntax "$arg" "$launch_config" </dev/null)"

    test -n "$DBUS_SESSION_BUS_ADDRESS"
    env | grep '^DBUS_SESSION_BUS_ADDRESS='

    test -n "$DBUS_SESSION_BUS_PID"
    test "x$(env | grep '^DBUS_SESSION_BUS_PID=')" = "x"
    kill -0 "$DBUS_SESSION_BUS_PID"

    test -n "$DBUS_SESSION_BUS_WINDOWID"
    test "x$(env | grep '^DBUS_SESSION_BUS_WINDOWID=')" = "x"

    ${DBUS_TEST_DBUS_SEND} --session --dest=org.freedesktop.DBus \
        --type=method_call --print-reply / org.freedesktop.DBus.ListNames >&2

    test_disconnection

    test_num=$(($test_num + 1))
    echo "ok ${test_num} - dbus-launch $arg"
}

test_autolaunch () {
    unset DBUS_SESSION_BUS_ADDRESS
    unset DBUS_SESSION_BUS_PID
    unset DBUS_SESSION_BUS_WINDOWID
    unset XDG_RUNTIME_DIR
    fake_uuid="ffffffffffffffffffffffffffffffff"

    start_xvfb
    eval "$($DBUS_TEST_DBUS_LAUNCH --sh-syntax --autolaunch=${fake_uuid} "$launch_config" </dev/null)"

    test -n "$DBUS_SESSION_BUS_ADDRESS"
    env | grep '^DBUS_SESSION_BUS_ADDRESS='

    test -n "$DBUS_SESSION_BUS_PID"
    test "x$(env | grep '^DBUS_SESSION_BUS_PID=')" = "x"
    kill -0 "$DBUS_SESSION_BUS_PID"

    test -n "$DBUS_SESSION_BUS_WINDOWID"
    test "x$(env | grep '^DBUS_SESSION_BUS_WINDOWID=')" = "x"

    # It is idempotent
    old_address="$DBUS_SESSION_BUS_ADDRESS"
    old_pid="$DBUS_SESSION_BUS_PID"
    old_window="$DBUS_SESSION_BUS_WINDOWID"
    eval "$($DBUS_TEST_DBUS_LAUNCH --sh-syntax --autolaunch=${fake_uuid} "$launch_config" </dev/null)"
    test "$DBUS_SESSION_BUS_ADDRESS" = "$old_address"
    test "$DBUS_SESSION_BUS_PID" = "$old_pid"
    test "$DBUS_SESSION_BUS_WINDOWID" = "$old_window"

    ${DBUS_TEST_DBUS_SEND} --session --dest=org.freedesktop.DBus \
        --type=method_call --print-reply / org.freedesktop.DBus.ListNames >&2

    test_disconnection

    test_num=$(($test_num + 1))
    echo "ok ${test_num} - dbus-launch --autolaunch"
}

test_xdg_runtime_dir () {
    unset DBUS_SESSION_BUS_ADDRESS
    unset DBUS_SESSION_BUS_PID
    unset DBUS_SESSION_BUS_WINDOWID
    export XDG_RUNTIME_DIR="$workdir"
    fake_uuid="ffffffffffffffffffffffffffffffff"

    if echo "$workdir" | grep '[^-0-9A-Za-z_/.]'; then
        test_num=$(($test_num + 1))
        echo "ok ${test_num} # SKIP - $workdir would need escaping"
        return
    fi

    ${DBUS_TEST_DAEMON} "$bus_config" --address="unix:path=$XDG_RUNTIME_DIR/bus" --print-pid=9 --fork 9>"$workdir/bus-pid"
    bus_pid="$(cat "$workdir/bus-pid")"
    kill -0 "$bus_pid"

    start_xvfb
    eval "$($DBUS_TEST_DBUS_LAUNCH --sh-syntax --autolaunch=${fake_uuid} "$launch_config" </dev/null)"

    env | grep '^DBUS_SESSION_BUS_ADDRESS='
    test "$DBUS_SESSION_BUS_ADDRESS" = "unix:path=$XDG_RUNTIME_DIR/bus"

    # When dbus-launch picks up the address from XDG_RUNTIME_DIR/bus, it
    # doesn't know the pid
    test -z "$DBUS_SESSION_BUS_PID"

    # It still knows which window it stashed the information in
    test -n "$DBUS_SESSION_BUS_WINDOWID"
    test "x$(env | grep '^DBUS_SESSION_BUS_WINDOWID=')" = "x"

    ${DBUS_TEST_DBUS_SEND} --session --dest=org.freedesktop.DBus \
        --type=method_call --print-reply / org.freedesktop.DBus.ListNames >&2

    kill "$x_session_pid"

    tries=0
    while ! [ -e "$workdir/xvfb-done" ]; do
        if [ $tries -ge 10 ]; then
            echo "# xvfb-run did not exit within $tries seconds" >&2
            exit 1
        fi
        tries=$(($tries + 1))
        sleep 1
    done

    # dbus-launch has gone but the dbus-daemon is still alive
    ${DBUS_TEST_DBUS_SEND} --session --dest=org.freedesktop.DBus \
        --type=method_call --print-reply / org.freedesktop.DBus.ListNames >&2

    kill "$bus_pid"

    test_num=$(($test_num + 1))
    echo "ok ${test_num} - dbus-launch --autolaunch with XDG_RUNTIME_DIR"
}

echo "1..4"
test_exit_with_x11 --exit-with-session
test_exit_with_x11 --exit-with-x11
test_autolaunch
test_xdg_runtime_dir

# vim:set sw=4 sts=4 et:
