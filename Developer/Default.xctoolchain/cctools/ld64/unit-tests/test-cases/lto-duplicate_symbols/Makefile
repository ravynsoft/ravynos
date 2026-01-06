##
# Copyright (c) 2017 Apple Inc. All rights reserved.
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
# rdar://problem/35099885
# Check that with ThinLTO linker can still distinguish between static and
# non-static functions of the same name, and doesn't mix them up.
#

run: all

all:
	${CC} ${CCFLAGS} -flto=thin static.c -c -o static.o
	${CC} ${CCFLAGS} -flto=thin non-static.c -c -o non-static.o
	${CC} ${CCFLAGS} -flto=thin main.c -c -o main.o
	# Option -flto-codegen-only is used to avoid LTO optimizations because they
	# can rename a static function and thus prevent from testing how linker
	# handles duplicate names.
	${CC} ${CCFLAGS} -flto=thin -Wl,-flto-codegen-only static.o non-static.o main.o -o main
	${FAIL_IF_BAD_MACHO} main
	nm -j main | grep --count "^_same_name$$" | grep "^2$$" | ${PASS_IFF_STDIN}

clean:
	rm -rf main static.o non-static.o main.o
