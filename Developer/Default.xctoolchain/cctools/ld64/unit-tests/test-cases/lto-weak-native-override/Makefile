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
# Check that LTO works when a llvm bitcode has a weak symbol _foo
# and mach-o has a strong _foo.
#

run: all

all:
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	${CC} ${CCFLAGS} -flto main.c -c -o main.o
	${CC} ${CCFLAGS} main.o foo.o -o main 
	otool -Iv main | grep _abort | ${FAIL_IF_STDIN}
	${PASS_IFF_GOOD_MACHO} main
	
clean:
	rm  main foo.o  main.o
 