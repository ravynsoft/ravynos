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
# main2.o has hint to link with libfoo
# libfoo.a(libfoo2.o) has link to link with libbar.dylib
#

run: all

all:
	${CC} ${CCFLAGS} subbar.c -dynamiclib -o libsubbar.dylib
	${CC} ${CCFLAGS} bar.c -dynamiclib -o libbar.dylib -Wl,-reexport_library,libsubbar.dylib
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	${LD} -r foo.o -add_linker_option -lbar -o foo2.o
	libtool -static foo2.o -o libfoo.a
	${CC} ${CCFLAGS} main.c -c -o main.o
	${LD} -r main.o -add_linker_option -lfoo -o main2.o
	${CC} ${CCFLAGS} main2.o -o main -L.
	${DYLDINFO} -lazy_bind main | grep _bar | grep libbar | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -f libsubbar.dylib libbar.dylib foo.o foo2.o libfoo.a main.o main2.o main
	
