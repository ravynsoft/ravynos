##
# Copyright (c) 2011 Apple Inc. All rights reserved.
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
# Test that L$start$.. labels are tracked
#

all: 
	${CC} ${CCFLAGS} -c test.s -o test.o
	${CC} ${CCFLAGS} test.o -dynamiclib -o libtest.dylib
	dyldinfo -arch ${ARCH} -data_in_code libtest.dylib| ${FAIL_IF_EMPTY}
	${LD} -r -arch ${ARCH} test.o -o test2.o
	${CC} ${CCFLAGS} test2.o -dynamiclib -o libtest2.dylib
	dyldinfo -arch ${ARCH} -data_in_code libtest2.dylib| ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} libtest.dylib
	
clean:
	rm -rf test.o test2.o libtest.dylib libtest2.dylib
