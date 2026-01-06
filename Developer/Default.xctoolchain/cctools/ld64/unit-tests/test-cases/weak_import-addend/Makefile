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

#
# Test the weak_import attribute works
#

all: all-${ARCH}

all-ppc: all-true

all-ppc64: all-true

all-i386: all-true

all-armv6: all-true

all-armv7: all-true

all-true:
	${PASS_IFF} true

all-x86_64:	
	${CC} ${CCFLAGS} -dynamiclib test.s -o libtest.dylib -mmacosx-version-min=10.6
	${DYLDINFO} -bind -lazy_bind libtest.dylib | grep _malloc | grep weak | ${FAIL_IF_EMPTY}
	${DYLDINFO} -bind -lazy_bind libtest.dylib | grep _free | grep weak | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} libtest.dylib
	
clean:
	rm -rf libtest.dylib
