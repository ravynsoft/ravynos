##
# Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
# The point of this test is to verify a .o file can round-trip
# through ld -r correctly.  The ObjectDump utility is used
# dump a "canonical" textual representation of a .o file.
# The before and after .o files are then diff'ed.
# No differences means this test passes
#

run: all

all:
	${CC} ${CCFLAGS} test.m -c -o test.${ARCH}.o
	${FAIL_IF_ERROR} ${OBJECTDUMP} -no_content test.${ARCH}.o > test.${ARCH}.o.dump

	${LD} -arch ${ARCH} -r -keep_private_externs test.${ARCH}.o -o test-r.${ARCH}.o
	${FAIL_IF_ERROR} ${OBJECTDUMP} -no_content test-r.${ARCH}.o > test-r.${ARCH}.o.dump

	${PASS_IFF} diff test.${ARCH}.o.dump test-r.${ARCH}.o.dump

clean:
	rm -rf *.o *.dump
