##
# Copyright (c) 2006-2008 Apple Inc. All rights reserved.
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


LLVMAR  = /usr/local/bin/llvm-ar

#
# Test the we set the stack execution bit properly.


all: zero one two three four five six seven eight nine ten \
    eleven twelve thirteen fourteen fifteen sixteen seventeen \
	eighteen nineteen 

zero:
	#
	# llvm  : a.c : Dfoo3
	# llvm  : b.c : Dfoo2
	# MachO : main.c : Ufoo2, Ufoo3
	#
	#echo "Zero..."
	${CC} ${CCFLAGS} -flto a.c -c -o a.o 
	${CC} ${CCFLAGS} -flto b.c -c -o b.o
	${CC} ${CCFLAGS} main.c -c -o main.o
	${CC} ${CCFLAGS} a.o b.o main.o -o main.exe
	${PASS_IFF_GOOD_MACHO} main.exe
	
one:
	#
	# llvm  : a1.c : Dfoo3, Ufoo4
	# llvm  : b1.c : Dfoo2, Ufoo4
	# MachO : main1.c : Dfoo4, Ufoo2, Ufoo3
	#
	#echo "One..."
	${CC} ${CCFLAGS} -flto a1.c -c -o a1.o
	${CC} ${CCFLAGS} -flto b1.c -c -o b1.o
	${CC} ${CCFLAGS} main1.c -c -o main1.o
	${CC} ${CCFLAGS} a1.o b1.o main1.o -o main1.exe
	${PASS_IFF_GOOD_MACHO} main1.exe

two:
	#
	# llvm  : a2.c : Dfoo3, Ufoo4
	# llvm  : b2.c : Dfoo2, Dfoo4
	# MachO : main2.c : Ufoo2, Ufoo3
	#
	#echo "Two..."
	${CC} ${CCFLAGS} -flto a2.c -c -o a2.o
	${CC} ${CCFLAGS} -flto b2.c -c -o b2.o
	${CC} ${CCFLAGS} main2.c -c -o main2.o
	${CC} ${CCFLAGS} a2.o b2.o main2.o -o main2.exe
	${PASS_IFF_GOOD_MACHO} main2.exe

three:
	#
	# llvm  : a3.c : Dfoo1, Dbar
	# llvm  : b3.c : Dfoo2, Ubar
	# MachO : main3.c : Ufoo1, Ufoo2, Ubar
	#
	#echo "Three..."
	${CC} ${CCFLAGS} -flto a3.c -c -o a3.o
	${CC} ${CCFLAGS} -flto b3.c -c -o b3.o
	${CC} ${CCFLAGS} main3.c -c -o main3.o
	${CC} ${CCFLAGS} a3.o b3.o main3.o -o main3.exe
	${PASS_IFF_GOOD_MACHO} main3.exe

four:
	#
	# llvm  : a4.c : Dfoo3, Ufoo4
	# llvm  : b4.c : Dfoo2, DLmyfoo, Ufoo4
	# MachO : main4.c : Dfoo4, Ufoo2, Ufoo3
	#
	#echo "Four..."
	${CC} ${CCFLAGS} -flto a4.c -c -o a4.o
	${CC} ${CCFLAGS} -flto b4.c -c -o b4.o
	${CC} ${CCFLAGS} main4.c -c -o main4.o
	${CC} ${CCFLAGS} a4.o b4.o main4.o -o main4.exe
	${PASS_IFF_GOOD_MACHO} main4.exe

five:
	#
	# llvm  : a5.c : Dfoo1, Ufoo2, Ufoo3
	# llvm  : b5.c : Dfoo2
	# MachO : main5.c : Dfoo3, Ufoo1
	#
	#echo "verify that if call to mach-o is optimized away, mach-o func is dead stripped"
	${CC} ${CCFLAGS} -flto a5.c -c -o a5.o
	${CC} ${CCFLAGS} -flto b5.c -c -o b5.o
	${CC} ${CCFLAGS} main5.c -c -o main5.o
	${CC} ${CCFLAGS} a5.o b5.o main5.o -o main5.exe  -Wl,-dead_strip
	${OTOOL} -tV main5.exe | grep foo3 | ${PASS_IFF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main5.exe

six:
	#
	# llvm  : a6.c : Dfoo1, Dfoo2
	# MachO : main6.c : Ufoo1
	#
	#echo "verify dead stripping of foo2 in main executable"
	${CC} ${CCFLAGS} -flto a6.c -c -o a6.o
	${CC} ${CCFLAGS} main6.c -c -o main6.o
	${CC} ${CCFLAGS} a6.o  main6.o -o main6.exe  -Wl,-dead_strip
	${PASS_IFF_GOOD_MACHO} main6.exe
	${OTOOL} -tV main6.exe | grep foo2 | ${PASS_IFF_EMPTY}

seven:
	#
	# llvm  : a7.c : Dfoo1, Dfoo2, Ufoo3
	# llvm  : b7.c : Dfoo3, ufoo2
	# MachO : main7.c : Ufoo1
	#
	#echo "Seven..."
	${CC} ${CCFLAGS} -flto a7.c -c -o a7.o
	${CC} ${CCFLAGS} -flto b7.c -c -o b7.o
	${CC} ${CCFLAGS} main7.c -c -o main7.o
	${CC} ${CCFLAGS} a7.o b7.o  main7.o -o main7.exe
	${PASS_IFF_GOOD_MACHO} main7.exe

eight:
	#
	# llvm  : a8.c : Dfoo1, Dfoo2
	# MachO : main8.c : Ufoo1
	#
	#echo "Eight..."
	${CC} ${CCFLAGS} -flto a8.c -c -o a8.o
	${CC} ${CCFLAGS} main8.c -c -o main8.o
	${CC} ${CCFLAGS} a8.o main8.o -o main8.exe   -Wl,-dead_strip
	${OTOOL} -tV main8.exe | grep foo2 | ${PASS_IFF_EMPTY}
	${OTOOL} -tV main8.exe | grep unnamed_2_1 | ${PASS_IFF_EMPTY}

nine:
	#
	# llvm  : a9.c : Dfoo1, Dfoo2, Dfoo3, Ufoo3, Ufoo4
	# MachO : main9.c : Ufoo1, Dfoo4
	#
	#echo "Nine..."
	${CC} ${CCFLAGS} -flto a9.c -c -o a9.o
	${CC} ${CCFLAGS} main9.c -c -o main9.o
	${CC} ${CCFLAGS} a9.o main9.o -o main9.exe   -Wl,-dead_strip
	${OTOOL} -tV main9.exe | grep foo2 | ${PASS_IFF_EMPTY}
	${OTOOL} -tV main9.exe | grep foo4 | ${PASS_IFF_EMPTY}
	${OTOOL} -tV main9.exe | grep unnamed_2_1 | ${PASS_IFF_EMPTY}

ten:
	#
	# llvm  : a10.c
	# llvm  : b10.c
	# MachO : main10.c
	#
	#echo "Ten..."
	${CC} ${CCFLAGS} -flto a10.c -c -o a10.o
	${CC} ${CCFLAGS} -flto b10.c -c -o b10.o
	${CC} ${CCFLAGS} main10.c -c -o main10.o
	${CC} ${CCFLAGS} a10.o b10.o main10.o -o main10.exe
	${PASS_IFF_GOOD_MACHO} main10.exe

eleven:
	#
	# llvm  : a11.c
	# MachO : main11.c
	#
	#echo "Eleven..."
	${CC} ${CCFLAGS} -flto a11.c -c -o a11.o
	${CC} ${CCFLAGS} main11.c -c -o main11.o
	${CC} ${CCFLAGS} a11.o main11.o -o main11.exe
	${PASS_IFF_GOOD_MACHO} main11.exe

twelve:
	#
	# llvm  : a12.c
	# MachO : main12.c
	#
	#echo "verify tentative def in llvm .o referenced from mach-o"
	${CC} ${CCFLAGS} -flto a12.c -c -o a12.o
	${CC} ${CCFLAGS} main12.c -c -o main12.o
	${CC} ${CCFLAGS} a12.o main12.o -o main12.exe
	${PASS_IFF_GOOD_MACHO} main12.exe

thirteen:
	#
	# llvm : a13.cc
	# MachO : main13.cc
	#
	# echo "Thirteen..."
	${CC} ${CXXFLAGS} -flto a13.cc -c -o a13.o
	${CC} ${CXXFLAGS} main13.cc -c -o main13.o
	${CXX} a13.o main13.o -o main13.exe

fourteen:
	#
	# llvm : a14.c b14.c
	#
	# echo "verify an used hidden symbol is removed from a dylib"
	${CC} ${CXXFLAGS} -O4 -dynamiclib a14.c b14.c -o ab14.dylib
	${FAIL_IF_BAD_MACHO} ab14.dylib
	nm -m ab14.dylib | grep _X | ${PASS_IFF_EMPTY}

fifteen:
	# echo "verify -dead_strip works with hidden symbols"
	${CC} ${CXXFLAGS} -O4 -Wl,-dead_strip a15.c c15.c -o main15.exe
	${CC} ${CXXFLAGS} -O4 a15.c c15.c -o main15.exe
	${FAIL_IF_BAD_MACHO} main15.exe
	${CC} ${CXXFLAGS} -O4 -Wl,-dead_strip -dynamiclib a15.c b15.c -o a15.dylib
	${CC} ${CXXFLAGS} -O4 a15.c b15.c -dynamiclib -o a15.dylib 
	${FAIL_IF_BAD_MACHO} a15.dylib
	
sixteen:
	# echo "verify -save-temps"
	${CC} ${CCFLAGS} -flto main16.c -c -o main16.o
	${CC} ${CCFLAGS} main16.o -o main16.exe -Wl,-save-temps
	${PASS_IFF} test -e main16.exe.lto.bc
	${PASS_IFF} test -e main16.exe.lto.o
    
seventeen:
	# echo "verify ld -r of all bitcode files produces a bitcode file"
	${CC} ${CCFLAGS} -flto a17.c -c -o a17.o
	${CC} ${CCFLAGS} -flto b17.c -c -o b17.o
	${LD} -arch ${ARCH} -r a17.o b17.o -o ab17.o
	file ab17.o | grep "Mach-O" | ${PASS_IFF_EMPTY}
	# echo "verify ld -r of bitcode and mach-o produces mach-o"
	${CC} ${CCFLAGS} b17.c -c -o b17m.o
	${LD} -arch ${ARCH} -r a17.o b17m.o -o ab17m.o
	file ab17m.o | grep "Mach-O" | ${PASS_IFF_STDIN}

eighteen:
	#echo verify ld -r -keep_private_externs works
	${CC} ${CCFLAGS} -flto a18.c -c -o a18.o
	${LD} -arch ${ARCH} -r -keep_private_externs a18.o -o a18-rkpe.o
	nm -nm a18-rkpe.o | grep _common_hidden1 | grep "private external" | ${FAIL_IF_EMPTY}
	nm -nm a18-rkpe.o | grep _func_hidden2 | grep "private external" | ${FAIL_IF_EMPTY}
	#echo verify ld -r makes hidden symbols internal (except for commons)
	${LD} -arch ${ARCH} -r a18.o -o a18-r.o
	#ObjectDump -nm a18-r.o | grep _common_hidden1 | grep " hidden" | ${FAIL_IF_EMPTY}
	#ObjectDump -nm a18-r.o | grep _func_hidden2 | grep " internal" | ${FAIL_IF_EMPTY}
   
nineteen:
	#echo verify missing symbol error
	${CC} ${CCFLAGS} -flto main19.c -c -o main19.o
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} main19.o -o main19.exe 2>fail.log
	grep _foo fail.log | ${PASS_IFF_STDIN}

twenty:
	#echo verify bitcode files in archives works 
	#${CC} ${CCFLAGS} -flto a20.c -c -o a20.o
	#${CC} ${CCFLAGS} -flto b20.c -c -o b20.o
	#libtool -static a20.o b20.o -o lib20.a 
	#${CC} ${CCFLAGS} main20.c lib20.a -all_load -o main20.exe
	#nm main20.exe | grep _foo | ${PASS_IFF_STDIN}
	


	
	
clean:
	rm -rf *.o main*.exe big.* *.dylib main16.*.bc fail.log lib20.a main21.preload lib21.a 
	
