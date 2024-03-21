#!/bin/bash

ls /run/udev/rules.d/*litest*.rules 2>/dev/null
if [[ $? -eq 0 ]]; then
	exit 1
fi

ls /etc/udev/hwdb.d/*litest*REMOVEME*.hwdb 2>/dev/null
if [[ $? -eq 0 ]]; then
	exit 1
fi
