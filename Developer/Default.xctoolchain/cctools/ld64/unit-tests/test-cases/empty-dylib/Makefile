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
# Check that the n_sect number for __mh_dylib_header is valid when there is no __text section
# Also check that codesigning works on empty dylib.
#

CODESIGN_ARCH = ${ARCH}
ifeq (${ARCH},ppc)
	CODESIGN_ARCH = ppc7400
endif


run: all

all:
	${CC} ${CCFLAGS} -dynamiclib justdata.c -o libjustdata.dylib -Wl,-no_compact_unwind
	${PASS_IFF_GOOD_MACHO} libjustdata.dylib
	${CC} ${CCFLAGS} -dynamiclib empty.c -o libempty.dylib -Wl,-no_compact_unwind
	codesign_allocate -i libempty.dylib -a ${CODESIGN_ARCH} 1024 -o libsigned.dylib
	${PASS_IFF_GOOD_MACHO} libsigned.dylib

clean:
	rm -rf libempty.dylib libjustdata.dylib libsigned.dylib
