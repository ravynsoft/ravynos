##
# Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
# Test the weak_import attribute works
#


run: all

all:	
	${CC} ${CCFLAGS} -dynamiclib -single_module foo.c -o libfoo-${ARCH}.dylib 
	${FAIL_IF_BAD_MACHO} libfoo-${ARCH}.dylib

	${CC} ${CCFLAGS} main.c -o main-${ARCH} libfoo-${ARCH}.dylib 
	nm -m main-${ARCH} | grep _func1 | grep -v weak    >/dev/null
	nm -m main-${ARCH} | grep _func2 | grep    weak    >/dev/null
	nm -m main-${ARCH} | grep _func3 | grep -v weak    >/dev/null 
	nm -m main-${ARCH} | grep _func4 | grep    weak    >/dev/null
	nm -m main-${ARCH} | grep _data1 | grep -v weak    >/dev/null
	nm -m main-${ARCH} | grep _data2 | grep    weak    >/dev/null
	nm -m main-${ARCH} | grep _data3 | grep -v weak    >/dev/null
	nm -m main-${ARCH} | grep _data4 | grep    weak    >/dev/null
	nm -m main-${ARCH} | grep _data5 | grep -v weak    >/dev/null
	nm -m main-${ARCH} | grep _data6 | grep    weak    >/dev/null
	#otool -rv main-${ARCH} | grep _data6 > /dev/null
	${FAIL_IF_BAD_MACHO} main-${ARCH}

	${CC} ${CCFLAGS} main.c -dynamiclib -o main-${ARCH}.dylib libfoo-${ARCH}.dylib 
	nm -m main-${ARCH}.dylib | grep _func1 | grep -v weak    >/dev/null
	nm -m main-${ARCH}.dylib | grep _func2 | grep    weak    >/dev/null
	nm -m main-${ARCH}.dylib | grep _func3 | grep -v weak    >/dev/null 
	nm -m main-${ARCH}.dylib | grep _func4 | grep    weak    >/dev/null
	nm -m main-${ARCH}.dylib | grep _data1 | grep -v weak    >/dev/null
	nm -m main-${ARCH}.dylib | grep _data2 | grep    weak    >/dev/null
	nm -m main-${ARCH}.dylib | grep _data3 | grep -v weak    >/dev/null
	nm -m main-${ARCH}.dylib | grep _data4 | grep    weak    >/dev/null
	#otool -rv main-${ARCH}.dylib | grep _data6 > /dev/null
	${PASS_IFF_GOOD_MACHO} main-${ARCH}.dylib
	
clean:
	rm -rf *.dylib main-*
