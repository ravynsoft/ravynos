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
	${CC} ${CCFLAGS} main.c -c -o main.o
	${CC} ${CCFLAGS} other.c -c -o other.o
	${LD} -arch ${ARCH} main.o other.o -preload -o main.preload \
		-e _foo -trace_symbol_layout \
		-move_to_ro_segment __ROM1 rom1.symbols \
		-rename_section __TEXT __cstring __ROM2 mycstrings \
		-rename_segment __TEXT __ROM3 \
		-move_to_rw_segment __RAM1 ram1.symbols \
		-rename_section __DATA __data __RAM2 mydata \
		-rename_segment __DATA __RAM3 \
		-segment_order __ROM1:__ROM2:__ROM3:__RAM1:__RAM2:__RAM3
	nm -m main.preload | grep _foo | grep __ROM1 | grep __text | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _s1  | grep __ROM1 | grep __text | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _mylocal | grep __ROM1 | grep __text | ${FAIL_IF_EMPTY}
	size -m -l main.preload | grep __cstring |  ${FAIL_IF_STDIN}
	size -m -l main.preload | grep mycstrings |  ${FAIL_IF_EMPTY}
	size -m -l main.preload | grep __TEXT |  ${FAIL_IF_STDIN}
	nm -m main.preload | grep _mm | grep __ROM3 | grep __text | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _main | grep __ROM3 | grep __text | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _abc | grep __RAM1  | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _com | grep __RAM1  | ${FAIL_IF_EMPTY}
	size -m -l main.preload | grep __DATA |  ${FAIL_IF_STDIN}
	nm -m main.preload | grep _def | grep __RAM2 | grep mydata | ${FAIL_IF_EMPTY}
	nm -m main.preload | grep _ghi | grep __RAM2 | grep mydata | ${FAIL_IF_EMPTY}
	${PASS_IFF} true

clean:
	rm  -f main.preload main.o other.o
