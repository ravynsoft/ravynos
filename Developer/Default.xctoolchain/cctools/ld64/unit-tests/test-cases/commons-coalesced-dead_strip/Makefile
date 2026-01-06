##
# Copyright (c) 2007 Apple Inc. All rights reserved.
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
# The point of this test is to verify if a header file is missing an extern that there won't be
# duplicates definitions when using -dead_strip.
#

run: all

all:
	${CC} ${CCFLAGS} a.c -c -o a.o
	${CC} ${CCFLAGS} b.c -c -o b.o
	${CC} ${CCFLAGS} c.c -c -o c.o
	${CC} -arch ${ARCH} -dynamiclib a.o b.o c.o -o libabc.dylib -dead_strip
	${PASS_IFF_GOOD_MACHO} libabc.dylib

clean:
	rm -rf a.o b.o c.o libabc.dylib one  abc.bar.count
