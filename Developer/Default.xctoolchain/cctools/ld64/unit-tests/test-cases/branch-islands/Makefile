##
# Copyright (c) 2008-2009 Apple Inc. All rights reserved.
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
# Simple test for branch islands
#

run: all


all:
	# Verify that we fail if there is no valid place to insert branch islands.
	#${CC} ${CCFLAGS} hello.c atomic_space.s extra.c -o hello ${ARCH_FLAGS} 2>&1 | grep "Unable to insert branch island. No insertion point available." | ${PASS_IFF_STDIN}

	${CC} ${CCFLAGS} hello.c space.s extra.c -Os -o hello ${ARCH_FLAGS}
	${PASS_IFF_GOOD_MACHO} hello


clean:
	rm  hello
