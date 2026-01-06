##
# Copyright (c) 2012 Apple Inc. All rights reserved.
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
# The point of this test is to check that -ObjC loads all (and only)
# .o files that contain Objective-C code that has gone through ld -r.
#

run: all

all:
	${CC} ${CCFLAGS} foo.m -c -o foo.o 
	${LD} -r -arch ${ARCH} foo.o -o foo-r.o
	${CC} ${CCFLAGS} bar.c -c -o bar.o 
	${LD} -r -arch ${ARCH} bar.o -o bar-r.o
	${CC} ${CCFLAGS} baz.m -c -o baz.o 
	${LD} -r -arch ${ARCH} baz.o -o baz-r.o
	${CC} ${CCFLAGS} cat.m -c -o cat.o 
	${LD} -r -arch ${ARCH} cat.o -o cat-r.o
	libtool -static foo-r.o bar-r.o baz-r.o cat-r.o -o liball.a
	${CC} ${CCFLAGS} main.c liball.a -o main -ObjC -framework Foundation -framework CoreFoundation
	${FAIL_IF_BAD_MACHO} main
	nm main | grep "_bar" | ${FAIL_IF_STDIN}
	nm main | grep "_Foo" | ${FAIL_IF_EMPTY}
	nm main | grep "_Baz" | ${FAIL_IF_EMPTY}
	nm main | grep "_mycatfunc" | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -rf main *.o *.a
