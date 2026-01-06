##
# Copyright (c) 2010-2011 Apple Inc. All rights reserved.
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

SHELL = bash # use bash shell so we can redirect just stderr

#
# <rdar://problem/7438246> LTO with 'dead code strip' can't ignore unused functions with undefined references
# <rdar://problem/10052396> LTO many have eliminated need for some undefines
#

run: all

all:
	${CC} ${CCFLAGS} -flto bar.c  -c -o bar.o
	${CC} ${CCFLAGS} -flto main.c -c -o main.o 
	${CC} ${CCFLAGS} main.o bar.o -o main
	${CC} ${CCFLAGS} main.o bar.o -o main -dead_strip 
	${CC} ${CCFLAGS} main.o bar.o -dynamiclib -o libmain.dylib -Wl,-exported_symbol,_main
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} main.o bar.o -dynamiclib -o libmain.dylib -dead_strip 2>/dev/null
	${PASS_IFF_GOOD_MACHO} main
	
clean:
	rm -f bar.o main.o main
	
