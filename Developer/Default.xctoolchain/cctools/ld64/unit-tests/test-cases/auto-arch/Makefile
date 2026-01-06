##
# Copyright (c) 2006-2007 Apple Inc. All rights reserved.
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

ifeq ($(FILEARCH),arm)
	LD_VERS = -ios_version_min 4.0 -syslibroot $(IOS_SDK)
	CC_VERS = -miphoneos-version-min=4.0
else
ifeq ($(FILEARCH),arm64)
	LD_VERS = -ios_version_min 4.0 -syslibroot $(IOS_SDK)
	CC_VERS = -miphoneos-version-min=4.0
else
	LD_VERS = -macosx_version_min 10.6
	CC_VERS = -mmacosx-version-min=10.6
endif
endif


#
# Check that ld can figure out architecture from .o files without -arch option
#

run: all

all:
	${CC} ${CC_VERS} ${CCFLAGS} hello.c -c -o hello.o 
	${FAIL_IF_BAD_OBJ} hello.o
	${LD} ${LDFLAGS} hello.o -dylib -o hello.dylib -lSystem $(LD_VERS) 
	file hello.dylib | grep ${FILEARCH} | ${PASS_IFF_STDIN}

clean:
	rm -rf hello.o hello.dylib
