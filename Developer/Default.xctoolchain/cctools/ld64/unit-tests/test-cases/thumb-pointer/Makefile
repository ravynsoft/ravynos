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
# <rdar://problem/6401277> pointers to thumb symbols can be mangled
#

run: all

all:
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	${CC} ${CCFLAGS} bar.c -c -o bar.o
	# verify no +1 thumb errors
	${FAIL_IF_ERROR} ${OBJECTDUMP} foo.o | grep "plus 0x00000001" | ${FAIL_IF_STDIN}
	${FAIL_IF_ERROR} ${OBJECTDUMP} bar.o | grep "plus 0x00000001" | ${FAIL_IF_STDIN}
	${LD} -arch ${ARCH} -r -keep_private_externs foo.o bar.o -o foobar.o
	${LD} -arch ${ARCH} -r -keep_private_externs foobar.o -o foobar2.o
	${FAIL_IF_ERROR} ${OBJECTDUMP} -no_content foobar.o > foobar.o.dump
	${FAIL_IF_ERROR} ${OBJECTDUMP} -no_content foobar2.o > foobar2.o.dump
	# verify no +1 thumb errors in merged result
	grep "plus 0x00000001" foobar.o.dump | ${FAIL_IF_STDIN}
	# verify round trip though ld -r works
	${PASS_IFF} diff foobar.o.dump foobar2.o.dump 

clean:
	rm -rf foo.o bar.o foobar.o foobar2.o foobar.o.dump foobar2.o.dump
