#!/bin/sh

# If the test times out, meson sends SIGTERM to this process.
# Simply exec'ing "time" would result in no output from that in this case.
# Instead, we need to run "time" in the background, catch the signals and
# propagate them to the actual test process.

/usr/bin/time -v "$@" &
TIMEPID=$!
TESTPID=$(ps --ppid $TIMEPID -o pid=)

if test "x$TESTPID" != x; then
    trap 'kill -TERM $TESTPID; wait $TIMEPID; exit $?' TERM
fi

wait $TIMEPID
exit $?
