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


SHELL = bash # use bash shell so we can redirect just stderr


#
#
#   <rdar://problem/5726215> comdat warnings in ld -r
#
#
run: all

all:
	${CXX} ${CCXXFLAGS} foo.cxx -c -o foo.o
	${CXX} ${CCXXFLAGS} bar.cxx -c -o bar.o
	${CXX} ${CCXXFLAGS} baz.cxx -c -o baz.o
	${LD} -r foo.o bar.o baz.o -o foobarbaz.o 2> warnings.log
	grep warning warnings.log | ${PASS_IFF_EMPTY}


clean:
	rm  foo.o bar.o baz.o foobarbaz.o warnings.log
