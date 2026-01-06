##
# Copyright (c) 2013 Apple Inc. All rights reserved.
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
# Check that linker does/doesnot apply optimization hints
#

all: all-${ARCH}

all-i386: skip
all-x86_64: skip
all-armv6: skip 
all-armv7: skip

all-arm64: AdrpAdd AdrpAddLdr AdrpLdr AdrpLdrGotLdr AdrpLdrGotLdrField AdrpAddStr AdrpLdrGotStr AdrpLdrGot

main.o:
	${CC} ${CCFLAGS} main.s -c -o main.o
	
AdrpAdd:	main.o
	# test ADRP/ADD -> ADR when target is in __TEXT
	${CC} ${CCFLAGS} AdrpAdd.s -c -o AdrpAdd1.o -DFOO_AS_CONST=1
	${CC} ${CCFLAGS} AdrpAdd1.o main.o -o AdrpAdd1.exe
	${OTOOL} -tV AdrpAdd1.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAdd1.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAdd1.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	# test ADRP/ADD -> ADR when target is in __DATA and main executable
	${CC} ${CCFLAGS} AdrpAdd.s -c -o AdrpAdd2.o -DFOO_AS_DATA=1
	${CC} ${CCFLAGS} AdrpAdd2.o main.o -o AdrpAdd2.exe
	${OTOOL} -tV AdrpAdd2.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAdd2.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAdd2.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	# test ADRP/ADD -> ADR when target is in __DATA and dylib
	${CC} ${CCFLAGS} AdrpAdd.s -c -o AdrpAdd3.o -DFOO_AS_DATA=1
	${CC} ${CCFLAGS} AdrpAdd3.o -dynamiclib -o AdrpAdd3.dylib
	${OTOOL} -tV AdrpAdd3.dylib | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAdd3.dylib | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAdd3.dylib | grep 'add	x0' | ${FAIL_IF_STDIN}
	# test ADRP/ADD !->  when target is in __DATA and dylib in shared region
	${CC} ${CCFLAGS} AdrpAdd.s -c -o AdrpAdd4.o -DFOO_AS_DATA=1
	${CC} ${CCFLAGS} AdrpAdd4.o -dynamiclib -o AdrpAdd4.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAdd4.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAdd4.dylib | grep 'add	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAdd4.dylib | grep 'adr	x0' | ${FAIL_IF_STDIN}
	# test ADRP/ADD !->  when target is in __TEXT but too far
	${CC} ${CCFLAGS} AdrpAdd.s -c -o AdrpAdd5.o -DFOO_AS_CONST=1 -DPADDING
	${CC} ${CCFLAGS} AdrpAdd5.o main.o -o AdrpAdd5.exe
	${OTOOL} -tV AdrpAdd5.exe | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAdd5.exe | grep 'add	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAdd5.exe | grep 'adr	x0' | ${FAIL_IF_STDIN}
	


AdrpAddLdr: AdrpAddLdr-ldr AdrpAddLdr-far AdrpAddLdr-seg AdrpAddLdr-align AdrpAddLdr-addend
	true

AdrpAddLdr-ldr: main.o
	# test ADRP/ADD/LD -> ADR/LDR when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-ldr-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 
	${CC} ${CCFLAGS} AdrpAddLdr-ldr-g8.o main.o -o AdrpAddLdr-ldr-g8.exe 
	${OTOOL} -tV AdrpAddLdr-ldr-g8.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-ldr-g8.exe | grep 'ldr\s*b1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-ldr-g8.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/LD -> ADR/LDR when target is in __TEXT for 16-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-ldr-g16.o -DFOO_AS_CONST -DLOAD_GPR_16
	${CC} ${CCFLAGS} AdrpAddLdr-ldr-g16.o main.o -o AdrpAddLdr-ldr-g16.exe
	${OTOOL} -tV AdrpAddLdr-ldr-g16.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-ldr-g16.exe | grep 'ldr\s*h1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-ldr-g16.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/LD -> LDR when target is in __TEXT for 32-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-ldr-g32.o -DFOO_AS_CONST -DLOAD_GPR_32
	${CC} ${CCFLAGS} AdrpAddLdr-ldr-g32.o main.o -o AdrpAddLdr-ldr-g32.exe
	${OTOOL} -tV AdrpAddLdr-ldr-g32.exe | grep 'ldr	w1, _foo' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-ldr-g32.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-ldr-g32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/LD -> LDR when target is in __TEXT for 64-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-ldr-g64.o -DFOO_AS_CONST -DLOAD_GPR_64
	${CC} ${CCFLAGS} AdrpAddLdr-ldr-g64.o main.o -o AdrpAddLdr-ldr-g64.exe
#	${OTOOL} -tV AdrpAddLdr-ldr-g64.exe | grep 'ldr	x1, _foo' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-ldr-g64.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-ldr-g64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/LD -> LDR when target is in __TEXT for 32-bit fp load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-ldr-f32.o -DFOO_AS_CONST -DLOAD_FPR_32
	${CC} ${CCFLAGS} AdrpAddLdr-ldr-f32.o main.o -o AdrpAddLdr-ldr-f32.exe
	${OTOOL} -tV AdrpAddLdr-ldr-f32.exe | grep 'ldr	s1, _foo' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-ldr-f32.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-ldr-f32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/LD -> LDR when target is in __TEXT for 64-bit fp load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-ldr-f64.o -DFOO_AS_CONST -DLOAD_FPR_64
	${CC} ${CCFLAGS} AdrpAddLdr-ldr-f64.o main.o -o AdrpAddLdr-ldr-f64.exe
	${OTOOL} -tV AdrpAddLdr-ldr-f64.exe | grep 'ldr	d1, _foo' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-ldr-f64.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-ldr-f64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/LD -> LDR when target is in __TEXT for 128-bit vec load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-ldr-v128.o -DFOO_AS_CONST -DLOAD_VEC_128
	${CC} ${CCFLAGS} AdrpAddLdr-ldr-v128.o main.o -o AdrpAddLdr-ldr-v128.exe
	${OTOOL} -tV AdrpAddLdr-ldr-v128.exe | grep 'ldr	q1, _foo' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-ldr-v128.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-ldr-v128.exe | grep 'add	x0' | ${FAIL_IF_STDIN}


AdrpAddLdr-far: main.o
	# test ADRP/ADD/LD -> ADRP/LD when target is far for 8-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-far-g8.o -DFOO_AS_CONST -DLOAD_GPR_8  -DPADDING
	${CC} ${CCFLAGS} AdrpAddLdr-far-g8.o main.o -o AdrpAddLdr-far-g8.exe
	${OTOOL} -tV AdrpAddLdr-far-g8.exe | grep 'ldr	b1, \[x0, ' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-far-g8.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/LD -> ADRP/LD when target is far for 16-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-far-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DPADDING
	${CC} ${CCFLAGS} AdrpAddLdr-far-g16.o main.o -o AdrpAddLdr-far-g16.exe
	${OTOOL} -tV AdrpAddLdr-far-g16.exe | grep 'ldr	h1, \[x0, ' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-far-g16.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/LD -> ADRP/LD when target is far for 32-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-far-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DPADDING
	${CC} ${CCFLAGS} AdrpAddLdr-far-g32.o main.o -o AdrpAddLdr-far-g32.exe
	${OTOOL} -tV AdrpAddLdr-far-g32.exe | grep 'ldr	w1, \[x0, ' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-far-g32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/LD -> ADRP/LD when target is far for 64-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-far-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DPADDING
	${CC} ${CCFLAGS} AdrpAddLdr-far-g64.o main.o -o AdrpAddLdr-far-g64.exe
	${OTOOL} -tV AdrpAddLdr-far-g64.exe | grep 'ldr	x1, \[x0, ' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-far-g64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/LD -> ADRP/LD when target is far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-far-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DPADDING
	${CC} ${CCFLAGS} AdrpAddLdr-far-f32.o main.o -o AdrpAddLdr-far-f32.exe
	${OTOOL} -tV AdrpAddLdr-far-f32.exe | grep 'ldr	s1, \[x0, ' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-far-f32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/LD -> ADRP/LD when target is far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-far-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DPADDING
	${CC} ${CCFLAGS} AdrpAddLdr-far-f64.o main.o -o AdrpAddLdr-far-f64.exe
	${OTOOL} -tV AdrpAddLdr-far-f64.exe | grep 'ldr	d1, \[x0, ' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-far-f64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/LD -> ADRP/LD when target is far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-far-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DPADDING
	${CC} ${CCFLAGS} AdrpAddLdr-far-v128.o main.o -o AdrpAddLdr-far-v128.exe
	${OTOOL} -tV AdrpAddLdr-far-v128.exe | grep 'ldr	q1, \[x0, ' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-far-v128.exe | grep 'add	x0' | ${FAIL_IF_STDIN}



AdrpAddLdr-align: main.o
	# test ADRP/ADD/LD -> ADR/LD when target is for 8-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-align-g8.o -DFOO_AS_DATA -DLOAD_GPR_8 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddLdr-align-g8.o main.o -o AdrpAddLdr-align-g8.exe
	${OTOOL} -tV AdrpAddLdr-align-g8.exe | grep 'adr	x0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-align-g8.exe | grep 'ldr\tb1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-align-g8.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/LD -> ADR/LD when target is not aligned for 16-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-align-g16.o -DFOO_AS_DATA -DLOAD_GPR_16 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddLdr-align-g16.o main.o -o AdrpAddLdr-align-g16.exe
	${OTOOL} -tV AdrpAddLdr-align-g16.exe | grep 'adr	x0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-align-g16.exe | grep 'ldr	h1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-align-g16.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/LD -> ADR/LD  when target is not aligned for 32-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-align-g32.o -DFOO_AS_DATA -DLOAD_GPR_32 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddLdr-align-g32.o main.o -o AdrpAddLdr-align-g32.exe
	${OTOOL} -tV AdrpAddLdr-align-g32.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-align-g32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-align-g32.exe | grep 'ldr	w1, \[x0\]' | ${FAIL_IF_EMPTY}

	# test ADRP/ADD/LD -> ADR/LD  when target is not aligned for 64-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-align-g64.o -DFOO_AS_DATA -DLOAD_GPR_64 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddLdr-align-g64.o main.o -o AdrpAddLdr-align-g64.exe
	${OTOOL} -tV AdrpAddLdr-align-g64.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-align-g64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-align-g64.exe | grep 'ldr	x1, \[x0\]' | ${FAIL_IF_EMPTY}

	# test ADRP/ADD/LD -> ADR/LD  when target is not aligned for 32-bit fp load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-align-f32.o -DFOO_AS_DATA -DLOAD_FPR_32 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddLdr-align-f32.o main.o -o AdrpAddLdr-align-f32.exe
	${OTOOL} -tV AdrpAddLdr-align-f32.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-align-f32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-align-f32.exe | grep 'ldr	s1, \[x0\]' | ${FAIL_IF_EMPTY}

	# test ADRP/ADD/LD -> ADR/LD  when target is not aligned for 64-bit fp load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-align-f64.o -DFOO_AS_DATA -DLOAD_FPR_64 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddLdr-align-f64.o main.o -o AdrpAddLdr-align-f64.exe
	${OTOOL} -tV AdrpAddLdr-align-f64.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-align-f64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-align-f64.exe | grep 'ldr	d1, \[x0\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/ADD/LD -> ADR/LD  when target is not aligned for 128-bit vec load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-align-v128.o -DFOO_AS_DATA -DLOAD_VEC_128 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddLdr-align-v128.o main.o -o AdrpAddLdr-align-v128.exe
	${OTOOL} -tV AdrpAddLdr-align-v128.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-align-v128.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-align-v128.exe | grep 'ldr	q1, \[x0\]' | ${FAIL_IF_EMPTY}


	
AdrpAddLdr-seg: 	
	# test ADRP/ADD/LD -> ADRP/LD when target is in movable segment for 8-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-seg-g8.o -DFOO_AS_DATA -DLOAD_GPR_8 
	${CC} ${CCFLAGS} AdrpAddLdr-seg-g8.o -dynamiclib -o AdrpAddLdr-seg-g8.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddLdr-seg-g8.dylib | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-seg-g8.dylib | grep 'add\s*x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-seg-g8.dylib | grep 'ldr\s*b1, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/ADD/LD -> ADRP/LD when target is in movable segment for 16-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-seg-g16.o -DFOO_AS_DATA -DLOAD_GPR_16 
	${CC} ${CCFLAGS} AdrpAddLdr-seg-g16.o -dynamiclib -o AdrpAddLdr-seg-g16.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddLdr-seg-g16.dylib | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-seg-g16.dylib | grep 'add\s*x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-seg-g16.dylib | grep 'ldr\s*h1, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/ADD/LD -> ADRP/LD  when target is in movable segment for 32-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-seg-g32.o -DFOO_AS_DATA -DLOAD_GPR_32 
	${CC} ${CCFLAGS} AdrpAddLdr-seg-g32.o -dynamiclib -o AdrpAddLdr-seg-g32.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddLdr-seg-g32.dylib | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-seg-g32.dylib | grep 'add\s*x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-seg-g32.dylib | grep 'ldr\s*w1, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/ADD/LD -> ADRP/LD  when target is in movable segment for 64-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-seg-g64.o -DFOO_AS_DATA -DLOAD_GPR_64 
	${CC} ${CCFLAGS} AdrpAddLdr-seg-g64.o -dynamiclib -o AdrpAddLdr-seg-g64.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddLdr-seg-g64.dylib | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-seg-g64.dylib | grep 'add\s*x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-seg-g64.dylib | grep 'ldr\s*x1, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/ADD/LD -> ADRP/LD  when target is in movable segment for 32-bit fp load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-seg-f32.o -DFOO_AS_DATA -DLOAD_FPR_32 
	${CC} ${CCFLAGS} AdrpAddLdr-seg-f32.o -dynamiclib -o AdrpAddLdr-seg-f32.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddLdr-seg-f32.dylib | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-seg-f32.dylib | grep 'add\s*x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-seg-f32.dylib | grep 'ldr\s*s1, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/ADD/LD -> ADRP/LD  when target is in movable segment for 64-bit fp load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-seg-f64.o -DFOO_AS_DATA -DLOAD_FPR_64 
	${CC} ${CCFLAGS} AdrpAddLdr-seg-f64.o -dynamiclib -o AdrpAddLdr-seg-f64.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddLdr-seg-f64.dylib | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-seg-f64.dylib | grep 'add\s*x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-seg-f64.dylib | grep 'ldr\s*d1, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/ADD/LD -> ADRP/LD  when target is in movable segmentfor 128-bit vec load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-seg-v128.o -DFOO_AS_DATA -DLOAD_VEC_128 
	${CC} ${CCFLAGS} AdrpAddLdr-seg-v128.o -dynamiclib -o AdrpAddLdr-seg-v128.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddLdr-seg-v128.dylib | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-seg-v128.dylib | grep 'add\s*x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-seg-v128.dylib | grep 'ldr\s*q1, \[x0,' | ${FAIL_IF_EMPTY}



AdrpAddLdr-addend:	
	# test ADRP/ADD/LD -> ADRP/LD when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-addend-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddLdr-addend-g8.o main.o -o AdrpAddLdr-addend-g8.exe
	${OTOOL} -tV AdrpAddLdr-addend-g8.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-addend-g8.exe | grep 'ldr\tb1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-addend-g8.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/LD -> ADRP/LD when target is in __TEXT for 16-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-addend-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddLdr-addend-g16.o main.o -o AdrpAddLdr-addend-g16.exe
	${OTOOL} -tV AdrpAddLdr-addend-g16.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-addend-g16.exe | grep 'ldr\th1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-addend-g16.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/LD -> LDR when target is in __TEXT for 32-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-addend-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddLdr-addend-g32.o main.o -o AdrpAddLdr-addend-g32.exe
	${OTOOL} -tV AdrpAddLdr-addend-g32.exe | grep 'ldr	w1, _8foo8' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-addend-g32.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-addend-g32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/LD -> LDR when target is in __TEXT for 64-bit load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-addend-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddLdr-addend-g64.o main.o -o AdrpAddLdr-addend-g64.exe
	${OTOOL} -tV AdrpAddLdr-addend-g64.exe | grep 'ldr	x1, _8foo8' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-addend-g64.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-addend-g64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/LD -> LDR when target is in __TEXT for 32-bit fp load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-addend-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddLdr-addend-f32.o main.o -o AdrpAddLdr-addend-f32.exe
	${OTOOL} -tV AdrpAddLdr-addend-f32.exe | grep 'ldr	s1, _8foo8' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-addend-f32.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-addend-f32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/LD -> LDR when target is in __TEXT for 64-bit fp load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-addend-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddLdr-addend-f64.o main.o -o AdrpAddLdr-addend-f64.exe
	${OTOOL} -tV AdrpAddLdr-addend-f64.exe | grep 'ldr	d1, _8foo8' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-addend-f64.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-addend-f64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/LD -> LDR when target is in __TEXT for 128-bit vec load
	${CC} ${CCFLAGS} AdrpAddLdr.s -c -o AdrpAddLdr-addend-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DADDEND=16
	${CC} ${CCFLAGS} AdrpAddLdr-addend-v128.o main.o -o AdrpAddLdr-addend-v128.exe 
	${OTOOL} -tV AdrpAddLdr-addend-v128.exe | grep 'ldr	q1, _16foo16' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddLdr-addend-v128.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpAddLdr-addend-v128.exe | grep 'add	x0' | ${FAIL_IF_STDIN}




AdrpLdr: AdrpLdr-ldr AdrpLdr-seg AdrpLdr-addend
	true

AdrpLdr-ldr: main.o
	# test ADRP/LDR left untouched when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-ldr-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 
	${CC} ${CCFLAGS} AdrpLdr-ldr-g8.o main.o -o AdrpLdr-ldr-g8.exe 
	${OTOOL} -tV AdrpLdr-ldr-g8.exe | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-ldr-g8.exe | grep 'ldr\tb1, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR left untouched when target is in __TEXT for 16-bit load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-ldr-g16.o -DFOO_AS_CONST -DLOAD_GPR_16
	${CC} ${CCFLAGS} AdrpLdr-ldr-g16.o main.o -o AdrpLdr-ldr-g16.exe
	${OTOOL} -tV AdrpLdr-ldr-g16.exe | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-ldr-g16.exe | grep 'ldr\th1, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR -> LDR when target is in __TEXT for 32-bit load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-ldr-g32.o -DFOO_AS_CONST -DLOAD_GPR_32
	${CC} ${CCFLAGS} AdrpLdr-ldr-g32.o main.o -o AdrpLdr-ldr-g32.exe
	${OTOOL} -tV AdrpLdr-ldr-g32.exe | grep 'ldr	w1, _foo' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-ldr-g32.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdr-ldr-g32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/LDR left untouched when target is in __TEXT for 32-bit load from signed 8-bit value
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-ldr-g32s8.o -DFOO_AS_CONST -DLOAD_GPR_32_S8
	${CC} ${CCFLAGS} AdrpLdr-ldr-g32s8.o main.o -o AdrpLdr-ldr-g32s8.exe
	${OTOOL} -tV AdrpLdr-ldr-g32s8.exe | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-ldr-g32s8.exe | grep 'ldrsb\tw1, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR -> LDR when target is in __TEXT for 32-bit load from signed 16-bit value
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-ldr-g32s16.o -DFOO_AS_CONST -DLOAD_GPR_32_S16
	${CC} ${CCFLAGS} AdrpLdr-ldr-g32s16.o main.o -o AdrpLdr-ldr-g32s16.exe
	${OTOOL} -tV AdrpLdr-ldr-g32s16.exe | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-ldr-g32s16.exe | grep 'ldrsh\tw1, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR -> LDR when target is in __TEXT for 64-bit load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-ldr-g64.o -DFOO_AS_CONST -DLOAD_GPR_64
	${CC} ${CCFLAGS} AdrpLdr-ldr-g64.o main.o -o AdrpLdr-ldr-g64.exe
	${OTOOL} -tV AdrpLdr-ldr-g64.exe | grep 'ldr	x1, _foo' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-ldr-g64.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdr-ldr-g64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/LDR left untouched when target is in __TEXT for 64-bit load from signed 8-bit value
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-ldr-g64s8.o -DFOO_AS_CONST -DLOAD_GPR_64_S8
	${CC} ${CCFLAGS} AdrpLdr-ldr-g64s8.o main.o -o AdrpLdr-ldr-g64s8.exe
	${OTOOL} -tV AdrpLdr-ldr-g64s8.exe | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-ldr-g64s8.exe | grep 'ldrsb\tx1, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR left untouched when target is in __TEXT for 64-bit load from signed 16-bit value
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-ldr-g64s16.o -DFOO_AS_CONST -DLOAD_GPR_64_S16
	${CC} ${CCFLAGS} AdrpLdr-ldr-g64s16.o main.o -o AdrpLdr-ldr-g64s16.exe
	${OTOOL} -tV AdrpLdr-ldr-g64s16.exe | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-ldr-g64s16.exe | grep 'ldrsh\tx1, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR -> LDR when target is in __TEXT for 64-bit load from signed 32-bit value
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-ldr-g64s32.o -DFOO_AS_CONST -DLOAD_GPR_64_S32
	${CC} ${CCFLAGS} AdrpLdr-ldr-g64s32.o main.o -o AdrpLdr-ldr-g64s32.exe
	${OTOOL} -tV AdrpLdr-ldr-g64s32.exe | grep 'ldrsw	x1, _foo' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-ldr-g64s32.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdr-ldr-g64s32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/LDR -> LDR when target is in __TEXT for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-ldr-f32.o -DFOO_AS_CONST -DLOAD_FPR_32
	${CC} ${CCFLAGS} AdrpLdr-ldr-f32.o main.o -o AdrpLdr-ldr-f32.exe
	${OTOOL} -tV AdrpLdr-ldr-f32.exe | grep 'ldr	s1, _foo' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-ldr-f32.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdr-ldr-f32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/LDR -> LDR when target is in __TEXT for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-ldr-f64.o -DFOO_AS_CONST -DLOAD_FPR_64
	${CC} ${CCFLAGS} AdrpLdr-ldr-f64.o main.o -o AdrpLdr-ldr-f64.exe
	${OTOOL} -tV AdrpLdr-ldr-f64.exe | grep 'ldr	d1, _foo' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-ldr-f64.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdr-ldr-f64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/LDR -> LDR when target is in __TEXT for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-ldr-v128.o -DFOO_AS_CONST -DLOAD_VEC_128
	${CC} ${CCFLAGS} AdrpLdr-ldr-v128.o main.o -o AdrpLdr-ldr-v128.exe
	${OTOOL} -tV AdrpLdr-ldr-v128.exe | grep 'ldr	q1, _foo' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-ldr-v128.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdr-ldr-v128.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

AdrpLdr-seg: 
	# test ADRP/LDR left untouched when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-seg-g8.o -DFOO_AS_DATA -DLOAD_GPR_8 
	${CC} ${CCFLAGS} AdrpLdr-seg-g8.o  -dynamiclib -o AdrpLdr-seg-g8.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpLdr-seg-g8.dylib | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-seg-g8.dylib | grep 'ldr\tb1, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR left untouched when target is in __TEXT for 16-bit load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-seg-g16.o -DFOO_AS_DATA -DLOAD_GPR_16
	${CC} ${CCFLAGS} AdrpLdr-seg-g16.o  -dynamiclib -o AdrpLdr-seg-g16.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpLdr-seg-g16.dylib | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-seg-g16.dylib | grep 'ldr\th1, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR -> LDR when target is in __TEXT for 32-bit load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-seg-g32.o -DFOO_AS_DATA -DLOAD_GPR_32
	${CC} ${CCFLAGS} AdrpLdr-seg-g32.o  -dynamiclib -o AdrpLdr-seg-g32.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpLdr-seg-g32.dylib | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-seg-g32.dylib | grep 'ldr\tw1, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR -> LDR when target is in __TEXT for 64-bit load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-seg-g64.o -DFOO_AS_DATA -DLOAD_GPR_64
	${CC} ${CCFLAGS} AdrpLdr-seg-g64.o  -dynamiclib -o AdrpLdr-seg-g64.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpLdr-seg-g64.dylib | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-seg-g64.dylib | grep 'ldr\tx1, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR -> LDR when target is in __TEXT for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-seg-f32.o -DFOO_AS_DATA -DLOAD_FPR_32
	${CC} ${CCFLAGS} AdrpLdr-seg-f32.o  -dynamiclib -o AdrpLdr-seg-f32.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpLdr-seg-f32.dylib | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-seg-f32.dylib | grep 'ldr\ts1, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR -> LDR when target is in __TEXT for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-seg-f64.o -DFOO_AS_DATA -DLOAD_FPR_64
	${CC} ${CCFLAGS} AdrpLdr-seg-f64.o  -dynamiclib -o AdrpLdr-seg-f64.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpLdr-seg-f64.dylib | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-seg-f64.dylib | grep 'ldr\td1, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR -> LDR when target is in __TEXT for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-seg-v128.o -DFOO_AS_DATA -DLOAD_VEC_128
	${CC} ${CCFLAGS} AdrpLdr-seg-v128.o  -dynamiclib -o AdrpLdr-seg-v128.dylib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpLdr-seg-v128.dylib | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-seg-v128.dylib | grep 'ldr\tq1, \[x0,' | ${FAIL_IF_EMPTY}

AdrpLdr-addend: main.o
	# test ADRP/LDR left untouched when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-addend-g8.o -DFOO_AS_DATA -DLOAD_GPR_8 -DADDEND=+8
	${CC} ${CCFLAGS} AdrpLdr-addend-g8.o main.o  -o AdrpLdr-addend-g8.exe 
	${OTOOL} -tV AdrpLdr-addend-g8.exe | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-addend-g8.exe | grep 'ldr\tb1, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR left untouched when target is in __TEXT for 16-bit load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-addend-g16.o -DFOO_AS_DATA -DLOAD_GPR_16 -DADDEND=+8
	${CC} ${CCFLAGS} AdrpLdr-addend-g16.o main.o  -o AdrpLdr-addend-g16.exe 
	${OTOOL} -tV AdrpLdr-addend-g16.exe | grep 'adrp\tx0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-addend-g16.exe | grep 'ldr\th1, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR -> LDR when target is in __TEXT for 32-bit load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-addend-g32.o -DFOO_AS_DATA -DLOAD_GPR_32 -DADDEND=+8
	${CC} ${CCFLAGS} AdrpLdr-addend-g32.o main.o -o AdrpLdr-addend-g32.exe
	${OTOOL} -tV AdrpLdr-addend-g32.exe | grep 'ldr	w1, _8foo8' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-addend-g32.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdr-addend-g32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/LDR -> LDR when target is in __TEXT for 64-bit load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-addend-g64.o -DFOO_AS_DATA -DLOAD_GPR_64 -DADDEND=+8
	${CC} ${CCFLAGS} AdrpLdr-addend-g64.o main.o -o AdrpLdr-addend-g64.exe
	${OTOOL} -tV AdrpLdr-addend-g64.exe | grep 'ldr	x1, _8foo8' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-addend-g64.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdr-addend-g64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/LDR -> LDR when target is in __TEXT for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-addend-f32.o -DFOO_AS_DATA -DLOAD_FPR_32 -DADDEND=+8
	${CC} ${CCFLAGS} AdrpLdr-addend-f32.o main.o -o AdrpLdr-addend-f32.exe
	${OTOOL} -tV AdrpLdr-addend-f32.exe | grep 'ldr	s1, _8foo8' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-addend-f32.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdr-addend-f32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/LDR -> LDR when target is in __TEXT for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-addend-f64.o -DFOO_AS_DATA -DLOAD_FPR_64 -DADDEND=+8
	${CC} ${CCFLAGS} AdrpLdr-addend-f64.o main.o -o AdrpLdr-addend-f64.exe
	${OTOOL} -tV AdrpLdr-addend-f64.exe | grep 'ldr	d1, _8foo8' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-addend-f64.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdr-addend-f64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/LDR -> LDR when target is in __TEXT for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdr.s -c -o AdrpLdr-addend-v128.o -DFOO_AS_DATA -DLOAD_VEC_128 -DADDEND=+16
	${CC} ${CCFLAGS} AdrpLdr-addend-v128.o main.o -o AdrpLdr-addend-v128.exe
	${OTOOL} -tV AdrpLdr-addend-v128.exe | grep 'ldr	q1, _16foo16' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdr-addend-v128.exe | grep 'adrp	x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdr-addend-v128.exe | grep 'add	x0' | ${FAIL_IF_STDIN}


AdrpLdrGotLdr: AdrpLdrGotLdr-extern AdrpLdrGotLdr-externfargot AdrpLdrGotLdr-near AdrpLdrGotLdr-far AdrpLdrGotLdr-nearunaligned AdrpLdrGotLdr-farunaligned
	true

AdrpLdrGotLdr-extern: main.o
	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close  for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-extern-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdr-extern-g8.o main.o -o AdrpLdrGotLdr-extern-g8.exe 
	${OTOOL} -tV AdrpLdrGotLdr-extern-g8.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-extern-g8.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-extern-g8.exe | grep 'ldr\tb2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close  for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-extern-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdr-extern-g16.o main.o -o AdrpLdrGotLdr-extern-g16.exe 
	${OTOOL} -tV AdrpLdrGotLdr-extern-g16.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-extern-g16.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-extern-g16.exe | grep 'ldr\th2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-extern-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdr-extern-g32.o main.o -o AdrpLdrGotLdr-extern-g32.exe 
	${OTOOL} -tV AdrpLdrGotLdr-extern-g32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-extern-g32.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-extern-g32.exe | grep 'ldr\tw2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-extern-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdr-extern-g64.o main.o -o AdrpLdrGotLdr-extern-g64.exe 
	${OTOOL} -tV AdrpLdrGotLdr-extern-g64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-extern-g64.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-extern-g64.exe | grep 'ldr\tx2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-extern-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdr-extern-f32.o main.o -o AdrpLdrGotLdr-extern-f32.exe 
	${OTOOL} -tV AdrpLdrGotLdr-extern-f32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-extern-f32.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-extern-f32.exe | grep 'ldr\ts2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-extern-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdr-extern-f64.o main.o -o AdrpLdrGotLdr-extern-f64.exe 
	${OTOOL} -tV AdrpLdrGotLdr-extern-f64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-extern-f64.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-extern-f64.exe | grep 'ldr\td2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-extern-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdr-extern-v128.o main.o -o AdrpLdrGotLdr-extern-v128.exe 
	${OTOOL} -tV AdrpLdrGotLdr-extern-v128.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-extern-v128.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-extern-v128.exe | grep 'ldr\tq2, \[x1\]' | ${FAIL_IF_EMPTY}


AdrpLdrGotLdr-externfargot: main.o
	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-externfargot-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-externfargot-g8.o main.o -o AdrpLdrGotLdr-externfargot-g8.exe 
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-g8.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-g8.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-g8.exe | grep 'ldr\tb2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-externfargot-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-externfargot-g16.o main.o -o AdrpLdrGotLdr-externfargot-g16.exe 
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-g16.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-g16.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-g16.exe | grep 'ldr\th2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR left untouched when target is external and GOT slot farfor 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-externfargot-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-externfargot-g32.o main.o -o AdrpLdrGotLdr-externfargot-g32.exe 
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-g32.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-g32.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-g32.exe | grep 'ldr\tw2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-externfargot-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-externfargot-g64.o main.o -o AdrpLdrGotLdr-externfargot-g64.exe 
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-g64.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-g64.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-g64.exe | grep 'ldr\tx2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-externfargot-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-externfargot-f32.o main.o -o AdrpLdrGotLdr-externfargot-f32.exe 
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-f32.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-f32.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-f32.exe | grep 'ldr\ts2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-externfargot-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-externfargot-f64.o main.o -o AdrpLdrGotLdr-externfargot-f64.exe 
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-f64.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-f64.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-f64.exe | grep 'ldr\td2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -left untouched when target is external and GOT slot far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-externfargot-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-externfargot-v128.o main.o -o AdrpLdrGotLdr-externfargot-v128.exe 
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-v128.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-v128.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-externfargot-v128.exe | grep 'ldr\tq2, \[x1\]' | ${FAIL_IF_EMPTY}


AdrpLdrGotLdr-near: main.o 
	# test ADRP/LDR left untouched when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-near-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 
	${CC} ${CCFLAGS} AdrpLdrGotLdr-near-g8.o main.o -o AdrpLdrGotLdr-near-g8.exe 
	${OTOOL} -tV AdrpLdrGotLdr-near-g8.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-near-g8.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-near-g8.exe | grep 'ldr\tb2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-near-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 
	${CC} ${CCFLAGS} AdrpLdrGotLdr-near-g16.o main.o -o AdrpLdrGotLdr-near-g16.exe 
	${OTOOL} -tV AdrpLdrGotLdr-near-g16.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-near-g16.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-near-g16.exe | grep 'ldr\th2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR literal when target close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-near-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 
	${CC} ${CCFLAGS} AdrpLdrGotLdr-near-g32.o main.o -o AdrpLdrGotLdr-near-g32.exe 
	${OTOOL} -tV AdrpLdrGotLdr-near-g32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-near-g32.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-near-g32.exe | grep 'ldr\tw2, _foo' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-near-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 
	${CC} ${CCFLAGS} AdrpLdrGotLdr-near-g64.o main.o -o AdrpLdrGotLdr-near-g64.exe 
	${OTOOL} -tV AdrpLdrGotLdr-near-g64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-near-g64.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-near-g64.exe | grep 'ldr\tx2, _foo' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-near-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 
	${CC} ${CCFLAGS} AdrpLdrGotLdr-near-f32.o main.o -o AdrpLdrGotLdr-near-f32.exe 
	${OTOOL} -tV AdrpLdrGotLdr-near-f32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-near-f32.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-near-f32.exe | grep 'ldr\ts2, _foo' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-near-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 
	${CC} ${CCFLAGS} AdrpLdrGotLdr-near-f64.o main.o -o AdrpLdrGotLdr-near-f64.exe 
	${OTOOL} -tV AdrpLdrGotLdr-near-f64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-near-f64.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-near-f64.exe | grep 'ldr\td2, _foo' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -left untouched when target is external and GOT slot far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-near-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 
	${CC} ${CCFLAGS} AdrpLdrGotLdr-near-v128.o main.o -o AdrpLdrGotLdr-near-v128.exe 
	${OTOOL} -tV AdrpLdrGotLdr-near-v128.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-near-v128.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-near-v128.exe | grep 'ldr\tq2, _foo' | ${FAIL_IF_EMPTY}


AdrpLdrGotLdr-nearunaligned: main.o 
	# test ADRP/LDR left untouched when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-nearunaligned-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdr-nearunaligned-g8.o main.o -o AdrpLdrGotLdr-nearunaligned-g8.exe 
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-g8.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-g8.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-g8.exe | grep 'ldr\tb2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-nearunaligned-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdr-nearunaligned-g16.o main.o -o AdrpLdrGotLdr-nearunaligned-g16.exe 
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-g16.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-g16.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-g16.exe | grep 'ldr\th2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR literal when target close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-nearunaligned-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdr-nearunaligned-g32.o main.o -o AdrpLdrGotLdr-nearunaligned-g32.exe 
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-g32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-g32.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-g32.exe | grep 'ldr\tw2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-nearunaligned-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdr-nearunaligned-g64.o main.o -o AdrpLdrGotLdr-nearunaligned-g64.exe 
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-g64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-g64.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-g64.exe | grep 'ldr\tx2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-nearunaligned-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdr-nearunaligned-f32.o main.o -o AdrpLdrGotLdr-nearunaligned-f32.exe 
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-f32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-f32.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-f32.exe | grep 'ldr\ts2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-nearunaligned-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdr-nearunaligned-f64.o main.o -o AdrpLdrGotLdr-nearunaligned-f64.exe 
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-f64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-f64.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-f64.exe | grep 'ldr\td2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -left untouched when target is external and GOT slot far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-nearunaligned-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdr-nearunaligned-v128.o main.o -o AdrpLdrGotLdr-nearunaligned-v128.exe 
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-v128.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-v128.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-nearunaligned-v128.exe | grep 'ldr\tq2, \[x1\]' | ${FAIL_IF_EMPTY}


AdrpLdrGotLdr-far: main.o 
	# test ADRP/LDR left untouched when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-far-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-far-g8.o main.o -o AdrpLdrGotLdr-far-g8.exe 
	${OTOOL} -tV AdrpLdrGotLdr-far-g8.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-far-g8.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-far-g8.exe | grep 'ldr\tb2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-far-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-far-g16.o main.o -o AdrpLdrGotLdr-far-g16.exe  
	${OTOOL} -tV AdrpLdrGotLdr-far-g16.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-far-g16.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-far-g16.exe | grep 'ldr\th2, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR literal when target close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-far-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotLdr-far-g32.o main.o -o AdrpLdrGotLdr-far-g32.exe  
	${OTOOL} -tV AdrpLdrGotLdr-far-g32.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-far-g32.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-far-g32.exe | grep 'ldr\tw2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-far-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotLdr-far-g64.o main.o -o AdrpLdrGotLdr-far-g64.exe  
	${OTOOL} -tV AdrpLdrGotLdr-far-g64.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-far-g64.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-far-g64.exe | grep 'ldr\tx2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-far-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotLdr-far-f32.o main.o -o AdrpLdrGotLdr-far-f32.exe  
	${OTOOL} -tV AdrpLdrGotLdr-far-f32.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-far-f32.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-far-f32.exe | grep 'ldr\ts2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-far-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotLdr-far-f64.o main.o -o AdrpLdrGotLdr-far-f64.exe  
	${OTOOL} -tV AdrpLdrGotLdr-far-f64.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-far-f64.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-far-f64.exe | grep 'ldr\td2, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -left untouched when target is external and GOT slot far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-far-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotLdr-far-v128.o main.o -o AdrpLdrGotLdr-far-v128.exe  
	${OTOOL} -tV AdrpLdrGotLdr-far-v128.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-far-v128.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-far-v128.exe | grep 'ldr\tq2, \[x0,' | ${FAIL_IF_EMPTY}


AdrpLdrGotLdr-farunaligned: main.o 
	# test ADRP/LDR left untouched when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-farunaligned-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-farunaligned-g8.o main.o -o AdrpLdrGotLdr-farunaligned-g8.exe 
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-g8.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-g8.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-g8.exe | grep 'ldr\tb2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-farunaligned-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-farunaligned-g16.o main.o -o AdrpLdrGotLdr-farunaligned-g16.exe 
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-g16.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-g16.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-g16.exe | grep 'ldr\th2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR literal when target close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-farunaligned-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-farunaligned-g32.o main.o -o AdrpLdrGotLdr-farunaligned-g32.exe 
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-g32.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-g32.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-g32.exe | grep 'ldr\tw2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-farunaligned-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-farunaligned-g64.o main.o -o AdrpLdrGotLdr-farunaligned-g64.exe 
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-g64.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-g64.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-g64.exe | grep 'ldr\tx2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-farunaligned-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-farunaligned-f32.o main.o -o AdrpLdrGotLdr-farunaligned-f32.exe 
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-f32.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-f32.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-f32.exe | grep 'ldr\ts2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-farunaligned-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-farunaligned-f64.o main.o -o AdrpLdrGotLdr-farunaligned-f64.exe 
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-f64.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-f64.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-f64.exe | grep 'ldr\td2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -left untouched when target is external and GOT slot far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotLdr.s -c -o AdrpLdrGotLdr-farunaligned-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdr-farunaligned-v128.o main.o -o AdrpLdrGotLdr-farunaligned-v128.exe 
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-v128.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-v128.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdr-farunaligned-v128.exe | grep 'ldr\tq2, \[x1\]' | ${FAIL_IF_EMPTY}


AdrpLdrGotLdrField: AdrpLdrGotLdrField-extern AdrpLdrGotLdrField-externfargot AdrpLdrGotLdrField-near AdrpLdrGotLdrField-far AdrpLdrGotLdrField-nearunaligned AdrpLdrGotLdrField-farunaligned
	true

AdrpLdrGotLdrField-extern: main.o
	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close  for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-extern-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-extern-g8.o main.o -o AdrpLdrGotLdrField-extern-g8.exe
	${OTOOL} -tV AdrpLdrGotLdrField-extern-g8.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-g8.exe | grep 'ldr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-g8.exe | grep 'ldr\s*b2, \[x1, #8\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close  for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-extern-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-extern-g16.o main.o -o AdrpLdrGotLdrField-extern-g16.exe
	${OTOOL} -tV AdrpLdrGotLdrField-extern-g16.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-g16.exe | grep 'ldr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-g16.exe | grep 'ldr\s*h2, \[x1, #8\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-extern-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-extern-g32.o main.o -o AdrpLdrGotLdrField-extern-g32.exe
	${OTOOL} -tV AdrpLdrGotLdrField-extern-g32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-g32.exe | grep 'ldr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-g32.exe | grep 'ldr\s*w2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-extern-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-extern-g64.o main.o -o AdrpLdrGotLdrField-extern-g64.exe
	${OTOOL} -tV AdrpLdrGotLdrField-extern-g64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-g64.exe | grep 'ldr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-g64.exe | grep 'ldr\s*x2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-extern-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-extern-f32.o main.o -o AdrpLdrGotLdrField-extern-f32.exe
	${OTOOL} -tV AdrpLdrGotLdrField-extern-f32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-f32.exe | grep 'ldr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-f32.exe | grep 'ldr\s*s2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-extern-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-extern-f64.o main.o -o AdrpLdrGotLdrField-extern-f64.exe
	${OTOOL} -tV AdrpLdrGotLdrField-extern-f64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-f64.exe | grep 'ldr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-f64.exe | grep 'ldr\s*d2, \[x1, #8\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR/LDR when GOT slot is close for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-extern-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-extern-v128.o main.o -o AdrpLdrGotLdrField-extern-v128.exe
	${OTOOL} -tV AdrpLdrGotLdrField-extern-v128.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-v128.exe | grep 'ldr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-extern-v128.exe | grep 'ldr\s*q2, \[x1, #16\]' | ${FAIL_IF_EMPTY}


AdrpLdrGotLdrField-externfargot: main.o
	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-externfargot-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-externfargot-g8.o main.o -o AdrpLdrGotLdrField-externfargot-g8.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-g8.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-g8.exe | grep 'ldr\s*x1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-g8.exe | grep 'ldr\s*b2, \[x1, #8\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-externfargot-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-externfargot-g16.o main.o -o AdrpLdrGotLdrField-externfargot-g16.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-g16.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-g16.exe | grep 'ldr\s*x1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-g16.exe | grep 'ldr\s*h2, \[x1, #8\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR left untouched when target is external and GOT slot farfor 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-externfargot-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-externfargot-g32.o main.o -o AdrpLdrGotLdrField-externfargot-g32.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-g32.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-g32.exe | grep 'ldr\s*x1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-g32.exe | grep 'ldr\s*w2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-externfargot-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-externfargot-g64.o main.o -o AdrpLdrGotLdrField-externfargot-g64.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-g64.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-g64.exe | grep 'ldr\s*x1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-g64.exe | grep 'ldr\s*x2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-externfargot-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-externfargot-f32.o main.o -o AdrpLdrGotLdrField-externfargot-f32.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-f32.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-f32.exe | grep 'ldr\s*x1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-f32.exe | grep 'ldr\s*s2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-externfargot-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-externfargot-f64.o main.o -o AdrpLdrGotLdrField-externfargot-f64.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-f64.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-f64.exe | grep 'ldr\s*x1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-f64.exe | grep 'ldr\s*d2, \[x1, #8\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -left untouched when target is external and GOT slot far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-externfargot-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-externfargot-v128.o main.o -o AdrpLdrGotLdrField-externfargot-v128.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-v128.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-v128.exe | grep 'ldr\s*x1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-externfargot-v128.exe | grep 'ldr\s*q2, \[x1, #16\]' | ${FAIL_IF_EMPTY}


AdrpLdrGotLdrField-near: main.o 
	# test ADRP/LDR left untouched when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-near-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-near-g8.o main.o -o AdrpLdrGotLdrField-near-g8.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-near-g8.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-g8.exe | grep 'adr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-near-g8.exe | grep 'ldr\s*b2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-near-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-near-g16.o main.o -o AdrpLdrGotLdrField-near-g16.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-near-g16.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-g16.exe | grep 'adr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-near-g16.exe | grep 'ldr\s*h2, \[x1, #8\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR literal when target close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-near-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-near-g32.o main.o -o AdrpLdrGotLdrField-near-g32.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-near-g32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-g32.exe | grep 'ldr\s*x1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-g32.exe | grep 'ldr\s*w2, _foo' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-g32.exe | grep 'ldr\s*w2,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-near-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-near-g64.o main.o -o AdrpLdrGotLdrField-near-g64.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-near-g64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-g64.exe | grep 'ldr\s*x1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-g64.exe | grep 'ldr\s*x2, _foo' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-g64.exe | grep 'ldr\s*x2,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-near-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-near-f32.o main.o -o AdrpLdrGotLdrField-near-f32.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-near-f32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-f32.exe | grep 'ldr\s*x1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-f32.exe | grep 'ldr\s*s2, _foo' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-f32.exe | grep 'ldr\s*s2,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-near-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-near-f64.o main.o -o AdrpLdrGotLdrField-near-f64.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-near-f64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-f64.exe | grep 'ldr\s*x1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-f64.exe | grep 'ldr\s*d2, _foo' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-f64.exe | grep 'ldr\s*d2,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -left untouched when target is external and GOT slot far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-near-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-near-v128.o main.o -o AdrpLdrGotLdrField-near-v128.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-near-v128.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-v128.exe | grep 'ldr\s*x1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-v128.exe | grep 'ldr\s*q2, _foo' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-near-v128.exe | grep 'ldr\s*q2,' | ${FAIL_IF_EMPTY}


AdrpLdrGotLdrField-nearunaligned: main.o 
	# test ADRP/LDR/LDR -> ADR/LDR when target in __TEXT for 8-bit unaligned load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-nearunaligned-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-nearunaligned-g8.o main.o -o AdrpLdrGotLdrField-nearunaligned-g8.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-g8.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-g8.exe | grep 'adr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-g8.exe | grep 'ldr\s*b2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -> ADR/LDR when target in __TEXT for 16-bit unaligned load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-nearunaligned-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-nearunaligned-g16.o main.o -o AdrpLdrGotLdrField-nearunaligned-g16.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-g16.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-g16.exe | grep 'adr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-g16.exe | grep 'ldr\s*h2, \[x1, #8\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> ADR/LDR when target in __TEXT for 32-bit unaligned load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-nearunaligned-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-nearunaligned-g32.o main.o -o AdrpLdrGotLdrField-nearunaligned-g32.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-g32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-g32.exe | grep 'adr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-g32.exe | grep 'ldr\s*w2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -> ADR/LDR when target in __TEXT for 64-bit unaligned load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-nearunaligned-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-nearunaligned-g64.o main.o -o AdrpLdrGotLdrField-nearunaligned-g64.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-g64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-g64.exe | grep 'adr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-g64.exe | grep 'ldr\s*x2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -> ADR/LDR when target in __TEXT for 32-bit FP unaligned load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-nearunaligned-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-nearunaligned-f32.o main.o -o AdrpLdrGotLdrField-nearunaligned-f32.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-f32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-f32.exe | grep 'adr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-f32.exe | grep 'ldr\s*s2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -> ADR/LDR when target in __TEXT for 64-bit FP unaligned load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-nearunaligned-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-nearunaligned-f64.o main.o -o AdrpLdrGotLdrField-nearunaligned-f64.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-f64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-f64.exe | grep 'adr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-f64.exe | grep 'ldr\s*d2, \[x1, #8\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> ADR/LDR when target in __TEXT for 12-bit vec unaligned load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-nearunaligned-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-nearunaligned-v128.o main.o -o AdrpLdrGotLdrField-nearunaligned-v128.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-v128.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-v128.exe | grep 'adr\s*x1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-nearunaligned-v128.exe | grep 'ldr\s*q2, \[x1, #16\]' | ${FAIL_IF_EMPTY}


AdrpLdrGotLdrField-far: main.o 
	# test ADRP/LDR left untouched when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-far-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-far-g8.o main.o -o AdrpLdrGotLdrField-far-g8.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-far-g8.exe | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-far-g8.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-far-g8.exe | grep 'ldr\s*b2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-far-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-far-g16.o main.o -o AdrpLdrGotLdrField-far-g16.exe  
	${OTOOL} -tV AdrpLdrGotLdrField-far-g16.exe | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-far-g16.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-far-g16.exe | grep 'ldr\s*h2, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR literal when target close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-far-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-far-g32.o main.o -o AdrpLdrGotLdrField-far-g32.exe  
	${OTOOL} -tV AdrpLdrGotLdrField-far-g32.exe | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-far-g32.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-far-g32.exe | grep 'ldr\s*w2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-far-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-far-g64.o main.o -o AdrpLdrGotLdrField-far-g64.exe  
	${OTOOL} -tV AdrpLdrGotLdrField-far-g64.exe | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-far-g64.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-far-g64.exe | grep 'ldr\s*x2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-far-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-far-f32.o main.o -o AdrpLdrGotLdrField-far-f32.exe  
	${OTOOL} -tV AdrpLdrGotLdrField-far-f32.exe | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-far-f32.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-far-f32.exe | grep 'ldr\s*s2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-far-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-far-f64.o main.o -o AdrpLdrGotLdrField-far-f64.exe  
	${OTOOL} -tV AdrpLdrGotLdrField-far-f64.exe | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-far-f64.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-far-f64.exe | grep 'ldr\s*d2, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-far-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-far-v128.o main.o -o AdrpLdrGotLdrField-far-v128.exe  
	${OTOOL} -tV AdrpLdrGotLdrField-far-v128.exe | grep 'adrp\s*x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-far-v128.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-far-v128.exe | grep 'ldr\s*q2, \[x0,' | ${FAIL_IF_EMPTY}


AdrpLdrGotLdrField-farunaligned: main.o 
	# test ADRP/LDR left untouched when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-farunaligned-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-farunaligned-g8.o main.o -o AdrpLdrGotLdrField-farunaligned-g8.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-g8.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-g8.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-g8.exe | grep 'ldr\s*b2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-farunaligned-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-farunaligned-g16.o main.o -o AdrpLdrGotLdrField-farunaligned-g16.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-g16.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-g16.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-g16.exe | grep 'ldr\s*h2, \[x1, #8\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR literal when target close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-farunaligned-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-farunaligned-g32.o main.o -o AdrpLdrGotLdrField-farunaligned-g32.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-g32.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-g32.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-g32.exe | grep 'ldr\s*w2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-farunaligned-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-farunaligned-g64.o main.o -o AdrpLdrGotLdrField-farunaligned-g64.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-g64.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-g64.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-g64.exe | grep 'ldr\s*x2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-farunaligned-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-farunaligned-f32.o main.o -o AdrpLdrGotLdrField-farunaligned-f32.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-f32.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-f32.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-f32.exe | grep 'ldr\s*s2, \[x1, #8\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-farunaligned-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-farunaligned-f64.o main.o -o AdrpLdrGotLdrField-farunaligned-f64.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-f64.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-f64.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-f64.exe | grep 'ldr\s*d2, \[x1, #8\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -left untouched when target is external and GOT slot far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotLdrField.s -c -o AdrpLdrGotLdrField-farunaligned-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotLdrField-farunaligned-v128.o main.o -o AdrpLdrGotLdrField-farunaligned-v128.exe 
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-v128.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-v128.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotLdrField-farunaligned-v128.exe | grep 'ldr\s*q2, \[x1, #16\]' | ${FAIL_IF_EMPTY}




AdrpAddStr: AdrpAddStr-base AdrpAddStr-seg AdrpAddStr-align AdrpAddStr-addend AdrpAddStr-faraddend
	true

AdrpAddStr-base: main.o
	# test ADRP/ADD/STR -> ADR/STR for 8-bit store
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-base-g8.o  -DLOAD_GPR_8 
	${CC} ${CCFLAGS} AdrpAddStr-base-g8.o main.o -o AdrpAddStr-base-g8.exe 
	${OTOOL} -tV AdrpAddStr-base-g8.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-g8.exe | grep 'str\tb1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-g8.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/STR -> ADR/STR for 16-bit store
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-base-g16.o  -DLOAD_GPR_16
	${CC} ${CCFLAGS} AdrpAddStr-base-g16.o main.o -o AdrpAddStr-base-g16.exe
	${OTOOL} -tV AdrpAddStr-base-g16.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-g16.exe | grep 'str\th1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-g16.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/STR -> ADR/STR  for 32-bit store
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-base-g32.o  -DLOAD_GPR_32
	${CC} ${CCFLAGS} AdrpAddStr-base-g32.o main.o -o AdrpAddStr-base-g32.exe
	${OTOOL} -tV AdrpAddStr-base-g32.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-g32.exe | grep 'str\tw1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-g32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/STR -> ADR/STR  for 64-bit store
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-base-g64.o  -DLOAD_GPR_64
	${CC} ${CCFLAGS} AdrpAddStr-base-g64.o main.o -o AdrpAddStr-base-g64.exe
	${OTOOL} -tV AdrpAddStr-base-g64.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-g64.exe | grep 'str\tx1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-g64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/STR -> ADR/STR  for 32-bit fp store
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-base-f32.o  -DLOAD_FPR_32
	${CC} ${CCFLAGS} AdrpAddStr-base-f32.o main.o -o AdrpAddStr-base-f32.exe
	${OTOOL} -tV AdrpAddStr-base-f32.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-f32.exe | grep 'str\ts1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-f32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/STR -> ADR/STR  for 64-bit fp store
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-base-f64.o  -DLOAD_FPR_64
	${CC} ${CCFLAGS} AdrpAddStr-base-f64.o main.o -o AdrpAddStr-base-f64.exe
	${OTOOL} -tV AdrpAddStr-base-f64.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-f64.exe | grep 'str\td1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-f64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/STR -> ADR/STR  for 128-bit vec store
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-base-v128.o  -DLOAD_VEC_128
	${CC} ${CCFLAGS} AdrpAddStr-base-v128.o main.o -o AdrpAddStr-base-v128.exe
	${OTOOL} -tV AdrpAddStr-base-v128.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-v128.exe | grep 'str\tq1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-base-v128.exe | grep 'add	x0' | ${FAIL_IF_STDIN}


AdrpAddStr-seg: main.o
	# test ADRP/ADD/STR -> ADRP/STR for 8-bit store
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-seg-g8.o  -DLOAD_GPR_8 
	${CC} ${CCFLAGS} AdrpAddStr-seg-g8.o -o AdrpAddStr-seg-g8.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddStr-seg-g8.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-g8.dylib | grep 'str\tb1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-g8.dylib | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/STR -> ADRP/STR for 16-bit v
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-seg-g16.o  -DLOAD_GPR_16
	${CC} ${CCFLAGS} AdrpAddStr-seg-g16.o -o AdrpAddStr-seg-g16.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddStr-seg-g16.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-g16.dylib | grep 'str\th1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-g16.dylib | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/STR -> ADRP/STR for 32-bit store
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-seg-g32.o  -DLOAD_GPR_32
	${CC} ${CCFLAGS} AdrpAddStr-seg-g32.o -o AdrpAddStr-seg-g32.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddStr-seg-g32.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-g32.dylib | grep 'str\tw1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-g32.dylib | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/STR -> ADRP/STR  for 64-bit store
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-seg-g64.o  -DLOAD_GPR_64
	${CC} ${CCFLAGS} AdrpAddStr-seg-g64.o -o AdrpAddStr-seg-g64.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddStr-seg-g64.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-g64.dylib | grep 'str\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-g64.dylib | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/STR -> ADRP/STR  for 32-bit fp store
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-seg-f32.o  -DLOAD_FPR_32
	${CC} ${CCFLAGS} AdrpAddStr-seg-f32.o -o AdrpAddStr-seg-f32.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddStr-seg-f32.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-f32.dylib | grep 'str\ts1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-f32.dylib | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/STR -> ADRP/STR  for 64-bit fp store
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-seg-f64.o  -DLOAD_FPR_64
	${CC} ${CCFLAGS} AdrpAddStr-seg-f64.o -o AdrpAddStr-seg-f64.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddStr-seg-f64.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-f64.dylib | grep 'str\td1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-f64.dylib | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/STR -> ADRP/STR  for 128-bit vec store
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-seg-v128.o  -DLOAD_VEC_128
	${CC} ${CCFLAGS} AdrpAddStr-seg-v128.o -o AdrpAddStr-seg-v128.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddStr-seg-v128.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-v128.dylib | grep 'str\tq1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-seg-v128.dylib | grep 'add	x0' | ${FAIL_IF_STDIN}


AdrpAddStr-align: main.o
	# test ADRP/ADD/STR -> ADR/STR when target is for 8-bit load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-align-g8.o  -DLOAD_GPR_8 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddStr-align-g8.o main.o -o AdrpAddStr-align-g8.exe
	${OTOOL} -tV AdrpAddStr-align-g8.exe | grep 'adr	x0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-g8.exe | grep 'str\tb1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-g8.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/STR -> ADR/STR when target is not aligned for 16-bit load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-align-g16.o  -DLOAD_GPR_16 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddStr-align-g16.o main.o -o AdrpAddStr-align-g16.exe
	${OTOOL} -tV AdrpAddStr-align-g16.exe | grep 'adr	x0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-g16.exe | grep 'str	h1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-g16.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/STR -> ADR/STR when target is not aligned for 32-bit load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-align-g32.o  -DLOAD_GPR_32 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddStr-align-g32.o main.o -o AdrpAddStr-align-g32.exe
	${OTOOL} -tV AdrpAddStr-align-g32.exe | grep 'adr	x0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-g32.exe | grep 'str	w1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-g32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/STR -> ADR/STR  when target is not aligned for 64-bit load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-align-g64.o  -DLOAD_GPR_64 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddStr-align-g64.o main.o -o AdrpAddStr-align-g64.exe
	${OTOOL} -tV AdrpAddStr-align-g64.exe | grep 'adr	x0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-g64.exe | grep 'str	x1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-g64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/STR -> ADR/STR  when target is not aligned for 32-bit fp load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-align-f32.o  -DLOAD_FPR_32 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddStr-align-f32.o main.o -o AdrpAddStr-align-f32.exe
	${OTOOL} -tV AdrpAddStr-align-f32.exe | grep 'adr	x0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-f32.exe | grep 'str	s1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-f32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/STR -> ADR/STR  when target is not aligned for 64-bit fp load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-align-f64.o  -DLOAD_FPR_64 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddStr-align-f64.o main.o -o AdrpAddStr-align-f64.exe
	${OTOOL} -tV AdrpAddStr-align-f64.exe | grep 'adr	x0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-f64.exe | grep 'str	d1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-f64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/STR -> ADR/STR  when target is not aligned for 128-bit vec load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-align-v128.o  -DLOAD_VEC_128 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpAddStr-align-v128.o main.o -o AdrpAddStr-align-v128.exe
	${OTOOL} -tV AdrpAddStr-align-v128.exe | grep 'adr	x0,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-v128.exe | grep 'str	q1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-align-v128.exe | grep 'add	x0' | ${FAIL_IF_STDIN}



AdrpAddStr-addend:	
	# test ADRP/ADD/STR -> ADRP/LD when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-addend-g8.o  -DLOAD_GPR_8 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddStr-addend-g8.o main.o -o AdrpAddStr-addend-g8.exe
	${OTOOL} -tV AdrpAddStr-addend-g8.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-g8.exe | grep 'str\tb1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-g8.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/STR -> ADRP/LD when target is in __TEXT for 16-bit load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-addend-g16.o  -DLOAD_GPR_16 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddStr-addend-g16.o main.o -o AdrpAddStr-addend-g16.exe
	${OTOOL} -tV AdrpAddStr-addend-g16.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-g16.exe | grep 'str\th1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-g16.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/STR -> LDR when target is in __TEXT for 32-bit load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-addend-g32.o  -DLOAD_GPR_32 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddStr-addend-g32.o main.o -o AdrpAddStr-addend-g32.exe
	${OTOOL} -tV AdrpAddStr-addend-g32.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-g32.exe | grep 'str\tw1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-g32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/STR -> LDR when target is in __TEXT for 64-bit load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-addend-g64.o  -DLOAD_GPR_64 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddStr-addend-g64.o main.o -o AdrpAddStr-addend-g64.exe
	${OTOOL} -tV AdrpAddStr-addend-g64.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-g64.exe | grep 'str\tx1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-g64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/STR -> LDR when target is in __TEXT for 32-bit fp load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-addend-f32.o  -DLOAD_FPR_32 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddStr-addend-f32.o main.o -o AdrpAddStr-addend-f32.exe
	${OTOOL} -tV AdrpAddStr-addend-f32.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-f32.exe | grep 'str\ts1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-f32.exe | grep 'add	x0' | ${FAIL_IF_STDIN}

	# test ADRP/ADD/STR -> LDR when target is in __TEXT for 64-bit fp load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-addend-f64.o  -DLOAD_FPR_64 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddStr-addend-f64.o main.o -o AdrpAddStr-addend-f64.exe
	${OTOOL} -tV AdrpAddStr-addend-f64.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-f64.exe | grep 'str\td1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-f64.exe | grep 'add	x0' | ${FAIL_IF_STDIN}
	
	# test ADRP/ADD/STR -> LDR when target is in __TEXT for 128-bit vec load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-addend-v128.o  -DLOAD_VEC_128 -DADDEND=16
	${CC} ${CCFLAGS} AdrpAddStr-addend-v128.o main.o -o AdrpAddStr-addend-v128.exe 
	${OTOOL} -tV AdrpAddStr-addend-v128.exe | grep 'adr	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-v128.exe | grep 'str\tq1, \[x0\]' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-addend-v128.exe | grep 'add	x0' | ${FAIL_IF_STDIN}


AdrpAddStr-faraddend:	
	# test ADRP/ADD/STR -> ADRP/LD when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-faraddend-g8.o  -DLOAD_GPR_8 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddStr-faraddend-g8.o -o AdrpAddStr-faraddend-g8.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddStr-faraddend-g8.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-g8.dylib | grep 'add	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-g8.dylib | grep 'str\tb1, \[x0' | ${FAIL_IF_EMPTY}
	
	# test ADRP/ADD/STR -> ADRP/LD when target is in __TEXT for 16-bit load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-faraddend-g16.o  -DLOAD_GPR_16 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddStr-faraddend-g16.o -o AdrpAddStr-faraddend-g16.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddStr-faraddend-g16.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-g16.dylib | grep 'add	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-g16.dylib | grep 'str\th1, \[x0' | ${FAIL_IF_EMPTY}
	
	# test ADRP/ADD/STR -> LDR when target is in __TEXT for 32-bit load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-faraddend-g32.o  -DLOAD_GPR_32 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddStr-faraddend-g32.o -o AdrpAddStr-faraddend-g32.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddStr-faraddend-g32.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-g32.dylib | grep 'add	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-g32.dylib | grep 'str\tw1, \[x0' | ${FAIL_IF_EMPTY}

	# test ADRP/ADD/STR -> LDR when target is in __TEXT for 64-bit load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-faraddend-g64.o  -DLOAD_GPR_64 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddStr-faraddend-g64.o -o AdrpAddStr-faraddend-g64.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddStr-faraddend-g64.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-g64.dylib | grep 'add	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-g64.dylib | grep 'str\tx1, \[x0' | ${FAIL_IF_EMPTY}

	# test ADRP/ADD/STR -> LDR when target is in __TEXT for 32-bit fp load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-faraddend-f32.o  -DLOAD_FPR_32 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddStr-faraddend-f32.o -o AdrpAddStr-faraddend-f32.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddStr-faraddend-f32.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-f32.dylib | grep 'add	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-f32.dylib | grep 'str\ts1, \[x0' | ${FAIL_IF_EMPTY}

	# test ADRP/ADD/STR -> LDR when target is in __TEXT for 64-bit fp load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-faraddend-f64.o  -DLOAD_FPR_64 -DADDEND=8
	${CC} ${CCFLAGS} AdrpAddStr-faraddend-f64.o -o AdrpAddStr-faraddend-f64.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib
	${OTOOL} -tV AdrpAddStr-faraddend-f64.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-f64.dylib | grep 'add	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-f64.dylib | grep 'str\td1, \[x0' | ${FAIL_IF_EMPTY}
	
	# test ADRP/ADD/STR -> LDR when target is in __TEXT for 128-bit vec load
	${CC} ${CCFLAGS} AdrpAddStr.s -c -o AdrpAddStr-faraddend-v128.o  -DLOAD_VEC_128 -DADDEND=16
	${CC} ${CCFLAGS} AdrpAddStr-faraddend-v128.o -o AdrpAddStr-faraddend-v128.dylib -dynamiclib -install_name /usr/lib/libjunk.dylib 
	${OTOOL} -tV AdrpAddStr-faraddend-v128.dylib | grep 'adrp	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-v128.dylib | grep 'add	x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpAddStr-faraddend-v128.dylib | grep 'str\tq1, \[x0' | ${FAIL_IF_EMPTY}



AdrpLdrGotStr: AdrpLdrGotStr-extern AdrpLdrGotStr-externfargot AdrpLdrGotStr-near AdrpLdrGotStr-far AdrpLdrGotStr-nearunaligned AdrpLdrGotStr-farunaligned
	true

AdrpLdrGotStr-extern: main.o
	# test ADRP/LDR/LDR -> LDR/STR when GOT slot is close  for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-extern-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotStr-extern-g8.o main.o -o AdrpLdrGotStr-extern-g8.exe 
	${OTOOL} -tV AdrpLdrGotStr-extern-g8.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-extern-g8.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-extern-g8.exe | grep 'str\tb2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR/STR when GOT slot is close  for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-extern-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotStr-extern-g16.o main.o -o AdrpLdrGotStr-extern-g16.exe 
	${OTOOL} -tV AdrpLdrGotStr-extern-g16.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-extern-g16.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-extern-g16.exe | grep 'str\th2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR/STR when GOT slot is close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-extern-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotStr-extern-g32.o main.o -o AdrpLdrGotStr-extern-g32.exe 
	${OTOOL} -tV AdrpLdrGotStr-extern-g32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-extern-g32.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-extern-g32.exe | grep 'str\tw2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -> LDR/STR when GOT slot is close for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-extern-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotStr-extern-g64.o main.o -o AdrpLdrGotStr-extern-g64.exe 
	${OTOOL} -tV AdrpLdrGotStr-extern-g64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-extern-g64.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-extern-g64.exe | grep 'str\tx2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -> LDR/STR when GOT slot is close for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-extern-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotStr-extern-f32.o main.o -o AdrpLdrGotStr-extern-f32.exe 
	${OTOOL} -tV AdrpLdrGotStr-extern-f32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-extern-f32.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-extern-f32.exe | grep 'str\ts2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR -> LDR/STR when GOT slot is close for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-extern-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotStr-extern-f64.o main.o -o AdrpLdrGotStr-extern-f64.exe 
	${OTOOL} -tV AdrpLdrGotStr-extern-f64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-extern-f64.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-extern-f64.exe | grep 'str\td2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR/STR when GOT slot is close for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-extern-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DTARGET=_malloc
	${CC} ${CCFLAGS} AdrpLdrGotStr-extern-v128.o main.o -o AdrpLdrGotStr-extern-v128.exe 
	${OTOOL} -tV AdrpLdrGotStr-extern-v128.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-extern-v128.exe | grep 'ldr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-extern-v128.exe | grep 'str\tq2, \[x1\]' | ${FAIL_IF_EMPTY}


AdrpLdrGotStr-externfargot: main.o
	# test ADRP/LDR/STR left untouched when target is external and GOT slot far for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-externfargot-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-externfargot-g8.o main.o -o AdrpLdrGotStr-externfargot-g8.exe 
	${OTOOL} -tV AdrpLdrGotStr-externfargot-g8.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-g8.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-g8.exe | grep 'str\tb2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/STR left untouched when target is external and GOT slot far for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-externfargot-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-externfargot-g16.o main.o -o AdrpLdrGotStr-externfargot-g16.exe 
	${OTOOL} -tV AdrpLdrGotStr-externfargot-g16.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-g16.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-g16.exe | grep 'str\th2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/STR left untouched when target is external and GOT slot farfor 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-externfargot-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-externfargot-g32.o main.o -o AdrpLdrGotStr-externfargot-g32.exe 
	${OTOOL} -tV AdrpLdrGotStr-externfargot-g32.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-g32.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-g32.exe | grep 'str\tw2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/STR left untouched when target is external and GOT slot far for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-externfargot-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-externfargot-g64.o main.o -o AdrpLdrGotStr-externfargot-g64.exe 
	${OTOOL} -tV AdrpLdrGotStr-externfargot-g64.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-g64.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-g64.exe | grep 'str\tx2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/STR left untouched when target is external and GOT slot far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-externfargot-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-externfargot-f32.o main.o -o AdrpLdrGotStr-externfargot-f32.exe 
	${OTOOL} -tV AdrpLdrGotStr-externfargot-f32.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-f32.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-f32.exe | grep 'str\ts2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/STR left untouched when target is external and GOT slot far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-externfargot-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-externfargot-f64.o main.o -o AdrpLdrGotStr-externfargot-f64.exe 
	${OTOOL} -tV AdrpLdrGotStr-externfargot-f64.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-f64.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-f64.exe | grep 'str\td2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/STR -left untouched when target is external and GOT slot far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-externfargot-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DTARGET=_malloc -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-externfargot-v128.o main.o -o AdrpLdrGotStr-externfargot-v128.exe 
	${OTOOL} -tV AdrpLdrGotStr-externfargot-v128.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-v128.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-externfargot-v128.exe | grep 'str\tq2, \[x1\]' | ${FAIL_IF_EMPTY}


AdrpLdrGotStr-near: main.o 
	# test ADRP/LDR/STR -> ADR/STR when target close for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-near-g8.o -DLOAD_GPR_8 
	${CC} ${CCFLAGS} AdrpLdrGotStr-near-g8.o main.o -o AdrpLdrGotStr-near-g8.exe 
	${OTOOL} -tV AdrpLdrGotStr-near-g8.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-near-g8.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-near-g8.exe | grep 'str\tb2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/STR -> ADR/STR when target close for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-near-g16.o -DLOAD_GPR_16 
	${CC} ${CCFLAGS} AdrpLdrGotStr-near-g16.o main.o -o AdrpLdrGotStr-near-g16.exe 
	${OTOOL} -tV AdrpLdrGotStr-near-g16.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-near-g16.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-near-g16.exe | grep 'str\th2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/STR -> ADR/STR when target close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-near-g32.o -DLOAD_GPR_32 
	${CC} ${CCFLAGS} AdrpLdrGotStr-near-g32.o main.o -o AdrpLdrGotStr-near-g32.exe 
	${OTOOL} -tV AdrpLdrGotStr-near-g32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-near-g32.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-near-g32.exe | grep 'str\tw2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/STR -> ADR/STR when target close for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-near-g64.o -DLOAD_GPR_64 
	${CC} ${CCFLAGS} AdrpLdrGotStr-near-g64.o main.o -o AdrpLdrGotStr-near-g64.exe 
	${OTOOL} -tV AdrpLdrGotStr-near-g64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-near-g64.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-near-g64.exe | grep 'str\tx2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/STR -> ADR/STR when target close for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-near-f32.o -DLOAD_FPR_32 
	${CC} ${CCFLAGS} AdrpLdrGotStr-near-f32.o main.o -o AdrpLdrGotStr-near-f32.exe 
	${OTOOL} -tV AdrpLdrGotStr-near-f32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-near-f32.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-near-f32.exe | grep 'str\ts2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/STR -> ADR/STR when target close for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-near-f64.o -DLOAD_FPR_64 
	${CC} ${CCFLAGS} AdrpLdrGotStr-near-f64.o main.o -o AdrpLdrGotStr-near-f64.exe 
	${OTOOL} -tV AdrpLdrGotStr-near-f64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-near-f64.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-near-f64.exe | grep 'str\td2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/STR -> ADR/STR when target close for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-near-v128.o -DLOAD_VEC_128 
	${CC} ${CCFLAGS} AdrpLdrGotStr-near-v128.o main.o -o AdrpLdrGotStr-near-v128.exe 
	${OTOOL} -tV AdrpLdrGotStr-near-v128.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-near-v128.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-near-v128.exe | grep 'str\tq2, \[x1\]' | ${FAIL_IF_EMPTY}


AdrpLdrGotStr-nearunaligned: main.o 
	# test ADRP/LDR left untouched when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-nearunaligned-g8.o  -DLOAD_GPR_8 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotStr-nearunaligned-g8.o main.o -o AdrpLdrGotStr-nearunaligned-g8.exe 
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-g8.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-g8.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-g8.exe | grep 'str\tb2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-nearunaligned-g16.o  -DLOAD_GPR_16 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotStr-nearunaligned-g16.o main.o -o AdrpLdrGotStr-nearunaligned-g16.exe 
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-g16.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-g16.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-g16.exe | grep 'str\th2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR literal when target close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-nearunaligned-g32.o  -DLOAD_GPR_32 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotStr-nearunaligned-g32.o main.o -o AdrpLdrGotStr-nearunaligned-g32.exe 
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-g32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-g32.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-g32.exe | grep 'str\tw2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-nearunaligned-g64.o  -DLOAD_GPR_64 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotStr-nearunaligned-g64.o main.o -o AdrpLdrGotStr-nearunaligned-g64.exe 
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-g64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-g64.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-g64.exe | grep 'str\tx2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-nearunaligned-f32.o  -DLOAD_FPR_32 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotStr-nearunaligned-f32.o main.o -o AdrpLdrGotStr-nearunaligned-f32.exe 
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-f32.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-f32.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-f32.exe | grep 'str\ts2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-nearunaligned-f64.o  -DLOAD_FPR_64 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotStr-nearunaligned-f64.o main.o -o AdrpLdrGotStr-nearunaligned-f64.exe 
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-f64.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-f64.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-f64.exe | grep 'str\td2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -left untouched when target is external and GOT slot far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-nearunaligned-v128.o  -DLOAD_VEC_128 -DMISALIGN_DATA
	${CC} ${CCFLAGS} AdrpLdrGotStr-nearunaligned-v128.o main.o -o AdrpLdrGotStr-nearunaligned-v128.exe 
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-v128.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-v128.exe | grep 'adr\tx1,' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-nearunaligned-v128.exe | grep 'str\tq2, \[x1\]' | ${FAIL_IF_EMPTY}


AdrpLdrGotStr-far: main.o 
	# test ADRP/LDR left untouched when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-far-g8.o -DFOO_AS_CONST -DLOAD_GPR_8 -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-far-g8.o main.o -o AdrpLdrGotStr-far-g8.exe 
	${OTOOL} -tV AdrpLdrGotStr-far-g8.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-far-g8.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-far-g8.exe | grep 'str\tb2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-far-g16.o -DFOO_AS_CONST -DLOAD_GPR_16 -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-far-g16.o main.o -o AdrpLdrGotStr-far-g16.exe  
	${OTOOL} -tV AdrpLdrGotStr-far-g16.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-far-g16.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-far-g16.exe | grep 'str\th2, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR literal when target close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-far-g32.o -DFOO_AS_CONST -DLOAD_GPR_32 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotStr-far-g32.o main.o -o AdrpLdrGotStr-far-g32.exe  
	${OTOOL} -tV AdrpLdrGotStr-far-g32.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-far-g32.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-far-g32.exe | grep 'str\tw2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-far-g64.o -DFOO_AS_CONST -DLOAD_GPR_64 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotStr-far-g64.o main.o -o AdrpLdrGotStr-far-g64.exe  
	${OTOOL} -tV AdrpLdrGotStr-far-g64.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-far-g64.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-far-g64.exe | grep 'str\tx2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-far-f32.o -DFOO_AS_CONST -DLOAD_FPR_32 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotStr-far-f32.o main.o -o AdrpLdrGotStr-far-f32.exe  
	${OTOOL} -tV AdrpLdrGotStr-far-f32.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-far-f32.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-far-f32.exe | grep 'str\ts2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-far-f64.o -DFOO_AS_CONST -DLOAD_FPR_64 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotStr-far-f64.o main.o -o AdrpLdrGotStr-far-f64.exe  
	${OTOOL} -tV AdrpLdrGotStr-far-f64.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-far-f64.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-far-f64.exe | grep 'str\td2, \[x0,' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -left untouched when target is external and GOT slot far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-far-v128.o -DFOO_AS_CONST -DLOAD_VEC_128 -DPADDING 
	${CC} ${CCFLAGS} AdrpLdrGotStr-far-v128.o main.o -o AdrpLdrGotStr-far-v128.exe  
	${OTOOL} -tV AdrpLdrGotStr-far-v128.exe | grep 'adrp\tx0' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-far-v128.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-far-v128.exe | grep 'str\tq2, \[x0,' | ${FAIL_IF_EMPTY}


AdrpLdrGotStr-farunaligned: main.o 
	# test ADRP/LDR left untouched when target is in __TEXT for 8-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-farunaligned-g8.o  -DLOAD_GPR_8 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-farunaligned-g8.o main.o -o AdrpLdrGotStr-farunaligned-g8.exe 
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-g8.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-g8.exe | grep 'add\t' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-g8.exe | grep 'str\tb2, \[x0,' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 16-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-farunaligned-g16.o  -DLOAD_GPR_16 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-farunaligned-g16.o main.o -o AdrpLdrGotStr-farunaligned-g16.exe 
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-g16.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-g16.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-g16.exe | grep 'str\th2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -> LDR literal when target close for 32-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-farunaligned-g32.o  -DLOAD_GPR_32 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-farunaligned-g32.o main.o -o AdrpLdrGotStr-farunaligned-g32.exe 
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-g32.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-g32.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-g32.exe | grep 'str\tw2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-farunaligned-g64.o  -DLOAD_GPR_64 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-farunaligned-g64.o main.o -o AdrpLdrGotStr-farunaligned-g64.exe 
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-g64.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-g64.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-g64.exe | grep 'str\tx2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 32-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-farunaligned-f32.o  -DLOAD_FPR_32 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-farunaligned-f32.o main.o -o AdrpLdrGotStr-farunaligned-f32.exe 
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-f32.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-f32.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-f32.exe | grep 'str\ts2, \[x1\]' | ${FAIL_IF_EMPTY}

	# test ADRP/LDR/LDR left untouched when target is external and GOT slot far for 64-bit fp load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-farunaligned-f64.o  -DLOAD_FPR_64 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-farunaligned-f64.o main.o -o AdrpLdrGotStr-farunaligned-f64.exe 
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-f64.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-f64.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-f64.exe | grep 'str\td2, \[x1\]' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR/LDR -left untouched when target is external and GOT slot far for 128-bit vec load
	${CC} ${CCFLAGS} AdrpLdrGotStr.s -c -o AdrpLdrGotStr-farunaligned-v128.o  -DLOAD_VEC_128 -DMISALIGN_DATA -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGotStr-farunaligned-v128.o main.o -o AdrpLdrGotStr-farunaligned-v128.exe 
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-v128.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-v128.exe | grep 'add\t' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGotStr-farunaligned-v128.exe | grep 'str\tq2, \[x1\]' | ${FAIL_IF_EMPTY}


AdrpLdrGot:  main.o
	# test ADRP/LDR left untouched when target is extern and GOT is far
	${CC} ${CCFLAGS} AdrpLdrGot.s -c -o AdrpLdrGot-externfar.o -DPADDING
	${CC} ${CCFLAGS} AdrpLdrGot-externfar.o main.o -o AdrpLdrGot-externfar.exe
	${OTOOL} -tV AdrpLdrGot-externfar.exe | grep 'adrp' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGot-externfar.exe | grep 'ldr\tx1, \[x0' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR -> NOP/LDR literal when target is exern and GOT is near
	${CC} ${CCFLAGS} AdrpLdrGot.s -c -o AdrpLdrGot-externnear.o
	${CC} ${CCFLAGS} AdrpLdrGot-externnear.o main.o -o AdrpLdrGot-externnear.exe
	${OTOOL} -tV AdrpLdrGot-externnear.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGot-externnear.exe | grep 'ldr\tx1' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR -> ADR when target is local, near code 
	${CC} ${CCFLAGS} AdrpLdrGot.s -c -o AdrpLdrGot-localnear.o -DTARGET=_fooCode
	${CC} ${CCFLAGS} AdrpLdrGot-localnear.o main.o -o AdrpLdrGot-localnear.exe
	${OTOOL} -tV AdrpLdrGot-localnear.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGot-localnear.exe | grep 'adr\tx1' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR -> ADR when target is local, near data 
	${CC} ${CCFLAGS} AdrpLdrGot.s -c -o AdrpLdrGot-localnear2.o -DTARGET=_fooData
	${CC} ${CCFLAGS} AdrpLdrGot-localnear2.o main.o -o AdrpLdrGot-localnear2.exe
	${OTOOL} -tV AdrpLdrGot-localnear2.exe | grep 'adrp' | ${FAIL_IF_STDIN}
	${OTOOL} -tV AdrpLdrGot-localnear2.exe | grep 'adr\tx1' | ${FAIL_IF_EMPTY}
	
	# test ADRP/LDR -> ADR when target is local, far code 
	${CC} ${CCFLAGS} AdrpLdrGot.s -c -o AdrpLdrGot-localfar.o -DTARGET=_fooCode -DPADDING=1
	${CC} ${CCFLAGS} AdrpLdrGot-localfar.o main.o -o AdrpLdrGot-localfar.exe
	${OTOOL} -tV AdrpLdrGot-localfar.exe | grep 'adrp\tx1' | ${FAIL_IF_EMPTY}
	${OTOOL} -tV AdrpLdrGot-localfar.exe | grep 'add\tx1' | ${FAIL_IF_EMPTY}
	


clean:
	rm -f *.o   *.exe   *.dylib
