##
# Copyright (c) 2006-2010 Apple Inc. All rights reserved.
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
# The point of this test is to see that a dylib 
# run through the rebase tool is the same as if
# the dylib was originally built at that address
#

run: all

all:
	${CC} ${CCFLAGS} -c -g foo.c -o foo.${ARCH}.o
	${FAIL_IF_BAD_OBJ} foo.${ARCH}.o

	${CC} ${CCFLAGS} -c -g bar.m -o bar.${ARCH}.o
	${FAIL_IF_BAD_OBJ} bar.${ARCH}.o

	${CC} ${CCFLAGS} foo.${ARCH}.o bar.${ARCH}.o -dynamiclib -Wl,-no_uuid -Wl,-no_order_data -o libfoo.${ARCH}.dylib -framework Foundation  -framework CoreFoundation 
	${FAIL_IF_BAD_MACHO} libfoo.${ARCH}.dylib

	${CC} ${CCFLAGS} foo.${ARCH}.o bar.${ARCH}.o -dynamiclib -Wl,-no_uuid -Wl,-no_order_data -o libfoo-alt.${ARCH}.dylib -framework Foundation -framework CoreFoundation  -seg1addr 0x12340000 -install_name libfoo.${ARCH}.dylib
	${FAIL_IF_BAD_MACHO} libfoo-alt.${ARCH}.dylib
	
	${REBASE} -arch ${ARCH} -low_address 0x12340000 libfoo.${ARCH}.dylib
	${FAIL_IF_BAD_MACHO} libfoo.${ARCH}.dylib

	${PASS_IFF} diff libfoo.${ARCH}.dylib libfoo-alt.${ARCH}.dylib

clean:
	rm -f *.o *.dylib
