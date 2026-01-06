##
# Copyright (c) 2010 Apple Inc. All rights reserved.
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
# Verify that UUID is the same for the two binaries built
# from the same source file but with different intermediate
# object file paths.
#

run: all

all:
	${CC} ${CCFLAGS} -gdwarf-2 main.c -c -o main1.o
	${CC} ${CCFLAGS} -gdwarf-2 main.c -c -o main2.o
	${CC} ${CCFLAGS} main1.o -o main1
	${CC} ${CCFLAGS} main2.o -o main2
	otool -lv main1 | grep -A3 UUID > main1.uuid
	otool -lv main2 | grep -A3 UUID > main2.uuid
	${PASS_IFF} diff main1.uuid main2.uuid
	

clean:
	rm -rf main1.o main2.o main1 main2 main1.uuid main2.uuid
