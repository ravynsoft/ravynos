##
# Copyright (c) 2007-2008 Apple Inc. All rights reserved.
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
# Tests the use of wildcards in exported symbol lists
#

run: all

all:
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-exported_symbol -Wl,'_foo*bar'
	nm -j -g libfoo.dylib -s __TEXT __text | diff - expect1 | ${FAIL_IF_STDIN}
	${FAIL_IF_BAD_MACHO} libfoo.dylib

	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-exported_symbol -Wl,'_f?o'
	nm -j -g libfoo.dylib -s __TEXT __text | diff - expect2 | ${FAIL_IF_STDIN}
	${FAIL_IF_BAD_MACHO} libfoo.dylib

	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-exported_symbol -Wl,'_foo*'
	nm -j -g libfoo.dylib -s __TEXT __text | diff - expect3 | ${FAIL_IF_STDIN}
	${FAIL_IF_BAD_MACHO} libfoo.dylib

	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-exported_symbol -Wl,'_f*o*'
	nm -j -g libfoo.dylib -s __TEXT __text | diff - expect4 | ${FAIL_IF_STDIN}
	${FAIL_IF_BAD_MACHO} libfoo.dylib

	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-exported_symbol -Wl,_foo -Wl,-exported_symbol -Wl,'_*bar'
	nm -j -g libfoo.dylib -s __TEXT __text | diff - expect5 | ${FAIL_IF_STDIN}
	${FAIL_IF_BAD_MACHO} libfoo.dylib

	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -exported_symbols_list list5
	nm -j -g libfoo.dylib -s __TEXT __text | diff - expect5 | ${FAIL_IF_STDIN}
	${FAIL_IF_BAD_MACHO} libfoo.dylib

	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-unexported_symbol -Wl,'_*2*'
	nm -j -g libfoo.dylib -s __TEXT __text | diff - expect6 | ${FAIL_IF_STDIN}
	${FAIL_IF_BAD_MACHO} libfoo.dylib

	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-exported_symbol -Wl,'_f[abcdef]o'
	nm -j -g libfoo.dylib -s __TEXT __text | diff - expect7 | ${FAIL_IF_STDIN}
	${FAIL_IF_BAD_MACHO} libfoo.dylib

	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-exported_symbol -Wl,'_f[a-f]o'
	nm -j -g libfoo.dylib -s __TEXT __text | diff - expect7 | ${FAIL_IF_STDIN}
	${FAIL_IF_BAD_MACHO} libfoo.dylib

	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-exported_symbol -Wl,'_f[a-z]o'
	nm -j -g libfoo.dylib -s __TEXT __text | diff - expect8 | ${FAIL_IF_STDIN}
	${FAIL_IF_BAD_MACHO} libfoo.dylib

	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-exported_symbol -Wl,'_f[a-fnop]o'
	nm -j -g libfoo.dylib -s __TEXT __text | diff - expect8 | ${FAIL_IF_STDIN}
	${PASS_IFF_GOOD_MACHO} libfoo.dylib
	
clean:
	rm libfoo.dylib
