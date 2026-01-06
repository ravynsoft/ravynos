##
# Copyright (c) 2010Apple Inc. All rights reserved.
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
# Tests weak-external and weak-can-be-hidden symbols work.
#

run: all

all: 
	# test that _my_other_weak is hidden
	${CC} ${CCFLAGS} main.c -c -o main.o
	${CC} ${CCFLAGS} other.s -c -o other.o
	${CC} ${CCFLAGS} main.o other.o -o main
	nm -m main | grep _my_weak | grep "weak external" | ${FAIL_IF_EMPTY}
	nm -m main | grep _my_other_weak | grep "non-external" | ${FAIL_IF_EMPTY}
	${FAIL_IF_BAD_MACHO} main
	# test that .exp file can override auto-hide
	${CC} ${CCFLAGS} main.o other.o -o main2 -Wl,-exported_symbol,_my_other_weak
	nm -m main2 | grep _my_weak | grep "non-external" | ${FAIL_IF_EMPTY}
	nm -m main2 | grep _my_other_weak | grep "weak external" | ${FAIL_IF_EMPTY}
	${FAIL_IF_BAD_MACHO} main2
	${CC} ${CCFLAGS} main.o other.o -o main2 -Wl,-exported_symbol,_main
	nm -m main2 | grep _my_weak | grep "non-external" | ${FAIL_IF_EMPTY}
	nm -m main2 | grep _my_other_weak | grep "non-external" | ${FAIL_IF_EMPTY}
	${FAIL_IF_BAD_MACHO} main2
	# test that auto-hide bit survives ld -r
	${LD} -r -arch ${ARCH} other.o -o other-r.o
	${OBJECTDUMP} other.o > other.o.dump
	${OBJECTDUMP} other-r.o > other-r.o.dump
	${PASS_IFF} diff other.o.dump other-r.o.dump 
	
clean:
	rm -f main.o other.o main main2 other-r.o other.o.dump other-r.o.dump 
