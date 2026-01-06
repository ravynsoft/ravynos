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
# verify -segment_order works
#

run: all

all:
	${CC} ${CCFLAGS} a.c -c -o a.o -static
	${CC} ${CCFLAGS} b.c -c -o b.o -static
	${CC} ${CCFLAGS} main.c -c -o main.o -static
	${CC} ${CCFLAGS} main.o a.o b.o -Wl,-preload -Wl,-pie -o main.preload \
		-e _entry -nostdlib -Wl,-segalign,0x20 -Wl,-seg1addr,0x200 \
		-Wl,-rename_section,__TEXT,__text,__ROM,__code \
		-Wl,-rename_section,__TEXT,__eh_frame,__ROM,__eh_frame \
		-Wl,-rename_section,__TEXT,__cstring,__ROM2,__strings \
		-Wl,-rename_section,__DATA,__data,__RAM,__inited \
		-Wl,-rename_section,__DATA,__common,__ZF,__zf \
		-Wl,-segment_order,__ROM2:__ROM:__RAM:__ZF 
	${OTOOL} -l main.preload | grep -A2 LC_SEGMENT | grep segname > main-segs.found
	${PASS_IFF} diff main-segs.found main-segs.expected


	
clean:
	rm  a.o b.o main.o main.preload main-segs.found
