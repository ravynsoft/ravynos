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
#
# Tests that a main executable with a weak symbol has MH_WEAK_DEFINES 
# Tests that a main executable with a weak symbol made non-global by 
# an export list does not has MH_WEAK_DEFINES 
#

run: all

all: 
	${CC} ${CCFLAGS} main.c -o main
	otool -hv main | grep WEAK_DEFINES | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} main.c -o main -Wl,-exported_symbol,_main
	otool -hv main | grep WEAK_DEFINES | ${FAIL_IF_STDIN}
	${CC} ${CCFLAGS} main.c -o main -Wl,-exported_symbol,_my_weak
	otool -hv main | grep WEAK_DEFINES | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} main-strip-weak.c -o main-strip-weak
	otool -hv main-strip-weak | grep WEAK_DEFINES | ${FAIL_IF_STDIN}
	${PASS_IFF_GOOD_MACHO} main
	
clean:
	rm -f main main-strip-weak