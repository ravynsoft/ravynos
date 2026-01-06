##
# Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
# The point of this test if the linker already has a weak definition
# it does not try to find another copy in an archive
#
# There are two case to test:
# 1) both the main .o files and the archive have the same weak symbol (_foo)
# 2) main.o has a weak symbol and the archive has a non-weak symbol (_baz)
# In both cases the linker should ignore the archive. 
#

run: all

all:
	${CC} ${CCFLAGS} foo.c -c -o foo-${ARCH}.o 
	${FAIL_IF_BAD_OBJ} foo-${ARCH}.o
	${CC} ${CCFLAGS} bar.c -c -o bar-${ARCH}.o 
	${FAIL_IF_BAD_OBJ} bar-${ARCH}.o
	${CC} ${CCFLAGS} baz.c -c -o baz-${ARCH}.o 
	${FAIL_IF_BAD_OBJ} baz-${ARCH}.o
	libtool -static foo-${ARCH}.o  bar-${ARCH}.o  baz-${ARCH}.o -o libfoobar-${ARCH}.a
	${CC} ${CCFLAGS} main.c foo.c -lfoobar-${ARCH} -L. -o main-${ARCH} 
	${FAIL_IF_BAD_MACHO} main-${ARCH}
	nm -m main-${ARCH} | grep _baz | grep weak | ${PASS_IFF_STDIN}

clean:
	rm -rf main-* *.o *.a
