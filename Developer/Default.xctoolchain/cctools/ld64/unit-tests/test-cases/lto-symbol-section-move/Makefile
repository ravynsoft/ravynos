##
# Copyright (c) 2014 Apple Inc. All rights reserved.
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
# Check interaction of -section_rename, -segment_rename, and -move_to_r._segment
#

run: all

all:
	${CC} ${CCFLAGS} main.c -c -o main.o -flto
	${CC} ${CCFLAGS} foo.c -c -o foo.o -flto
	${CC} ${CCFLAGS} other.c -c -o other.o -flto
	${LD} -arch ${ARCH} main.o foo.o other.o -preload -o main.preload \
		-e _foo -trace_symbol_layout \
		-move_to_ro_segment __ROM1 rom1.symbols \
		-move_to_rw_segment __RAM1 ram1.symbols 
	nm -m main.preload | grep _mainget | grep __ROM1 | grep __text | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _getpi | grep __ROM1 | grep __text | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _bar | grep __ROM1 | grep __text | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _foo | grep __ROM1 | grep __text | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _def | grep __RAM1  | grep __data | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _ghi | grep __RAM1  | grep __data | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _com | grep __RAM1  | grep __bss | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _abc | grep __RAM1  | grep __data | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _com4 | grep __RAM1  | grep __bss | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _main | grep __TEXT | grep __text | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _version | grep __TEXT | grep __text | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _otherget | grep __TEXT | grep __text | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _mylocal | grep __TEXT | grep __text | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _x | grep __DATA | grep __data | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _all | grep __DATA | grep __data | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _x | grep __DATA | grep __data | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _com5 | grep __DATA | grep __bss | ${FAIL_IF_EMPTY}
	${PASS_IFF} true

clean:
	rm  -f main.preload main.o other.o foo.o
