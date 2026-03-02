#! /bin/sh

# Simple program to make new version numbers for the shell.
# Big deal, but it was getting out of hand to do everything
# in the makefile.  This creates a file named by the -o option,
# otherwise everything is echoed to the standard output.

# Copyright (C) 1996-2020 Free Software Foundation, Inc.
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
USAGE="$PROGNAME [-b] [-S srcdir] -d version -p patchlevel [-s status] [-o outfile]"

source_dir="."

while [ $# -gt 0 ]; do
	case "$1" in
	-o)	shift; OUTFILE=$1; shift ;;
	-b)	shift; inc_build=yes ;;
	-s)	shift; rel_status=$1; shift ;;
	-p)	shift; patch_level=$1; shift ;;
	-d)	shift; dist_version=$1; shift ;;
	-S)	shift; source_dir="$1"; shift ;;
	*)	echo "$PROGNAME: usage: $USAGE" >&2 ; exit 2 ;;
	esac
done

# Required arguments
if [ -z "$dist_version" ]; then
	echo "${PROGNAME}: required argument -d missing" >&2
	echo "$PROGNAME: usage: $USAGE" >&2
	exit 1
fi

#if [ -z "$patch_level" ]; then
#	echo "${PROGNAME}: required argument -p missing" >&2
#	echo "$PROGNAME: usage: $USAGE" >&2
#	exit 1
#fi

# Defaults
if [ -z "$rel_status" ]; then
	rel_status="release"
fi

build_ver=
if [ -r .build ]; then
	build_ver=`cat .build`
fi
if [ -z "$build_ver" ]; then
	build_ver=0
fi

# increment the build version if that's what's required

if [ -n "$inc_build" ]; then
	build_ver=`expr 1 + $build_ver`
fi

# what's the patch level?
if [ -z "$patch_level" ]; then
	patchlevel_h=$source_dir/patchlevel.h
	if [ -s $patchlevel_h ]; then
		patch_level=`cat $patchlevel_h | grep '^#define[ 	]*PATCHLEVEL' | awk '{print $NF}'`
	fi
fi
if [ -z "$patch_level" ]; then
	patch_level=0
fi

# If we have an output file specified, make it the standard output
if [ -n "$OUTFILE" ]; then
	if exec >$OUTFILE; then
		:
	else
		echo "${PROGNAME}: cannot redirect standard output to $OUTFILE" >&2
		exit 1
	fi
fi

# Output the leading comment.
echo "/* Version control for the shell.  This file gets changed when you say"
echo "   \`make version.h' to the Makefile.  It is created by mkversion. */"

# Output the distribution version.  Single numbers are converted to x.00.
# Allow, as a special case, `[:digit:].[:digit:][:alpha:]' for
# intermediate versions (e.g., `2.5a').
# Any characters other than digits and `.' are invalid.
case "$dist_version" in
[0-9].[0-9][a-z])	;;	# special case
*[!0-9.]*)	echo "mkversion.sh: ${dist_version}: bad distribution version" >&2
		exit 1 ;;
*.*)	;;
*)	dist_version=${dist_version}.00 ;;
esac

dist_major=`echo $dist_version | sed 's:\..*$::'`
[ -z "${dist_major}" ] && dist_major=0

dist_minor=`echo $dist_version | sed 's:^.*\.::'`
case "$dist_minor" in
"")	dist_minor=0 ;;
[a-z])	dist_minor=0${dist_minor} ;;
?)	dist_minor=${dist_minor} ;;
*)	;;
esac

#float_dist=`echo $dist_version | awk '{printf "%.2f\n", $1}'`
float_dist=${dist_major}.${dist_minor}

echo
echo "/* The distribution version number of this shell. */"
echo "#define DISTVERSION \"${float_dist}\""

# Output the patch level
#echo
#echo "/* The patch level of this version of the shell. */"
#echo "#define PATCHLEVEL ${patch_level}"

# Output the build version
echo
echo "/* The last built version of this shell. */"
echo "#define BUILDVERSION ${build_ver}"

# Output the release status
echo
echo "/* The release status of this shell. */"
echo "#define RELSTATUS \"${rel_status}\""

echo
echo "/* The default shell compatibility-level (the current version) */"
echo "#define DEFAULT_COMPAT_LEVEL ${dist_major}${dist_minor}"

# Output the SCCS version string
sccs_string="${float_dist}.${patch_level}(${build_ver}) ${rel_status} GNU"
echo
echo "/* A version string for use by sccs and the what command. */"
echo "#define SCCSVERSION \"@(#)Bash version ${sccs_string}\""

# extern function declarations
#echo
#echo '/* Functions from version.c. */'
#echo 'extern char *shell_version_string PARAMS((void));'
#echo 'extern void show_shell_version PARAMS((int));'

if [ -n "$inc_build" ]; then
	# Make sure we can write to .build
	if [ -f .build ] && [ ! -w .build ]; then
		echo "$PROGNAME: cannot write to .build, not incrementing build version" >&2
	else
		echo "$build_ver" > .build
	fi
fi
	
exit 0
