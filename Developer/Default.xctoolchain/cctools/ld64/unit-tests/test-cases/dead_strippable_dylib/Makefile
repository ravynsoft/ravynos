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
# Test -mark_dead_strippable_dylib
#
# foo is used.  
# bar is not used by should still be linked 
# baz is not used and should be automatically not linked
#


run: all

all:
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib 
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib 
	${CC} ${CCFLAGS} -dynamiclib baz.c -o libbaz.dylib -Wl,-mark_dead_strippable_dylib
	${CC} ${CCFLAGS} main.c libfoo.dylib libbar.dylib libbaz.dylib -o main
	otool -L main | grep libfoo.dylib | ${FAIL_IF_EMPTY}
	otool -L main | grep libbar.dylib | ${FAIL_IF_EMPTY}
	otool -L main | grep libbaz.dylib | ${FAIL_IF_STDIN}
	${PASS_IFF_GOOD_MACHO} main


clean:

	rm -rf libbar.dylib libfoo.dylib libbaz.dylib main 
