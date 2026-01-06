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
# verify -section_order works
#

run: all

all:
	${CC} ${CCFLAGS} extra.s -c -o extra.o 
	${CC} ${CCFLAGS} more.s -c -o more.o 
	${CC} ${CCFLAGS} main.c -c -o main.o
	# test basic re-order of sections from different files
	${CC} ${CCFLAGS} main.o more.o extra.o -Wl,-preload -Wl,-pie -o main1.preload \
		-e _entry -nostdlib -Wl,-segalign,0x20 -Wl,-seg1addr,0x200 \
		-Wl,-section_order,__MYSEG,__my_yyy:__my_ccc:__my_aaa
	${OTOOL} -l main1.preload | grep "sectname __my_" > main1.found
	${FAIL_IF_ERROR} diff main1.found main1.expected
	# test renaming and re-ordering
	${CC} ${CCFLAGS} main.o more.o extra.o -Wl,-preload -Wl,-pie -o main2.preload \
		-e _entry -nostdlib -Wl,-segalign,0x20 -Wl,-seg1addr,0x200 \
		-Wl,-rename_section,__MYSEG,__my_yyy,__MYSEG,__my_iii \
		-Wl,-rename_section,__MYSEG,__my_ccc,__MYSEG,__my_jjj \
		-Wl,-section_order,__MYSEG,__my_iii:__my_aaa:__my_jjj
	${OTOOL} -l main2.preload | grep "sectname __my_" > main2.found
	${PASS_IFF} diff main1.found main1.expected
	
	
	
clean:
	rm  extra.o more.o main.o main1.preload main1.found main2.preload main2.found
