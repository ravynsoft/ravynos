#!/bin/sh
#
#   clean_os.sh
#
#   Clean up the target OS name for GNUstep.
#
#   Copyright (C) 1997 Free Software Foundation, Inc.
#
#   Author:  Scott Christley <scottc@net-community.com>
#
#   This file is part of the GNUstep Makefile Package.
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation; either version 3
#   of the License, or (at your option) any later version.
#   
#   You should have received a copy of the GNU General Public
#   License along with this library; see the file COPYING.
#   If not, write to the Free Software Foundation,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

# Nothing to clean right now
case "$1" in
    # Remove version number for FreeBSD
    freebsd2*)
	echo freebsdaout
	exit 0
	;;
    freebsd*)
	echo freebsd
	exit 0
	;;
    # Remove version number for Darwin
    # Versions currently most common have a quick hardcoded lookup
    darwin9*)
        echo darwin9
        exit 0
        ;;
    darwin8*)
        echo darwin8
        exit 0
        ;;
    darwin7*)
        echo darwin7
        exit 0
        ;;
    # Any other Darwin version falls here, where we use a slower sed
    # subprocess to remove everything but the first major number.
    darwin*)
        echo `echo "$1" | sed s/\\\\..*//`
        exit 0
        ;;
    *)
	echo $1
        exit 0
	;;
esac
