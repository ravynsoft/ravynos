#!/bin/sh

# Copyright © 2017 Collabora Ltd.
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

if test -z "$XDG_RUNTIME_DIR"; then
    echo "1..0 # SKIP - $XDG_RUNTIME_DIR is empty or unset"
    exit 0
fi

# test -O is non-POSIX, but bash and dash have it. If it doesn't work,
# we're probably not on Linux and so would likely be skipping this test
# anyway.
if ! test -O "$XDG_RUNTIME_DIR"; then
    echo "1..0 # SKIP - $XDG_RUNTIME_DIR is not ours or test -O does not work"
    exit 0
fi

if ! test -S "$XDG_RUNTIME_DIR/bus"; then
    echo "1..0 # SKIP - $XDG_RUNTIME_DIR/bus is not a socket"
    exit 0
fi

if ! test -O "$XDG_RUNTIME_DIR/bus"; then
    echo "1..0 # SKIP - $XDG_RUNTIME_DIR/bus is not ours"
    exit 0
fi

if test -n "$DBUS_SESSION_BUS_ADDRESS" && \
        ! test "unix:path=$XDG_RUNTIME_DIR/bus" = "$DBUS_SESSION_BUS_ADDRESS"; then
    echo "1..0 # SKIP - DBUS_SESSION_BUS_ADDRESS does not point to $XDG_RUNTIME_DIR/bus"
    exit 0
fi

# Any unique token that won't collide would do here. It must start with
# a letter or underscore.
unique="t$(dbus-uuidgen)"

if ! workdir="$(mktemp -d)"; then
    echo "1..0 # SKIP - mktemp -d doesn't work"
    exit 0
fi

cleanup () {
    rm -fr "$workdir"
    rm -f "$XDG_RUNTIME_DIR/dbus-1/services/com.example.DBusTests.$unique.tmp"
    rm -f "$XDG_RUNTIME_DIR/dbus-1/services/com.example.DBusTests.$unique.service"
    rm -f "$XDG_RUNTIME_DIR/dbus-1/services/com.example.DBusTests.Systemd.$unique.tmp"
    rm -f "$XDG_RUNTIME_DIR/dbus-1/services/com.example.DBusTests.Systemd.$unique.service"
    rm -f "$XDG_RUNTIME_DIR/systemd/user/dbus-com.example.DBusTests.Systemd.$unique.service"
}
trap cleanup EXIT

echo "1..2"

# If the dbus-daemon is launched on-demand by a systemd socket unit, it
# might not be there yet, even if the socket is
(
dbus-send --session --dest="org.freedesktop.DBus" \
    --type=method_call --print-reply /org/freedesktop/DBus \
    org.freedesktop.DBus.Peer.Ping || touch "$workdir/failed" \
) 2>&1 | sed -e 's/^/# /'

if [ -e "$workdir/failed" ]; then
    echo "Bail out! Unable to ensure dbus-daemon has started"
    exit 1
fi

if ! test -d "$XDG_RUNTIME_DIR/dbus-1/services"; then
    echo "Bail out! $XDG_RUNTIME_DIR/dbus-1/services is not a directory"
    exit 1
fi

cd "$XDG_RUNTIME_DIR/dbus-1/services"

sed -e 's/^ *//' > "com.example.DBusTests.$unique.tmp" <<EOF
    [D-BUS Service]
    Name=com.example.DBusTests.$unique
    Exec=/bin/sh -c 'touch "\$1"; exit 1' sh '$workdir/ran-$unique'
EOF
mv "com.example.DBusTests.$unique.tmp" "com.example.DBusTests.$unique.service"

(
dbus-send --session --dest="org.freedesktop.DBus" \
    --type=method_call --print-reply /org/freedesktop/DBus \
    org.freedesktop.DBus.ReloadConfig || touch "$workdir/failed" \
) 2>&1 | sed -e 's/^/# /'

if [ -e "$workdir/failed" ]; then
    echo "Bail out! Unable to tell dbus-daemon to reload"
    exit 1
fi

# Because the service never actually takes a bus name, we expect this to fail;
# but its exit status is ignored because it's in a pipeline.
dbus-send --session --dest="com.example.DBusTests.$unique" \
    --type=method_call --print-reply / com.example.DBusTests.ThisWillFail \
    2>&1 | sed -e 's/^/# /'

if [ -e "$workdir/ran-$unique" ]; then
    echo "ok 1 - attempted to run transient service directly"
else
    echo "not ok 1 - no sign of having run transient service"
fi

# See whether systemd is on the session bus.
(
dbus-send --session --dest="org.freedesktop.systemd1" \
    --type=method_call --print-reply /org/freedesktop/systemd1/Manager \
    org.freedesktop.DBus.Peer.Ping || touch "$workdir/no-systemd" \
) 2>&1 | sed -e 's/^/# /'

if [ -d "$XDG_RUNTIME_DIR/systemd" ] && ! [ -e "$workdir/no-systemd" ]; then

    # systemd is Linux-specific, so we can assume GNU mkdir.
    mkdir -p -m700 "$XDG_RUNTIME_DIR/systemd/user"
    cd "$XDG_RUNTIME_DIR/systemd/user"

    sed -e 's/^ *//' > "dbus-com.example.DBusTests.Systemd.$unique.tmp" <<EOF
        [Unit]
        Description=Non-functional service in the dbus integration tests
        [Service]
        ExecStart=/bin/sh -c 'touch "\$1"; exit 1' sh '$workdir/ran-$unique-via-systemd'
        Type=forking
EOF
    mv "dbus-com.example.DBusTests.Systemd.$unique.tmp" \
        "dbus-com.example.DBusTests.Systemd.$unique.service"

    if ! systemctl --user daemon-reload; then
        echo "Bail out! Unable to tell systemd to reload"
        exit 1
    fi

    cd "$XDG_RUNTIME_DIR/dbus-1/services"

    sed -e 's/^ *//' > "com.example.DBusTests.Systemd.$unique.tmp" <<EOF
        [D-BUS Service]
        Name=com.example.DBusTests.Systemd.$unique
        SystemdService=dbus-com.example.DBusTests.Systemd.$unique.service
        Exec=/bin/sh -c 'touch "\$1"; exit 1' sh '$workdir/ran-$unique-wrong'
EOF
    mv "com.example.DBusTests.Systemd.$unique.tmp" \
        "com.example.DBusTests.Systemd.$unique.service"

    (
    dbus-send --session --dest="org.freedesktop.DBus" \
        --type=method_call --print-reply /org/freedesktop/DBus \
        org.freedesktop.DBus.ReloadConfig || touch "$workdir/failed" \
    ) 2>&1 | sed -e 's/^/# /'

    if [ -e "$workdir/failed" ]; then
        echo "Bail out! Unable to tell dbus-daemon to reload"
        exit 1
    fi

    # Because the service never actually takes a bus name, we expect this to fail;
    # but its exit status is ignored because it's in a pipeline.
    #
    # systemd never sends back an ActivationFailure message for a service
    # if it starts the job but the service subsequently doesn't behave as
    # intended, so use a relatively short timeout (3000ms).
    dbus-send --session --dest="com.example.DBusTests.Systemd.$unique" \
        --reply-timeout=3000 --type=method_call --print-reply \
        / com.example.DBusTests.ThisWillFail \
        2>&1 | sed -e 's/^/# /'

    if [ -e "$workdir/ran-$unique-via-systemd" ]; then
        echo "ok 2 - attempted to run transient service via systemd"
    elif [ -e "$workdir/ran-$unique-wrong" ]; then
        echo "not ok 2 - ran transient service incorrectly"
    else
        echo "not ok 2 - no sign of having run transient service"
    fi
else
    echo "ok 2 # SKIP session bus does not appear to be managed by systemd"
fi

echo "# Done."
cleanup
trap '' EXIT
exit 0
