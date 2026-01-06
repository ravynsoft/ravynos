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
# Test magic $ld$hide works with public sub library
# <rdar://problem/8388362> RedGarnet's linker does not honor $ld$hide for umbrella libraries
#


run: all

all:
	# In this test case aaa and bbb both moved between libfoo and libar
	# between 10.4 and 10.5.
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib -install_name /usr/lib/libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -Wl,-reexport_library,libbar.dylib -o libfoo.dylib 
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib -install_name /usr/lib/libbar_alt.dylib
	${CC} ${CCFLAGS} main.c -o main-10.5  libfoo.dylib -L. -mmacosx-version-min=10.5
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} main.c -o main-10.6  libfoo.dylib -L. -mmacosx-version-min=10.6  2>/dev/null
	${CC} ${CCFLAGS} main.c -o main-10.7  libfoo.dylib -L. -mmacosx-version-min=10.7
	${PASS_IFF_GOOD_MACHO} main-10.7


clean:

	rm -rf  libbar.dylib libfoo.dylib main-10.5 main-10.6 main-10.7
