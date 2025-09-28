#! /bin/sh
#
# rlvers.sh -- run a program that prints out the readline version number
#	       using locally-installed readline libraries
#

# Copyright (C) 1996-2002 Free Software Foundation, Inc.
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

PROGNAME=`basename $0`

: ${TMPDIR:=/tmp}
TDIR=$TMPDIR/rlvers

# defaults
CC=cc
RL_LIBDIR=/usr/local/lib
RL_INCDIR=/usr/local/include

TERMCAP_LIB="-ltermcap"

# cannot rely on the presence of getopts
while [ $# -gt 0 ]; do
	case "$1" in
	-C)	shift ; CC="$1"; shift ;;
	-I)	shift ; RL_INCDIR="$1" ; shift ;;
	-L)	shift ; RL_LIBDIR="$1" ; shift ;;
	-T)	shift ; TERMCAP_LIB="$1" ; shift ;;
	-v)	shift ; verbose=y ;;
	--)	shift ; break ;;
	*)	echo "${PROGNAME}: usage: $PROGNAME [-C compiler] [-L libdir] [-v]" >&2 ; exit 2;;
	esac
done

# if someone happened to install examples/rlversion, use it (it's not
# installed by default)
if test -f ${RL_LIBDIR}/rlversion ; then
	if [ -n "$verbose" ]; then
		echo "${PROGNAME}: using installed rlversion from ${RL_LIBDIR}/rlversion"
	fi
	v=`${RL_LIBDIR}/rlversion 2>/dev/null`
	case "$v" in
	unknown | "")	echo 0 ;;
	*)		echo "$v" ;;
	esac
	exit 0
fi

if [ -n "$verbose" ]; then
	echo "${PROGNAME}: using ${RL_LIBDIR} to find libreadline"
	echo "${PROGNAME}: attempting program compilation"
fi

# make $TDIR mode 0700
mkdir $TDIR || {
	echo "${PROGNAME}: ${TDIR}: file exists" >&2
	echo 0
	exit 1
}
chmod 700 $TDIR

trap 'rm -f $TDIR/rlvers $TDIR/rlvers.? ; rmdir $TDIR' 0 1 2 3 6 15

cat > $TDIR/rlvers.c << EOF
#include <stdio.h>
extern char *rl_library_version;

main()
{
	printf("%s\n", rl_library_version ? rl_library_version : "0");
	exit(0);
}
EOF

opwd=`pwd`

cd $TDIR || {
	echo "${PROGNAME}: cannot cd to $TDIR" >&2
	echo 0
	exit 1
}
	
if eval ${CC} -L${RL_LIBDIR} -I${RL_INCDIR} -o $TDIR/rlvers $TDIR/rlvers.c -lreadline ${TERMCAP_LIB};
then
	v=`$TDIR/rlvers`
else
	if [ -n "$verbose" ] ; then
		echo "${PROGNAME}: compilation failed: status $?"
		echo "${PROGNAME}: using version 0"
	fi
	v=0
fi

case "$v" in
unknown | "")	echo 0 ;;
*)		echo "$v" ;;
esac

cd $opwd
exit 0
