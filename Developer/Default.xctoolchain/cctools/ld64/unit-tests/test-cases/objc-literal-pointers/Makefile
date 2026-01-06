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
# Verify an Objective-C object file when run through
# ld -r is unaltered.
#   <rdar://problem/5680988> __cls_refs section losing S_LITERAL_POINTERS section type
#
# note: i386 and ppc objc use some anonymous zerofill that moves and needs to be ignore to compare
#

run: all

all:
	${CC} ${CCFLAGS} test.m -c -o test.o
	${OBJECTDUMP}  -no_content test.o > test.dump

	${LD} -arch ${ARCH} -r test.o -keep_private_externs -o test-r.o
	${OBJECTDUMP}  -no_content test-r.o  > test-r.dump

	diff test.dump test-r.dump | ${PASS_IFF_EMPTY}
	
clean:
	rm -rf test.o test.dump test-r.o test-r.dump
