#!/bin/sh
scan-build -v --status-bugs -plist-html "$@"
