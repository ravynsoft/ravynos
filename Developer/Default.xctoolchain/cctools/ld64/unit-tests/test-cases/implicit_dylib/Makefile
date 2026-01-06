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

PWD = $(shell pwd)


#	Verify -no_implicit_dylibs option 
# 	<rdar://problem/5451987> add option to disable implicit load commands for indirectly used public dylibs
#


run: all

all:
	${CC} ${CCFLAGS} bar.c -dynamiclib -o libbar.dylib -install_name /usr/lib/libbar.dylib
	${CC} ${CCFLAGS} foo.c  -dynamiclib -o libfoo.dylib -Wl,-reexport_library,libbar.dylib -sub_library libbar
	# verify that main gets bar from libbar
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main -L.
	nm -m main | grep _bar | grep libbar | ${FAIL_IF_EMPTY}
	# verify that -no_implicit_dylibs causes main to get bar from libfoo
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main -Wl,-no_implicit_dylibs -L.
	nm -m main | grep _bar | grep libfoo | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main
	
clean:
	rm -rf libfoo.dylib libbar.dylib main 
