##
# Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#ALL_ARCH_OPTIONS = $(patsubst %,-arch %,$(subst ppc,,$(VALID_ARCHS)) )

# build the list of all valid archs for this platform...
ifeq ($(ARCH),i386)
	ALL_ARCH_OPTIONS = $(patsubst %,-arch %,$(filter-out arm%,$(VALID_ARCHS)) )
	SYSROOT = -isysroot $(OSX_SDK)
endif
ifeq ($(ARCH),x86_64)
	ALL_ARCH_OPTIONS = $(patsubst %,-arch %,$(filter-out arm%,$(VALID_ARCHS)) )
	SYSROOT = -isysroot $(OSX_SDK)
endif
ifeq ($(ARCH),armv7)
	ALL_ARCH_OPTIONS = $(patsubst %,-arch %,$(filter arm%,$(VALID_ARCHS)) )
	SYSROOT = -isysroot $(IOS_SDK)
endif
ifeq ($(ARCH),arm64)
	ALL_ARCH_OPTIONS = $(patsubst %,-arch %,$(filter arm%,$(VALID_ARCHS)) )
	SYSROOT = -isysroot $(IOS_SDK)
endif

#
# Test that blank stubs are handled properly
#

run: all

all:
# build example fully fat dylib
	gcc $(SYSROOT) ${CCFLAGS} `echo ${ALL_ARCH_OPTIONS}` -dynamiclib foo.c -o libfoo.dylib -install_name libfoo.dylib 
	${FAIL_IF_BAD_MACHO} libfoo.dylib

# handle the case of a native ppc compile--this sets the subtype, which must be passed to lipo
	if [ x${ARCH} != xppc ]; \
	then \
		SUB_ARCH=${ARCH}; \
	else \
		SUB_ARCH=`lipo -info libfoo.dylib | sed 's/.*://;s/ppc64 //;s/.* \(ppc[^ ]*\).*/\1/'`; \
		echo SUB_ARCH $$SUB_ARCH; \
		if [ x$$SUB_ARCH = xALL ]; \
		then \
			SUB_ARCH=ppc; \
		fi \
	fi; \
	lipo libfoo.dylib -remove $$SUB_ARCH -output libfoo.dylib

	lipo -create libfoo.dylib -arch_blank ${ARCH} -output libfoo.dylib
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main
	${OTOOL} -L main | grep libfoo | ${FAIL_IF_STDIN}
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -rf libfoo.dylib main
