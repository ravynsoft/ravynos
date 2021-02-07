#!/bin/sh
#
#   clean_cpu.sh
#
#   Clean up the target cpu name.
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

case "$1" in
    # Intel processors are made equivalent
    i[3456]86)
	echo ix86
	exit 0
	;;
    # Make all alpha variants the same
    alpha*)
	echo alpha
	exit 0
	;;
    # Make all hppa variants the same
    hppa*)
        echo hppa
        exit 0
        ;;
    *)
	echo $1
        exit 0
	;;
esac
