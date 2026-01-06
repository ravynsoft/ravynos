##
# Copyright (c) 2011 Apple Inc. All rights reserved.
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

RELOC_COUNT = 3
ifeq (${ARCH},x86_64)
	RELOC_COUNT = 2
endif


#
# Test that ld can link a pie static executable
#

all:
	${CC} ${CCFLAGS} test.c -c -o test.o
	${CC} ${CCFLAGS} test.o -static -Wl,-pie -o test -e _entry  -nostdlib -Wl,-new_linker
	${DYLDINFO} -rebase test | grep __DATA | wc -l | grep ${RELOC_COUNT} | ${FAIL_IF_EMPTY}
	# verify trying to use absolute addressing fails
	${CC} ${CCFLAGS} -static bad.c -c -o bad.o
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} test.o bad.o -static -Wl,-pie -o test.bad -e _entry -nostdlib -Wl,-new_linker 2>/dev/null
	${PASS_IFF_GOOD_MACHO} test

clean:
	rm -rf test test.o bad.o test.bad


