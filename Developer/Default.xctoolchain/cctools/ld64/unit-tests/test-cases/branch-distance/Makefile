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
# test thumb2 branch ranges
#

run: all


all:
	${CC} ${CCFLAGS} foo.s -c 
	${CC} ${CCFLAGS} bar.s -c 
	${CC} ${CCFLAGS} foo.o bar.o -e _foo -o foobar ${ARCH_FLAGS} -nostdlib -lSystem
	${LD} -arch ${ARCH} -r foo.o bar.o -o foobar.o
	${CC} ${CCFLAGS} foobar.o -e _foo -o foobar2 ${ARCH_FLAGS} -nostdlib -lSystem
	${PASS_IFF_GOOD_MACHO} foobar

clean:
	rm  *.o foobar*
