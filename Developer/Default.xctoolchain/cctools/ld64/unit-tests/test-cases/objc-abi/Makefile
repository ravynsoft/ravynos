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
# Verify -objc_abi_version 2 works for i386
#
ifeq (${ARCH},i386)
	ALL = all-i386
else
	ALL = all
endif

run: ${ALL}

all:
	${PASS_IFF} /usr/bin/true

all-i386:
	${CC} ${CCFLAGS} test.m -framework Foundation -o test1 -Wno-objc-root-class
	size -m -l test1 | grep __image_info | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} test.m -Wno-objc-root-class -fobjc-abi-version=2 -Wl,-objc_abi_version,2 -framework Foundation -o test2
	size -m -l test2 | grep __objc_imageinfo | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} test2

clean:
	rm -rf test1 test2
