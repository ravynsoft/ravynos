##
# Copyright (c) 2012 Apple Inc. All rights reserved.
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
# <rdar://problem/12409381> Assertion failure when linking CoreFoundation with LTO
#
# Command line aliases need to have section type reset after lto pass.
#

run: all

all:
	${CC} ${CCFLAGS} -flto foo.c -c -o foo.o
	${CC} ${CCFLAGS} foo.o -dynamiclib -o libfoo.dylib -Wl,-alias,_foo,_bar
	${PASS_IFF_GOOD_MACHO} libfoo.dylib
	
clean:
	rm -rf foo.o libfoo.dylib
