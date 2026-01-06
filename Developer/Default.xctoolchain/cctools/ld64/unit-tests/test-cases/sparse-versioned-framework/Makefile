##
# Copyright (c) 2020 Apple Inc. All rights reserved.
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
# Check versioned framework with missing symlinks can be used with -framework
#

run: all

all:
	mkdir -p Foo.framework/Versions/A
	ln -s A Foo.framework/Versions/Current
	${CC} ${CCFLAGS} foo.c -dynamiclib -o Foo.framework/Versions/Current/Foo_test
	${PASS_IFF_ERROR} ${CC} ${CCFLAGS} main.c -o main -framework Foo,_test -F. 2>/dev/null
	${PASS_IFF} ${CC} ${CCFLAGS} main.c -o main -Wl,-search_in_sparse_frameworks -framework Foo,_test -F.
	${PASS_IFF} ${CC} ${CCFLAGS} main.c -o main -Wl,-search_in_sparse_frameworks -Wl,-image_suffix,_test -framework Foo -F.
	${PASS_IFF_GOOD_MACHO} main

clean:
	rm -rf main Foo.framework
	
