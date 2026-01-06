##
# Copyright (c) 2007 Apple Inc. All rights reserved.
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
# Test the if all symbols from a dylib are weak_import, that the whole dylib is weakly loaded
#


run: all

all:	
	${CC} ${CCFLAGS} -dynamiclib  foo.c -o libfoo.dylib 
	${FAIL_IF_BAD_MACHO} libfoo.dylib

	${CC} ${CCFLAGS} -dynamiclib  bar.c -o libbar.dylib 
	${FAIL_IF_BAD_MACHO} libbar.dylib

	${CC} ${CCFLAGS} ${VERSION_OLD_LINKEDIT} main.c -o main libfoo.dylib libbar.dylib
	# libfoo.dylib should be weakly loaded because all symbols are weakly imported
	otool -lv main | grep -B2 libfoo.dylib | grep LC_LOAD_WEAK_DYLIB | ${FAIL_IF_EMPTY}
	# libbar.dylib should not be weakly loaded because _bar1 is not weakly imported
	otool -lv main | grep -B2 libbar.dylib | grep LC_LOAD_DYLIB | ${FAIL_IF_EMPTY}
	${FAIL_IF_BAD_MACHO} main
	
	${CC} ${CCFLAGS} ${VERSION_NEW_LINKEDIT} main.c -o main2 libfoo.dylib libbar.dylib
	# libfoo.dylib should be weakly loaded because all symbols are weakly imported
	otool -lv main2 | grep -B2 libfoo.dylib | grep LC_LOAD_WEAK_DYLIB | ${FAIL_IF_EMPTY}
	# libbar.dylib should not be weakly loaded because _bar1 is not weakly imported
	otool -lv main2 | grep -B2 libbar.dylib | grep LC_LOAD_DYLIB | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main2

	${CC} ${CCFLAGS} ${VERSION_NEW_LINKEDIT} data.c -o data libfoo.dylib libbar.dylib
	# libfoo.dylib should be weakly loaded because all symbols are weakly imported
	otool -lv data | grep -B2 libfoo.dylib | grep LC_LOAD_WEAK_DYLIB | ${FAIL_IF_EMPTY}
	# libbar.dylib should not be weakly loaded because _bar1 is not weakly imported
	otool -lv data | grep -B2 libbar.dylib | grep LC_LOAD_DYLIB | ${FAIL_IF_EMPTY}
	${FAIL_IF_BAD_MACHO} data

	${CC} ${CCFLAGS} ${VERSION_NEW_LINKEDIT} data.c -o data2 libfoo.dylib libbar.dylib
	# libfoo.dylib should be weakly loaded because all symbols are weakly imported
	otool -lv data2 | grep -B2 libfoo.dylib | grep LC_LOAD_WEAK_DYLIB | ${FAIL_IF_EMPTY}
	# libbar.dylib should not be weakly loaded because _bar1 is not weakly imported
	otool -lv data2 | grep -B2 libbar.dylib | grep LC_LOAD_DYLIB | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} data2
		
clean:
	rm -rf libfoo.dylib libbar.dylib main main2 data data2
