##
# Copyright (c) 2007 Apple Inc. All rights reserved.
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
# Validate linker processing of absolute symbols
#

all:
	${CC} ${CCFLAGS} main.c -static -c
	${CC} ${CCFLAGS} abs.s -static -c
	${LD} -arch ${ARCH} -static main.o abs.o -e _main -o main -new_linker -macosx_version_min 10.6
	${LD} -arch ${ARCH} -static -r main.o abs.o -o all.o -new_linker 
	nm -m all.o | grep _myAbs | grep absolute | ${FAIL_IF_EMPTY}
	${LD} -arch ${ARCH} -static all.o -e _main -o main2 -new_linker -macosx_version_min 10.6
	${PASS_IFF_GOOD_MACHO} main2
	
clean:
	rm -rf main.o abs.o all.o main main2
