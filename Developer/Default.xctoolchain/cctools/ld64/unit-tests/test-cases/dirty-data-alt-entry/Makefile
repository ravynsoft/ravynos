##
# Copyright (c) 2016 Apple Inc. All rights reserved.
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
# Test that -dirty_data_list will move alt-entry symbols
##


run: all

all:
	${CC} ${CCFLAGS} -dynamiclib test.s -o libtest1.dylib -Wl,-dirty_data_list,1.dirty
	${FAIL_IF_BAD_MACHO} libtest1.dylib
	nm -nm libtest1.dylib | grep __DATA_DIRTY | grep _sym1 | ${FAIL_IF_EMPTY}
	nm -nm libtest1.dylib | grep __DATA_DIRTY | grep _sym2 | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} -dynamiclib test.s -o libtest2.dylib -Wl,-dirty_data_list,2.dirty
	${FAIL_IF_BAD_MACHO} libtest2.dylib
	nm -nm libtest2.dylib | grep __DATA_DIRTY | grep _sym1 | ${FAIL_IF_EMPTY}
	 nm -nm libtest2.dylib | grep __DATA_DIRTY | grep _sym2 | ${FAIL_IF_EMPTY}
	${PASS_IFF} /usr/bin/true


clean:

	rm -rf   libtest1.dylib libtest2.dylib
