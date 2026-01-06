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
# This test case checks -non_global_symbols_no_strip_list and -non_global_symbols_strip_list
# with and without wildcards
#


run: all

all:
	${CC} ${CCFLAGS} main.c foo.c -o main
	${FAIL_IF_BAD_MACHO} main
	nm -j main > main.nm
	# build stripping a.list
	${CC} ${CCFLAGS} main.c foo.c -Wl,-non_global_symbols_strip_list,a.list -o main-a
	${FAIL_IF_BAD_MACHO} main-a
	nm -j main-a > main-a.nm
	diff main.nm main-a.nm | egrep '<|>' > a.diff
	diff a.diff a.expect | ${FAIL_IF_STDIN}
	# build but strip at .o file level a.list
	${CC} ${CCFLAGS} main.c -c -o main.o
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	${LD} -r -arch ${ARCH} main.o foo.o -o all-a.o -non_global_symbols_strip_list a.list
	${CC} ${CCFLAGS} all-a.o -Wl,-non_global_symbols_strip_list,a.list -o main-a
	${FAIL_IF_BAD_MACHO} main-a
	nm -j main-a > main-a.nm
	diff main.nm main-a.nm | egrep '<|>' > a.diff
	diff a.diff a.expect | ${FAIL_IF_STDIN}
	# build stripping b.list
	${CC} ${CCFLAGS} main.c foo.c -Wl,-non_global_symbols_strip_list,b.list -o main-b
	${FAIL_IF_BAD_MACHO} main-b
	nm -j main-b > main-b.nm
	diff main.nm main-b.nm | egrep '<|>' > b.diff
	diff b.diff b.expect | ${FAIL_IF_STDIN}
	# build but strip at .o file level b.list
	${CC} ${CCFLAGS} main.c -c -o main.o
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	${LD} -r -arch ${ARCH} main.o foo.o -o all-b.o -non_global_symbols_strip_list b.list
	${CC} ${CCFLAGS} all-b.o -Wl,-non_global_symbols_strip_list,b.list -o main-b
	${FAIL_IF_BAD_MACHO} main-b
	nm -j main-b > main-b.nm
	diff main.nm main-b.nm | egrep '<|>' > b.diff
	diff b.diff b.expect | ${FAIL_IF_STDIN}
	# build stripping c.list
	${CC} ${CCFLAGS} main.c foo.c -Wl,-non_global_symbols_no_strip_list,c.list -o main-c
	nm -m main-c | grep non-external | grep -v my | ${FAIL_IF_STDIN}
	${PASS_IFF_GOOD_MACHO} main-c
	
	
clean:
	rm -rf main main.nm main-a main-a.nm a.diff main-b main-b.nm b.diff main-c all-a.o all-b.o foo.o main.o
