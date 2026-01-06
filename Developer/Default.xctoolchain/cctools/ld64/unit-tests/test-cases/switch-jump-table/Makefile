##
# Copyright (c) 2008 Apple Inc. All rights reserved.
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
# Test that -mdynamic-no-pic jump table in the middle of
# a function does not cause relocations.
#
# <rdar://problem/5847206> SPEC2000/eon built with -mdynamic-no-pic won't run
#

all: test-${FILEARCH}


test-ppc:  test-good
test-x86_64: test-good
test-i386: test
test-arm: test

test:
	# check jump table in a weak function
	${CC} ${CCFLAGS} main.c switch.s -o main -Wl,-no_pie
	otool -rv main | grep _foo | ${FAIL_IF_STDIN}
	otool -rv main | grep _bar | ${FAIL_IF_STDIN}
	# check jump table in a regular function with -flat_namespace
	${CC} ${CCFLAGS} main.c switch.s -o main  -flat_namespace -Wl,-no_pie
	otool -rv main | grep _foo | ${FAIL_IF_STDIN}
	otool -rv main | grep _bar | ${FAIL_IF_STDIN}
	# check jump table in a regular function that is interposable
	${CC} ${CCFLAGS} main.c switch.s -o main  -Wl,-interposable_list,interpose.exp -Wl,-no_pie
	otool -rv main | grep _foo | ${FAIL_IF_STDIN}
	otool -rv main | grep _bar | ${FAIL_IF_STDIN}
	# check jump table with -pie, should have no external and some local relocations
	${CC} ${CCFLAGS} main.c switch.s -o main  -Wl,-pie -read_only_relocs suppress
	otool -rv main | grep "External relocation" | ${FAIL_IF_STDIN}
#	otool -rv main | grep "Local relocation" | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main

test-good:
	${PASS_IFF} true

clean:
	rm -f main
