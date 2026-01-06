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

SHELL = bash # use bash shell so we can redirect just stderr


#
# Text -why_live option with dead code stripping
#

run: all

all:
	${CC} ${CCFLAGS} main.c foo.c bar.c -o main -dead_strip -Wl,-why_live,_bar 2>call_chains
	grep _bar call_chains | ${FAIL_IF_EMPTY}
	grep _foo call_chains | ${FAIL_IF_EMPTY}
	grep _main call_chains | ${FAIL_IF_EMPTY}
	grep _frob call_chains | ${FAIL_IF_STDIN}
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm  main call_chains
