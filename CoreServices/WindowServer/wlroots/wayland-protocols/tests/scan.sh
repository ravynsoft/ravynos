#!/bin/sh -e

if [ "x$SCANNER" = "x" ] ; then
	echo "No scanner present, test skipped." 1>&2
	exit 77
fi

$SCANNER client-header --strict $1 /dev/null
$SCANNER server-header --strict $1 /dev/null
$SCANNER private-code --strict $1 /dev/null
$SCANNER public-code --strict $1 /dev/null
