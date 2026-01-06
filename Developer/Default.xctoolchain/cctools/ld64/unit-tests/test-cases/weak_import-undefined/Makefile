##
# Copyright (c) 2010 Apple Computer, Inc. All rights reserved.
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
# Test that setting weak_import on symbols in a linkage unit works
#


run: all

all:	
	${CC} ${CCFLAGS} -o weak weak.c -undefined dynamic_lookup
	${DYLDINFO} -bind weak | grep _myweakfunc | grep "weak import" | ${FAIL_IF_EMPTY} 
	${DYLDINFO} -lazy_bind weak | grep _myweakfunc | grep "weak import" | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO}  weak
	
clean:
	rm -rf weak
