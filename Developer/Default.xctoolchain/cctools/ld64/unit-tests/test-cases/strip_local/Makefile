##
# Copyright (c) 2006-2007 Apple Inc. All rights reserved.
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
# This test case checks merges two .o files.  One uses
# a lazy and non-lazy pointer to access the second.
# The result then has local symbols stripped.  So the
# end result is a .o file with a lazy and non-lazy pointer to 
# anonymous stuff.
#
#


run: all

all:
	${CC} ${CCFLAGS} hello.c -c
	${FAIL_IF_BAD_OBJ} hello.o

	${CC} ${CCFLAGS} foo.c -c
	${FAIL_IF_BAD_OBJ} foo.o

	${LD} -r hello.o foo.o -o hellofoo.o
	${FAIL_IF_BAD_OBJ} hellofoo.o

	strip -x hellofoo.o -o hellofoostripped.o
	${CC} ${CCFLAGS} hellofoostripped.o -o main
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -rf *.o main
