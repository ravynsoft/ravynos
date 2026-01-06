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
# <rdar://problem/6069861> ld -r for x86_64 is changing visibility of cstring constants
#
# In order to coalesce strings from different .o files, the linker internally
# makes them weak/hidden.  Test to make sure that weak/hidden does not leak out.
#

STRING_LABEL_COUNT = 0

ifeq (${ARCH},x86_64)
	STRING_LABEL_COUNT = 2
endif
ifeq (${ARCH},arm64)
	STRING_LABEL_COUNT = 2
endif


run: all

all:
	${CC} ${CCFLAGS} -c foo.c -o foo.o
	${CC} ${CCFLAGS} -c bar.c -o bar.o
	${LD} -arch ${ARCH} -r foo.o bar.o -o foobar.o
	nm -nm foobar.o | grep __cstring | grep "weak private external" | ${FAIL_IF_STDIN}
	${LD} -arch ${ARCH} -r foo.o bar.o -o foobar.o -keep_private_externs
	nm -nm foobar.o | grep __cstring | grep "weak private external" | ${FAIL_IF_STDIN}
	nm -m foobar.o | grep __cstring | wc -l | grep ${STRING_LABEL_COUNT}  | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} foo.o bar.o -dynamiclib -o libfoobar.dylib
	nm -m libfoobar.dylib | grep __cstring | wc -l |  egrep '[01]'  | ${FAIL_IF_EMPTY}
	${PASS_IFF} /usr/bin/true

clean:
	rm -f foo.o bar.o foobar.o libfoobar.dylib
