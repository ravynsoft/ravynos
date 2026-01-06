##
# Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
# <rdar://problem/9551362> clang ld: bad codegen, pointer diff
# Test that codegen needing direct access to a weak symbol just issues
# a warning.  Check that if export list makes weak symbol hidden, there is
# no warning.
#

run: all

all:
	${CC} ${CCFLAGS} myglobal.c myhidden.s -dynamiclib -o libmy.dylib 2>warnings.txt
	grep "global weak"  warnings.txt | ${FAIL_IF_EMPTY}
	${FAIL_IF_BAD_MACHO} libmy.dylib
	${CC} ${CCFLAGS} myglobal.c myhidden.s -dynamiclib -Wl,-exported_symbol,_test -o libmy2.dylib 
	${PASS_IFF_GOOD_MACHO} libmy2.dylib

clean:
	rm -f libmy.dylib  libmy2.dylib warnings.txt
