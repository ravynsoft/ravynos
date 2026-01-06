##
# Copyright (c) 2008 Apple Inc. All rights reserved.
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
# Check that cstrings in custom sections are not uniqued with
# cstrings in the standard section.
#

run: all

all:
	${CC} ${CCFLAGS} -c foo.s -o foo.o
	${CC} ${CCFLAGS} -c bar.s -o bar.o
	${LD} -arch ${ARCH} -r foo.o bar.o -o foobar.o
	size -m -l foobar.o | grep "(__TEXT, __cstring): 13" | ${FAIL_IF_EMPTY}
	size -m -l foobar.o | grep "(__TEXT, __mystring): 15" | ${FAIL_IF_EMPTY}
	otool -lv foobar.o | grep -A10 __mystring | grep S_CSTRING_LITERALS | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} foo.o bar.o -dynamiclib -o libfoobar.dylib
	size -m -l libfoobar.dylib | grep "__cstring: 13" | ${FAIL_IF_EMPTY}
	size -m -l libfoobar.dylib | grep "__mystring: 15" | ${FAIL_IF_EMPTY}
	otool -lv libfoobar.dylib | grep -A10 __mystring | grep S_CSTRING_LITERALS | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} libfoobar.dylib

clean:
	rm  foo.o bar.o libfoobar.dylib foobar.o
