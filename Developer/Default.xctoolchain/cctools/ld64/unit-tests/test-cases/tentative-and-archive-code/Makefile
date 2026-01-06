##
# Copyright (c) 2011 Apple Inc. All rights reserved.
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
# Test how tentative definitions interact with archives
# main.c has a tenative definition for _foo which 
# should cause libfoo_data.a to be loaded, but not libfoo_tent.a
# nor libfoo_code.a.
#
# <rdar://problem/7890410> Snow Leopard linker bug with common symbols
#

run: all

all:
	${CC} ${CCFLAGS} junk.c -c -o junk.o
	# verify data symbol does get pulled in
	${CC} ${CCFLAGS} foo_data.c -c -o foo_data.o
	libtool -static junk.o foo_data.o -o libfoo_data.a
	${CC} ${CCFLAGS} main.c libfoo_data.a -o main_data
	nm -m main_data | grep _foo | grep __TEXT | ${FAIL_IF_STDIN}
	# verify tentative does not
	${CC} ${CCFLAGS} foo_tent.c -c -o foo_tent.o
	libtool -static junk.o foo_tent.o -o libfoo_tent.a
	${CC} ${CCFLAGS} main.c libfoo_tent.a -o main_tent
	nm -m main_tent | grep _foo | grep __TEXT | ${FAIL_IF_STDIN}
	# verify code does not
	${CC} ${CCFLAGS} foo_code.c -c -o foo_code.o
	libtool -static junk.o foo_code.o -o libfoo_code.a
	${CC} ${CCFLAGS} main.c libfoo_code.a -o main_code
	nm -m main_code | grep _foo | grep __TEXT | ${FAIL_IF_STDIN}
	# verify warning when code force to override tent
	#${CC} ${CCFLAGS} main.c -Wl,-force_load,libfoo_code.a -o main_code_force 2>main_code_force_warnings.txt
	#grep warning main_code_force_warnings.txt | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main_code

clean:
	rm -rf junk.o foo_data.o libfoo_data.a main_data foo_tent.o libfoo_tent.a main_tent foo_code.o libfoo_code.a main_code main_code_force main_code_force_warnings.txt
	
