##
# Copyright (c) 2008-2010 Apple Inc. All rights reserved.
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

#
# Verify that -lazy_library works for function calls
# but fails for data references
#

run: all

all:
	${CC} ${CCFLAGS} foo.c -dynamiclib -o libfoo.dylib
	${CC} ${CCFLAGS} main.c -Wl,-lazy_library,libfoo.dylib -o main
	${CC} ${CCFLAGS} main.c -lazy-lfoo -L. -o main
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} bad.c -Wl,-lazy_library,libfoo.dylib -o bad  2> fail.log
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} bad2.c -Wl,-lazy_library,libfoo.dylib -o bad2  2> fail.log
	
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -f libfoo.dylib main bad bad2 fail.log
	
