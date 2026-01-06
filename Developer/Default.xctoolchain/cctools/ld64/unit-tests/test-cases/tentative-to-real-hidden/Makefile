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
# Verify that -r -d -exported_symbol_list uses proper relocations for hidden 
# newly defined (no longer tentative) definitions.
#

ifneq (${ARCH},x86_64)
	BETTER_NOT_FIND = _tent
else
	# x86_64 uses a different style of relocations, so external relocs are ok to have
	BETTER_NOT_FIND = blahblah
endif



run: all

all:
	${CC} ${CCFLAGS} test.c -c -o test.o
	${FAIL_IF_BAD_OBJ} test.o

	${LD} -arch ${ARCH} -d -r test.o -exported_symbol _tent1 -o test-r.o
	otool -rv test-r.o | grep ${BETTER_NOT_FIND} | ${PASS_IFF_EMPTY}
	

clean:
	rm -rf *.o
