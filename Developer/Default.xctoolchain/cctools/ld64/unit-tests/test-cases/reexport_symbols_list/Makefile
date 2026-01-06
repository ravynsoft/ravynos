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

SHELL = bash # use bash shell so we can redirect just stderr

#
# Test -reexport_symbols_list 
#

run: all

all:
	# build base library
	${CC} ${CCFLAGS} -dynamiclib bar.c -o `pwd`/libbar.dylib
	${FAIL_IF_BAD_MACHO} libbar.dylib

	# verify re-export fails with -bundle
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} -bundle foo.c -o libfoo.bundle libbar.dylib -Wl,-reexported_symbols_list,bart.exp 2>/dev/null
	
	# verify failure if re-exported symbol does not exist
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib libbar.dylib -Wl,-reexported_symbols_list,junk.exp 2>/dev/null
	
	# verify failure if re-exported symbol is from .o file
	${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib libbar.dylib -Wl,-reexported_symbols_list,foo.exp 2>/dev/null
	
	# build library the re-exports _bart from base library
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib libbar.dylib -Wl,-reexported_symbols_list,bart.exp
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	${DYLDINFO} -export libfoo.dylib | grep _barb | ${FAIL_IF_STDIN}
	${DYLDINFO} -export libfoo.dylib | grep _bart | grep 're-export' | ${FAIL_IF_EMPTY}
	
	# link against dylib and verify _bart is marked as coming from libfoo
	${CC} ${CCFLAGS} main1.c libfoo.dylib -o main1
	${DYLDINFO} -bind -lazy_bind main1 | grep _bart | grep libfoo | ${FAIL_IF_EMPTY}

	${PASS_IFF_GOOD_MACHO} libfoo.dylib
	
clean:
	rm -rf libbar.dylib libfoo.dylib main1 
