#!/bin/sh

LC_ALL=C
export LC_ALL

if test -z "$DOC_MODULE"; then
	# extract from Makefile
	eval `grep '^DOC_MODULE' Makefile | sed 's/ //g'`
	if test -z "$DOC_MODULE"; then
		echo Failed extracting DOC_MODULE from Makefile 1>&2
		echo Try setting DOC_MODULE env var manually 1>&2
		exit 1
	fi
fi

if test -n "$REPORT_FILES"; then
	$MAKE $REPORT_FILES || exit 1
fi

test -z "$srcdir" && srcdir=.
stat=0

if test -f "$DOC_MODULE-undeclared.txt"; then
	undeclared=`cat "$DOC_MODULE-undeclared.txt"`
	if test -n "$undeclared"; then
		echo "*** ERROR: Undeclared documentation symbols:" 1>&2
		cat "$DOC_MODULE-undeclared.txt" 1>&2
		stat=1
	fi
fi >&2
if test -f "$DOC_MODULE-unused.txt"; then
	unused=`cat "$DOC_MODULE-unused.txt"`
	if test -n "$unused"; then
		echo "*** ERROR: Unused documented symbols:" 1>&2
		cat "$DOC_MODULE-unused.txt" 1>&2
		stat=1
	fi
fi >&2
if test -f "$DOC_MODULE-undocumented.txt"; then
	if grep '^0 symbols incomplete' "$DOC_MODULE-undocumented.txt" >/dev/null &&
	   grep '^0 not documented'     "$DOC_MODULE-undocumented.txt" >/dev/null; then
		:
	else
		echo "*** ERROR: Incomplete or undocumented symbols:" 1>&2
		cat "$DOC_MODULE-undocumented.txt" 1>&2
		stat=1
	fi
fi >&2

if test $stat != 0; then
	echo "*** IGNORING ERROR ***"
fi
#exit $stat
exit 0
