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
# Test that a public re-exported library is automatically added as a dependent
# unless nothing is used from it.
#



all: all-${ARCH}

all-i386: all-new all-old
all-x86_64: all-new all-old
all-armv6: all-new 
all-armv7: all-new 


all-old:
	# -sub_library for 10.4
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib  -install_name /usr/lib/libbar.dylib -mmacosx-version-min=10.4
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -lbar -L. -sub_library libbar -mmacosx-version-min=10.4
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	${CC} ${CCFLAGS} main.c -DCALL_BAR libfoo.dylib -o main -L. -mmacosx-version-min=10.4
	otool -L main | grep libbar | ${FAIL_IF_EMPTY}
	nm -m main | grep _bar | grep libbar | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main -L. -mmacosx-version-min=10.4
	otool -L main | grep libbar | ${FAIL_IF_STDIN}
	

all-new:		
	# -sub_library for 10.5
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib  -install_name /usr/lib/libbar.dylib 
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -lbar -L. -sub_library libbar 
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	${CC} ${CCFLAGS} main.c -DCALL_BAR libfoo.dylib -o main -L. 
	otool -L main | grep libbar | ${FAIL_IF_EMPTY}
	nm -m main | grep _bar | grep libbar | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main -L. 
	otool -L main | grep libbar | ${FAIL_IF_STDIN}
	
			
	${PASS_IFF} /usr/bin/true


clean:

	rm -rf libbar.dylib libfoo.dylib main 
