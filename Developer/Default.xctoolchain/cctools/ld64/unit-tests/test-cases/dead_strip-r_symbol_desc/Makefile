##
# Copyright (c) 2009 Apple Inc. All rights reserved.
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
# Test that N_NO_DEAD_STRIP bit survives ld -r
#

run: all

all:
	${CC} ${CCFLAGS} main.c -c -o main.o
	nm -m main.o | grep _bar | grep "no dead strip" | ${FAIL_IF_EMPTY}
	nm -m main.o | grep _foo | grep "no dead strip" | ${FAIL_IF_EMPTY}
	${LD} -r main.o -o main2.o
	nm -m main2.o | grep _bar | grep "no dead strip" | ${FAIL_IF_EMPTY}
	nm -m main2.o | grep _foo | grep "no dead strip" | ${FAIL_IF_EMPTY}
	${CC} main2.o -o main 
	nm -m main | grep _bar | grep "no dead strip" | ${PASS_IFF_EMPTY}

clean:
	rm -rf main.o main2.o main
