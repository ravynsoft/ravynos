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
# verify -rename_sectione works with LTO 
#

run: all

all:
	${CC} ${CCFLAGS} -flto a.c -c -o a.o
	${CC} ${CCFLAGS} -flto b.c -c -o b.o
	${CC} ${CCFLAGS} -flto main.c -c -o main.o
	${CC} ${CCFLAGS} main.o a.o b.o -Wl,-preload -o main.preload \
		-e _entry -nostdlib -Wl,-segalign,0x20 -Wl,-seg1addr,0x200 \
		-Wl,-rename_section,__DATA,__data,__RAM,__vars \
		-Wl,-rename_section,__TEXT,__text,__ROM,__code \
		-Wl,-rename_section,__TEXT,__eh_frame,__ROM,__eh_frame \
		-Wl,-rename_section,__TEXT,__cstring,__ROM,__const 
	size -m -l main.preload | grep __TEXT  | ${FAIL_IF_STDIN}
	size -m -l main.preload | grep __DATA  | ${FAIL_IF_STDIN}
	nm -m main.preload  | grep __ROM | grep __code | grep _entry | ${FAIL_IF_EMPTY}
	nm -m main.preload  | grep __RAM | grep __vars | grep _mystring | ${FAIL_IF_EMPTY}
	size -m -l main.preload | grep __ROM  | ${PASS_IFF_STDIN}



	
clean:
	rm  -f a.o b.o main.o main.preload
