##
# Copyright (c) 2010 Apple Inc. All rights reserved.
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
# Test
#


run: all

all:	
	${CC} ${CCFLAGS} -dynamiclib -Wl,-force_symbols_weak_list foo1.exp foo.c -o libfoo.dylib 
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	nm -m libfoo.dylib | grep _foo1 | grep weak | ${FAIL_IF_EMPTY}
	nm -m libfoo.dylib | grep _wildcheck | grep weak | ${FAIL_IF_EMPTY}
	nm -m libfoo.dylib | grep _foo3 | grep weak | ${FAIL_IF_STDIN}
	nm -m libfoo.dylib | grep _willnot | grep weak | ${FAIL_IF_STDIN}
	${CC} ${CCFLAGS} -dynamiclib -Wl,-force_symbols_not_weak_list foo2.exp foo.c -o libfoo.dylib 
	nm -m libfoo.dylib | grep _foo2 | grep weak | ${FAIL_IF_STDIN}
	nm -m libfoo.dylib | grep _patterncheck | grep weak | ${FAIL_IF_STDIN}
	nm -m libfoo.dylib | grep _foo4 | grep weak | ${FAIL_IF_EMPTY}
	nm -m libfoo.dylib | grep _patnot | grep weak | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} libfoo.dylib


		
clean:
	rm -rf libfoo.dylib
