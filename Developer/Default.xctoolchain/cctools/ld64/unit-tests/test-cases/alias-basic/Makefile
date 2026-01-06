##
# Copyright (c) 2014 Apple Inc. All rights reserved.
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
# Verify that code and data references can be redirected via aliases. 
#

run: all

all:
	# verify aliases can redirect references to code and data
	${CC} -arch ${ARCH} ${CCFLAGS} main.c -c -o main.o
	${CC} -arch ${ARCH} ${ASMFLAGS} aliases.s -c -o aliases.o
	${CC} -arch ${ARCH} main.o aliases.o -o main.exe
	nm -nm main.exe | grep _barHidden | grep " external " | ${FAIL_IF_STDIN}
	# verify dead stripping can remove unused and undefined alias
	${CC} -arch ${ARCH} ${ASMFLAGS} aliases.s -c -o aliases2.o -DUNUSED_ALIAS=1
	${CC} -arch ${ARCH} main.o aliases2.o -dead_strip -o main2.exe 
	nm -nm main2.exe | grep _barAlt | ${FAIL_IF_STDIN}	
	${PASS_IFF_GOOD_MACHO} main2.exe

clean:
	rm -rf *.o *.dump *.exe
