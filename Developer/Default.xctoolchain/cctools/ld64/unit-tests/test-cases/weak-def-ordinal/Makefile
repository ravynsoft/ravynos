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
#  libfoo.dylib has weak defintiion of _foo
#  libbar.dylib has strong defintiion of _foo
#
# Tests that if you link against libfoo.dylib and libbar.dylib 
# that the two-level-namespace ordinal is set to the non-weak definition
#
#<rdar://problem/5137732> ld should keep looking when it finds a weak definition in a dylib#
#

run: all

all: 
	${CC} ${CCFLAGS} foo.c -dynamiclib -o libfoo.dylib
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	${CC} ${CCFLAGS} bar.c -dynamiclib -o libbar.dylib
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} main.c -o main libfoo.dylib libbar.dylib
	nm -m main | grep _aaa | grep libbar | ${FAIL_IF_EMPTY}
	nm -m main | grep _bbb | grep libfoo | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main
	
clean:
	rm libfoo.dylib  libbar.dylib main
