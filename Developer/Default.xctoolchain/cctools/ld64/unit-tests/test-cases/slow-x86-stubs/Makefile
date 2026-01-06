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
# Sanity check that  i386 for 10.6 does not use  __IMPORT segment
#

run: all



all:
	${CC} ${CCFLAGS} hello.c -o hello
	size -m -l hello | grep __IMPORT | ${FAIL_IF_STDIN} 
	${CC} ${CCFLAGS} hello.c -dynamiclib -o libhello.dylib 
	size -m -l libhello.dylib | grep __IMPORT | ${FAIL_IF_STDIN}
	${PASS_IFF_GOOD_MACHO} hello

clean:
	rm  hello libhello.dylib
