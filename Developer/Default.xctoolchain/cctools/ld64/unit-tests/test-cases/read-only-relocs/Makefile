 ##
# Copyright (c) 2006-2007 Apple Inc. All rights reserved.
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
# Test that ld can linke a dylib built with -mdynamic-no-pic
#


SHELL = bash # use bash shell so we can redirect just stderr

NO_PIC =  
STATIC =  
RELOC_FAIL = ${FAIL_IF_SUCCESS}
LRELOCS_NEEDED = ${FAIL_IF_EMPTY}
XRELOCS_NEEDED = ${FAIL_IF_EMPTY}

ifeq (${ARCH},i386)
	NO_PIC =  -mdynamic-no-pic
	STATIC =  -static
endif
ifeq (${ARCH},ppc)
	NO_PIC =  -mdynamic-no-pic
	STATIC =  -mdynamic-no-pic
	XRELOCS_NEEDED =  ${FAIL_IF_STDIN}
	LRELOCS_NEEDED =  ${FAIL_IF_EMPTY}
endif
ifeq (${ARCH},x86_64)
	RELOC_FAIL =  
	XRELOCS_NEEDED =  ${FAIL_IF_STDIN}
	LRELOCS_NEEDED =  ${FAIL_IF_STDIN}
endif
ifeq (${FILEARCH},arm)
	NO_PIC =  -mdynamic-no-pic
	STATIC =  -static
endif



all:
	# build libfoo.dylib as regular dylib
	${CC} ${CCFLAGS} foo.c -dynamiclib -o libfoo.dylib
	# build libtest.dylib using -mdynamic-no-pic, should fail
	${CC} ${CCFLAGS} test_rebase.c -c ${NO_PIC}
	${RELOC_FAIL} ${CC} ${CCFLAGS} test_rebase.o libfoo.dylib -dynamiclib -o libtestrebase.dylib -read_only_relocs error 2>/dev/null
	${CC} ${CCFLAGS} test_bind.c -c ${NO_PIC}
	${RELOC_FAIL} ${CC} ${CCFLAGS} test_bind.o libfoo.dylib -dynamiclib -o libtestbind.dylib -read_only_relocs error 2>/dev/null
	# build libtest.dylib using -mdynamic-no-pic and -read_only_relocs suppress
	${CC} ${CCFLAGS} test_rebase.c -c ${NO_PIC}
	${CC} ${CCFLAGS} test_bind.c -c ${NO_PIC}
	${CC} ${CCFLAGS} test_rebase.o test_bind.o libfoo.dylib -dynamiclib -o libtest-no-pic.dylib -read_only_relocs suppress -Wl,-w
	# build libtest.dylib using -static and -read_only_relocs suppress
	${CC} ${CCFLAGS} test_rebase.c -c ${STATIC}
	${CC} ${CCFLAGS} test_bind.c -c ${STATIC}
	${CC} ${CCFLAGS} test_rebase.o test_bind.o libfoo.dylib -dynamiclib -o libtest-static.dylib -read_only_relocs suppress -Wl,-w
	otool -lv libtest-static.dylib | grep -A9 "sectname __text" | grep attributes | grep EXT_RELOC | ${XRELOCS_NEEDED}
	otool -lv libtest-static.dylib | grep -A9 "sectname __text" | grep attributes | grep LOC_RELOC | ${LRELOCS_NEEDED}
	# build main using -static and -read_only_relocs suppress
	${CC} ${CCFLAGS} main.c -c ${STATIC}
	${CC} ${CCFLAGS} main.o libfoo.dylib -o main -read_only_relocs suppress -Wl,-w
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -rf test_bind.o test_rebase.o libfoo.dylib libtestrebase.dylib libtestbind.dylib libtest-no-pic.dylib libtest-static.dylib main.o main
