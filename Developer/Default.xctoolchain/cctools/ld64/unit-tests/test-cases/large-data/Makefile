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
# Check that ld can link > 4GB zero fill section
#

SHELL = bash # use bash shell so we can redirect just stderr

ifeq (,${findstring 64,$(ARCH)})
	32BIT_SHOULD_FAIL = ${FAIL_IF_SUCCESS}
else
	32BIT_SHOULD_FAIL = 
endif


run: all

all:
	${CC} ${CCFLAGS} test1.c -c -o test1.o
	${CC} ${CCFLAGS} test2.c -c -o test2.o
	${CC} ${CCFLAGS} test3.c -c -o test3.o
	${CC} ${CCFLAGS} test4.c -c -o test4.o
	${32BIT_SHOULD_FAIL} ${CC} ${CCFLAGS} test1.o test2.o test3.o test4.o -dynamiclib -o libtest.dylib 2> fail.log
	${PASS_IFF} true

clean:
	rm -rf test*.o libtest.dylib fail.log
