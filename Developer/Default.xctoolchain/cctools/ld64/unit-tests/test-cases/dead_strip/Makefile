##
# Copyright (c) 2010 Apple Inc. All rights reserved.
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
# The point of this test is a sanity check -dead_strip
#
# 1) in a main executable, dead globals are removed
# 2) in a dylib/bundle with -exported_symbols_list, dead globals are removed
# 3) in a dylib/bundle without -exported_symbols_list, dead globals are *not* removed
#

run: all

all:
	${CC} ${CCFLAGS} main.c deadwood.c -dead_strip -o main-${ARCH}
	size -m -l main-${ARCH} | grep __PAGEZERO | ${FAIL_IF_EMPTY}
	${FAIL_IF_BAD_MACHO} main-${ARCH}
	nm -j main-${ARCH} | egrep 'dead_wood|dead_door' | ${FAIL_IF_STDIN}
	${CC} ${CCFLAGS} -dynamiclib main.c deadwood.c -dead_strip -exported_symbols_list main.exp -o dylib-${ARCH}
	${FAIL_IF_BAD_MACHO} dylib-${ARCH}
	nm -j dylib-${ARCH} | grep dead | ${FAIL_IF_STDIN}
	${CC} ${CCFLAGS} -dynamiclib main.c deadwood.c -dead_strip -o dylib2-${ARCH}
	${FAIL_IF_BAD_MACHO} dylib2-${ARCH}
	nm -j dylib2-${ARCH} | grep dead_door_knob | ${FAIL_IF_EMPTY}
	nm -j dylib2-${ARCH} | grep deadwood | ${PASS_IFF_EMPTY}

clean:
	rm -rf main-* dylib-* dylib2-*
