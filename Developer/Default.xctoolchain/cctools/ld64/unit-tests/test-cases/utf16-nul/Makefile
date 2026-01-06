##
# Copyright (c) 2008-2010 Apple Inc. All rights reserved.
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
# Test that utf16 cfstring literals are coalesced.
# There is 3 CFSTR in foo.m and 1 CFSTR in bar.m
# After coalescing and dead stripping there should be only two CFSTR in the output
#


all:
	${CC} ${CCFLAGS} withnul.s -c -o withnul.o
	${CC} ${CCFLAGS} other.s -c -o other.o
	${LD} -r -arch ${ARCH} withnul.o other.o -o all.o
	size -m -l all.o | grep "__ustring): 44" | ${PASS_IFF_STDIN}
	

clean:
	rm -rf withnul.o other.o all.o
