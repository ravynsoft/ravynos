##
# Copyright (c) 2011 Apple Inc. All rights reserved.
#
# @APPLE_LICENSE_HEADER_START@
# 
# This file contains Original Code and/or Modifications of Original Code
# as defined in and that are subject to the Apple Public Source License
# Version 2.0 (the 'License'). You may not use this file except in
# compliance with the License. Please obtain a copy of the License at
# http://www.opensource.apple.com/apsl/ and read it before using this
# file.
# 
# The Original Code and all software distributed under the License are
# distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
# INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
# Please see the License for the specific language governing rights and
# limitations under the License.
# 
# @APPLE_LICENSE_HEADER_END@
##
TESTROOT = ../..
include ${TESTROOT}/include/common.makefile


#
# verify $ld$install_name$osXX$/new/path overides install_name
# 

FORCE_MIN_OS_VERSION = -mmacosx-version-min=10.5

ifeq ($(FILEARCH),arm)
	FORCE_MIN_OS_VERSION = -miphoneos-version-min=4.0
endif


run: all

all:
	${CC} ${CCFLAGS} foo.c -dynamiclib -o libfoo.dylib -install_name /usr/local/lib/libfoo.dylib
	otool -L libfoo.dylib | grep /usr/local/lib/libfoo.dylib | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main
	otool -L main | grep /usr/local/lib/libfoo.dylib | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} main.c libfoo.dylib -o mainAlt ${FORCE_MIN_OS_VERSION}
	otool -L mainAlt | grep /usr/lib/libfoo.dylib | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} libfoo.dylib

clean:
	rm -f libfoo.dylib main mainAlt
