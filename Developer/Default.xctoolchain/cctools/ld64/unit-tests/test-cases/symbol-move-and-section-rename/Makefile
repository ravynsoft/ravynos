##
# Copyright (c) 2016 Apple Inc. All rights reserved.
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
# Check interaction of -section_rename and -move_to_r[ow]_segment
#

run: all

all:
	${CC} ${CCFLAGS} main.c -c -o main.o
	${CC} ${CCFLAGS} foo1.c -c -o foo1.o
	${CC} ${CCFLAGS} foo2.c -c -o foo2.o
	libtool -static foo1.o foo2.o -o libfoo.a
	${LD} -arch ${ARCH} main.o -force_load libfoo.a \
        -preload -o main.preload \
		-e _main -trace_symbol_layout \
		-move_to_ro_segment __TMP spec.list \
		-rename_section __TMP __text __TEXT spec \
		-move_to_ro_segment __TMP2 foo.list \
		-rename_section __TMP2 __text __TEXT libfoo \
		-move_to_rw_segment __TMPD hot.list \
		-rename_section __TMPD __data __DATA hot \
		-rename_section __TMPD __common __DATA hot \
        > placement.log
	nm -m main.preload | grep _s1   | grep __TEXT | grep spec   | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _s2   | grep __TEXT | grep spec   | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _def  | grep __DATA | grep hot    | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _com3 | grep __DATA | grep hot    | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _foo1 | grep __TEXT | grep libfoo | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _foo2 | grep __TEXT | grep libfoo | ${FAIL_IF_EMPTY}
	${PASS_IFF} true

clean:
	rm  -f main.preload main.o foo1.o foo2.o libfoo.a placement.log
