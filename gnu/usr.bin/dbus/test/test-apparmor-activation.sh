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

if [ "x$(id -u)" != "x0" ]; then
    echo "1..0 # SKIP - this test can only be run as root"
    exit 0
fi

if [ -z "$DBUS_TEST_EXEC" ]; then
    DBUS_TEST_EXEC="$(dirname "$0")"
    if ! [ -x "$DBUS_TEST_EXEC/test-apparmor-activation" ]; then
        echo "1..0 # SKIP - executable not found and DBUS_TEST_EXEC not set"
        exit 0
    fi
fi

if [ -z "$DBUS_TEST_DATA" ]; then
    DBUS_TEST_DATA="$DBUS_TEST_EXEC/data"
    if ! [ -e "$DBUS_TEST_DATA/dbus-installed-tests.aaprofile" ]; then
        echo "1..0 # SKIP - required data not found and DBUS_TEST_DATA not set"
        exit 0
    fi
fi

echo "# Attempting to load AppArmor profiles"
if ! apparmor_parser --skip-cache --replace \
        "$DBUS_TEST_DATA/dbus-installed-tests.aaprofile"; then
    echo "1..0 # SKIP - unable to load AppArmor profiles"
    exit 0
fi

exec "$DBUS_TEST_EXEC/test-apparmor-activation" --tap "$@"

# vim:set sts=4 sw=4 et:
