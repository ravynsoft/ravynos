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
# Test that when linking a main executable for flat-namespace
# that undefines in loaded flat-namespace dylibs are resolved.
#

run: all

all:
	${CC} ${CCFLAGS} foo.c -flat_namespace -dynamiclib -o libfoo.dylib -undefined suppress
	${CC} ${CCFLAGS} bar.c -c -o bar.o
	libtool -static bar.o -o libbar.a
	# test that linking main executable -twolevel_namespace does not pull in bar()
	${CC} ${CCFLAGS} main.c libfoo.dylib libbar.a -o main
	nm -mn main | grep _bar | ${FAIL_IF_STDIN}
	# test that linking dylib -flat_namespace does not pull in bar()
	${CC} ${CCFLAGS} main.c -flat_namespace libfoo.dylib libbar.a -dynamiclib -o main.dylib
	nm -mn main.dylib | grep _bar | ${FAIL_IF_STDIN}
	# test that linking main executable -flat_namespace pulls in bar()
	${CC} ${CCFLAGS} main.c -flat_namespace libfoo.dylib libbar.a -o main_flat
	nm -mn main_flat | grep _bar | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main_flat
	
clean:
	rm libfoo.dylib libbar.a bar.o main main_flat main.dylib 
