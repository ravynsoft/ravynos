##
# Copyright (c) 2010 Apple Inc. All rights reserved.
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
# Test that fine grain re-exports works
#

run: all

all:
	# build base library
	${CC} ${CCFLAGS} -dynamiclib bar.c -o `pwd`/libbar.dylib
	${FAIL_IF_BAD_MACHO} libbar.dylib

	# build library that re-exports _bar from base library
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib libbar.dylib -exported_symbols_list foo.exp
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	${DYLDINFO} -export libfoo.dylib | grep _bar | grep 're-export' | ${FAIL_IF_EMPTY}
	${DYLDINFO} -export libfoo.dylib | grep _bar_weak | grep 're-export' | grep 'weak_def' |${FAIL_IF_EMPTY}
	# link against dylib and verify _bar is marked as coming from libfoo
	${CC} ${CCFLAGS} main1.c libfoo.dylib -o main1
	${DYLDINFO}  -bind -lazy_bind main1 | grep _bar | grep libfoo | ${FAIL_IF_EMPTY}
	${DYLDINFO}  -bind -lazy_bind main1 | grep _bar_weak | grep libfoo | ${FAIL_IF_EMPTY}

	# build library that re-exports _bar from base library as _mybar
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo2.dylib libbar.dylib -Wl,-alias,_bar,_mybar -exported_symbols_list foo2.exp -DUSE_MY
	${FAIL_IF_BAD_MACHO} libfoo2.dylib
	${DYLDINFO} -export libfoo2.dylib | grep _mybar | grep 're-export' | grep _bar | ${FAIL_IF_EMPTY} 
	${DYLDINFO} -lazy_bind libfoo2.dylib | grep _bar | grep libbar | ${FAIL_IF_EMPTY} 
	${DYLDINFO} -bind libfoo2.dylib | grep _bar | grep libbar | ${FAIL_IF_EMPTY} 
	# link against dylib and verify _mybar is marked as coming from libfoo
	${CC} ${CCFLAGS} main2.c libfoo2.dylib -o main2
	${DYLDINFO}  -bind -lazy_bind main2 | grep _mybar | grep libfoo2 | ${FAIL_IF_EMPTY}
	
	${PASS_IFF_GOOD_MACHO} libfoo2.dylib 
	
clean:
	rm -rf libbar.dylib libfoo.dylib libfoo2.dylib main1 main2
