#! /bin/sh

SCRIPTNAME=$0
MODE=$1

## so the tests can complain if you fail to use the script to launch them
DBUS_TEST_NAME_RUN_TEST_SCRIPT=1
export DBUS_TEST_NAME_RUN_TEST_SCRIPT

# Rerun ourselves with tmp session bus if we're not already
if test -z "$DBUS_TEST_NAME_IN_RUN_TEST"; then
  DBUS_TEST_NAME_IN_RUN_TEST=1
  export DBUS_TEST_NAME_IN_RUN_TEST
  exec $DBUS_TOP_SRCDIR/tools/run-with-tmp-session-bus.sh $SCRIPTNAME $MODE
fi 

if test -n "$DBUS_TEST_MONITOR"; then
  dbus-monitor --session >&2 &
fi

XDG_RUNTIME_DIR="$DBUS_TOP_BUILDDIR"/test/XDG_RUNTIME_DIR
test -d "$XDG_RUNTIME_DIR" || mkdir "$XDG_RUNTIME_DIR"
chmod 0700 "$XDG_RUNTIME_DIR"
export XDG_RUNTIME_DIR

# Translate a command and exit status into TAP syntax.
# Usage: interpret_result $? description-of-test
# Uses global variable $test_num.
interpret_result () {
  e="$1"
  shift
  case "$e" in
    (0)
      echo "ok $test_num $*"
      ;;
    (77)
      echo "ok $test_num # SKIP $*"
      ;;
    (*)
      echo "not ok $test_num $*"
      ;;
  esac
  test_num=$(( $test_num + 1 ))
}

c_test () {
  t="$1"
  shift
  e=0
  echo "# running test $t"
  "${DBUS_TOP_BUILDDIR}/libtool" --mode=execute $DEBUG "$DBUS_TOP_BUILDDIR/test/name-test/$t" "$@" >&2 || e=$?
  echo "# exit status $e"
  interpret_result "$e" "$t" "$@"
}

test_num=1
# TAP test plan: we will run 1 test
echo "1..1"

c_test test-autolaunch
