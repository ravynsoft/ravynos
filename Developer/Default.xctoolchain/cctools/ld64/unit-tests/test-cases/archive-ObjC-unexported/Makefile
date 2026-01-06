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

SHELL = bash # use bash shell so we can redirect just stderr

#
# The point of this test is to check that -ObjC loads all (and only)
# .o files that contain Objective-C code.
#

run: all

all:
	${CC} ${CCFLAGS} foo.m -c -o foo.o 
	${CC} ${CCFLAGS} bar.m -c -o bar.o 
	libtool -static bar.o foo.o -o libfoo.a
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} main.m libfoo.a -o main -framework Foundation -dead_strip -Wl,-unexported_symbols_list,main.nexp 2>main.log
	grep Foobar main.log | ${FAIL_IF_EMPTY}
	${PASS_IFF_SUCCESS} true
	
clean:
	rm -rf foo.o bar.o libfoo.a main.log
