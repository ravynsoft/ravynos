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

SHELL = bash # use bash shell so we can redirect just stderr


#
# Test how tentative definitions interact with archives
# main.c has a tenative definition for _var which 
# should cause libfoo.a(foo.o) to be loaded which in turn
# should cause _bar from libbar.dylib to be used.
#
# <rdar://problem/5779681> ld crashes building XsanFS
# <rdar://problem/5852023> -undefined dynamic_lookup causes spurious extra symbols
# <rdar://problem/5613343> need to search for definitions for common symbols
#

run: all

all:
	${CC} ${CCFLAGS} bar.c -dynamiclib -o libbar.dylib
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	libtool -static foo.o -o libfoo.a 
	${CC} ${CCFLAGS} main.c libfoo.a libbar.dylib -o main
	# verify _foo got pulled in because _var was a tentative in main.o
	nm main | grep "_foo" | ${FAIL_IF_EMPTY}
	# verify -dead_strip pulls in non-tentative _var from libfoo.a
	${CC} ${CCFLAGS} main.c libfoo.a libbar.dylib -o main -dead_strip
	nm -m main | grep "_var" | grep __data | ${FAIL_IF_EMPTY}
	# verify dynamic_lookup works and has no duplicate symbols
	${CC} ${CCFLAGS} main.c libfoo.a -o main -undefined dynamic_lookup
	nm -m main | grep "looked up" | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main
		

clean:
	rm -rf main libfoo.a foo.o libbar.dylib
