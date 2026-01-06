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
# Verify that -exported_symbols_list can be used with -r
# to reduce visibility of symbols and any missing symbols
# causes an error
#

run: all

all:
	${CC} ${CCFLAGS} test.c -c -o test.o
	${FAIL_IF_BAD_OBJ} test.o
	${LD} -arch ${ARCH} -r -keep_private_externs test.o -exported_symbols_list test.exp -o test-r.o
	${FAIL_IF_BAD_OBJ} test-r.o
	# verify common not in export-list got demoted to private extern
	nm -m test-r.o | grep "private external _common_global1" | ${FAIL_IF_EMPTY} 
	# verify only _common_global1 and _func_global1 changed
	nm -m test.o | egrep -v '_common_global1|_func_global1|__eh_frame|__data' > test.nm
	nm -m test-r.o | egrep -v '_common_global1|_func_global1|__eh_frame|__data' > test-r.nm
	diff test.nm test-r.nm
	# verify without -keep_private_externs that commons stay private extern 
	${LD} -arch ${ARCH} -r  test.o -exported_symbols_list test.exp -o test-rr.o
	nm -m test-rr.o | grep _common_hidden | grep ') external' | ${FAIL_IF_STDIN}
	nm -m test-rr.o | grep _common_global1 | grep ') external' | ${FAIL_IF_STDIN}
	# should error out if told to export unavailable symbol 
	${FAIL_IFF_SUCCESS} ${LD} -arch ${ARCH} -r test.o -exported_symbols_list test-bad.exp -o test2.o 2>/dev/null

clean:
	rm -rf test.o test-r.o test-rr.o test.nm test-r.nm test2.o
