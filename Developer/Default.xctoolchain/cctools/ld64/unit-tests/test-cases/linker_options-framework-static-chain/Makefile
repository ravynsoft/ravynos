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
# Check linker options work for -framework containing static archives
# in a recursive manner, chaining linkage from one archive to another
#

run: all

all:
	mkdir -p Baz.framework
	${CC} ${CCFLAGS} baz.c -c -o baz.o
	libtool -static baz.o -o Baz.framework/Baz
	mkdir -p Bar.framework
	${CC} ${CCFLAGS} bar.c -c -o bar.o
	libtool -static bar.o -o Bar.framework/Bar
	mkdir -p Foo.framework
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	${LD} -r foo.o -add_linker_option '-framework Bar' -add_linker_option '-framework Baz' -o foo.o
	libtool -static foo.o -o Foo.framework/Foo
	${CC} ${CCFLAGS} main.c -c -o main.o
	${LD} -r main.o -add_linker_option '-framework Foo' -o main.o 
	${CC} ${CCFLAGS} main.o -o main -F.
	${DYLDINFO} -export main | grep _baz | ${FAIL_IF_EMPTY}
	${DYLDINFO} -export main | grep _bar | ${FAIL_IF_EMPTY}
	${DYLDINFO} -export main | grep _foo | ${FAIL_IF_EMPTY}
	${DYLDINFO} -export main | grep _main | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -rf main main.o foo.o bar.o baz.o Foo.framework Bar.framework Baz.framework
