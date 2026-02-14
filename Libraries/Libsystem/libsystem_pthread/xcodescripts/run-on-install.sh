#!/bin/bash

if [[ "x${ACTION}" == "xinstall" && "x${SKIP_INSTALL}" == "xNO" ]]; then
	$@
else
	exit 0
fi
