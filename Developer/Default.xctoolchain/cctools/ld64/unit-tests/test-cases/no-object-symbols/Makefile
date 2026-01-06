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
# Test that when ld creates an object file with no symbols 
# and no section content, that the segment size is zero
# and the LC_SYMTAB is empty.
#
# <rdar://problem/6048484> LC_SEGMENT_64 filesize incorrect for MH_OBJECT filetype
#

run: all

all:
	${CC} -arch ${ARCH} -c empty.s -o empty.o
	${LD} -r empty.o -x -o empty2.o
	${OTOOL} -lv empty2.o | egrep 'vmsize 0x0[0]+$$' | ${FAIL_IF_EMPTY}
	${OTOOL} -lv empty2.o | grep 'filesize 0' | ${FAIL_IF_EMPTY}
	${OTOOL} -lv empty2.o | grep 'nsyms 0' | ${FAIL_IF_EMPTY}
	${OTOOL} -lv empty2.o | grep 'symoff 0' | ${FAIL_IF_EMPTY}
	${PASS_IFF} true
	
clean:
	rm empty.o empty2.o
