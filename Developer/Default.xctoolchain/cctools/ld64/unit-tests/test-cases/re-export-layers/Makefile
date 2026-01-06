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
# Test all the different ways that re-exports can be specified and implemented
#



all: all-${ARCH}

all-i386: all-new all-old
all-x86_64: all-new all-old
all-armv6: all-new 
all-armv7: all-new 


all-new:	
	# -reexport_library for 10.5 and later
	${CC} ${CCFLAGS} -dynamiclib baz.c -o libbaz.dylib  
	${FAIL_IF_BAD_MACHO} libbaz.dylib
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib -Wl,-reexport_library,libbaz.dylib 
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-reexport_library,libbar.dylib 
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main
	${PASS_IFF_GOOD_MACHO} main

all-old:
	# -reexport_library for 10.4
	${CC} ${CCFLAGS} -dynamiclib baz.c -o libbaz.dylib  -mmacosx-version-min=10.4
	${FAIL_IF_BAD_MACHO} libbaz.dylib
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib -Wl,-reexport_library,libbaz.dylib -mmacosx-version-min=10.4
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-reexport_library,libbar.dylib -mmacosx-version-min=10.4
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main
	${FAIL_IF_BAD_MACHO} main



clean:

	rm -rf  libbaz.dylib libbar.dylib libfoo.dylib main
	
