##
# Copyright (c) 2007-2010 Apple Inc. All rights reserved.
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
# Check that -U and -undefined dynamic_lookup work
#

ALL = all-mac

#  -undefined dynamic_lookup is deprecated on iOS
ifeq ($(ARCH),arm64)
	ALL = all-ios
else
ifeq ($(ARCH),armv7)
	ALL = all-ios
endif
endif


run: all

all: ${ALL}

all-mac:
	${CC} ${CCFLAGS} main.c -o main -undefined dynamic_lookup
	nm -m main | grep _foo | grep "dynamically looked up" | ${FAIL_IF_EMPTY}
	${DYLDINFO} -lazy_bind -bind main | grep _foo | grep "flat-namespace" | ${FAIL_IF_EMPTY}
	${FAIL_IF_BAD_MACHO} main

	${CC} ${CCFLAGS} main.c -o main -Wl,-U,_foo 
	nm -m main | grep _foo | grep "dynamically looked up" | ${FAIL_IF_EMPTY}
	${DYLDINFO} -lazy_bind -bind main | grep _foo | grep "flat-namespace" | ${FAIL_IF_EMPTY}
	${FAIL_IF_BAD_MACHO} main

	${CC} ${CCFLAGS} main.c -o main -flat_namespace -Wl,-U,_foo
	nm -m main | grep _foo | grep "dynamically looked up" | ${FAIL_IF_STDIN}
	${DYLDINFO} -lazy_bind -bind main | grep _foo | grep "flat-namespace" | ${FAIL_IF_EMPTY}
	${FAIL_IF_BAD_MACHO} main

	${CC} ${CCFLAGS} main.c -bundle -o main.bundle -nodefaultlibs -undefined dynamic_lookup 
	nm -m main.bundle | grep _foo | grep "dynamically looked up" | ${FAIL_IF_EMPTY}
	${DYLDINFO} -lazy_bind -bind main.bundle | grep _foo | grep "flat-namespace" | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main.bundle

all-ios:
	${PASS_IFF} true

clean:
	rm -f main main.bundle
