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
# <rdar://problem/5935600> LTO : 176.gcc and 177.mesa build failure at -O4
#
# Check that LTO can bring in an archive member and that member needs a 
# symbol from a dylib.
#

run: all

all:
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	libtool -static foo.o -o libfoo.a 
	${CC} ${CCFLAGS} -flto main.c -c -o main.o
	${CC} ${CCFLAGS} main.o -o main libfoo.a 
	${PASS_IFF_GOOD_MACHO} main
	
clean:
	rm -rf main foo.o libfoo.a main.o
