#!/bin/bash

set -m

if `which gnome-screensaver-command`; then
	gnome-screensaver-command -i -n "cairo-test-suite" -r "Cairo needs to read back from the screen in order to test rendering to xlib" &
	pid=$!

	restore_screensaver() { kill $pid; }
else
	restore_screensaver() { :; }
fi

trap cleanup SIGINT SIGTERM

./cairo-test-suite "$*"

restore_screensaver
