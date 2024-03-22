#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

if [[ -z "$STRACEDIR" ]]; then
    STRACEDIR=meson-logs/strace/$(for i in "$@"; do basename -z -- $i; echo -n _; done).$$
fi

mkdir -p $STRACEDIR

# If the test times out, meson sends SIGTERM to this process.
# Simply exec'ing "time" would result in no output from that in this case.
# Instead, we need to run "time" in the background, catch the signals and
# propagate them to the actual test process.

/usr/bin/time -v strace -ff -tt -T -o $STRACEDIR/log "$@" &
TIMEPID=$!
STRACEPID=$(ps --ppid $TIMEPID -o pid=)
TESTPID=$(ps --ppid $STRACEPID -o pid=)

if test "x$TESTPID" != x; then
    trap 'kill -TERM $TESTPID; wait $TIMEPID; exit $?' TERM
fi

wait $TIMEPID
EXITCODE=$?

# Only keep strace logs if the test timed out
rm -rf $STRACEDIR &

exit $EXITCODE
