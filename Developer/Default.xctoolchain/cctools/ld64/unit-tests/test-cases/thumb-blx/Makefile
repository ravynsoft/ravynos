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
# Verify the linker parses call sites correctly.
# The tricky case is thumb, which uses a blx to call to
# the arm stubs.  This test verifies that there is no
# +2 error by checking for "plus" and that when the file
# is regenerated through ld -r that the dumped output
# remains unchanged.
#

run: all

all:
	${CC} ${CCFLAGS} test.c -c -o test.o
	${FAIL_IF_ERROR} ${OBJECTDUMP} -no_content test.o > test.o.dump
	# verify no +2 errors
	grep "plus" test.o.dump | ${FAIL_IF_STDIN}
	# verify .o file can be regenerated to an equivalent state
	${LD} -arch ${ARCH} -r -keep_private_externs test.o -o test-r.o
	${FAIL_IF_ERROR} ${OBJECTDUMP} -no_content test-r.o > test-r.o.dump
	# verify final linked image has no +2 errors
	${CC} ${CCFLAGS} test.o -o test
	otool -tV -p _main test | grep blx | grep -v _malloc | ${FAIL_IF_STDIN}
	${CC} ${CCFLAGS} test-r.o -o test-r
	otool -tV -p _main test-r | grep blx | grep -v _malloc | ${FAIL_IF_STDIN}
	${PASS_IFF} diff test.o.dump test-r.o.dump 

clean:
	rm -rf test.o test-r.o test.o.dump test-r.o.dump test test-r
