##
# Copyright (c) 2020 Apple Inc. All rights reserved.
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

#  Verify weak nature of a link is propagated to re-exports, but a strong link on the command line overrides it.
run: all

all:
	${CC}  ${CCFLAGS} bar.c -dynamiclib -o libbar.dylib -install_name /usr/lib/libbar.dylib
	${PASS_IFF_GOOD_MACHO} libbar.dylib
	${CC}  ${CCFLAGS} foo.c -dynamiclib -o libfoo.dylib -L. -Wl,-reexport-lbar -install_name /usr/lib/libfoo.dylib
	${PASS_IFF_GOOD_MACHO} libfoo.dylib
	${CC}  ${CCFLAGS} main.c -o a.out -L. -weak-lfoo
	otool -L a.out | grep libbar | grep weak | ${FAIL_IF_EMPTY}

	${CC}  ${CCFLAGS} main.c -o a.out -L. -weak-lfoo -lbar
	otool -L a.out | grep libbar | grep weak | ${PASS_IFF_EMPTY}
clean:
	rm -rf libfoo.dylib libbar.dylib a.out
