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
# Tests the use of wildcards in exported symbol lists and dead stripping
#

run: all

all:
	${CC} -dynamiclib foo.c -o libfoo.dylib -Wl,-exported_symbol -Wl,'_AB*' -dead_strip
	nm -j libfoo.dylib | grep _good | ${FAIL_IF_EMPTY}
	nm -j libfoo.dylib | grep _bad | ${FAIL_IF_STDIN}
	${PASS_IFF_GOOD_MACHO} libfoo.dylib
	
clean:
	rm libfoo.dylib
