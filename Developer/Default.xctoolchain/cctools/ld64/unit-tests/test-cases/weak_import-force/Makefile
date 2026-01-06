##
# Copyright (c) 2008 Apple Computer, Inc. All rights reserved.
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
# Test that -weak_library marks all symbols used as weak
#


run: all

all:	
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib 
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib 
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} main.c -o main -weak_library libfoo.dylib libbar.dylib
	nm -m main | grep _foo1 | grep  weak | ${FAIL_IF_EMPTY}
	nm -m main | grep _bar | grep  weak | ${FAIL_IF_STDIN}
	${PASS_IFF_GOOD_MACHO} main
	
clean:
	rm -rf libfoo.dylib libbar.dylib main
