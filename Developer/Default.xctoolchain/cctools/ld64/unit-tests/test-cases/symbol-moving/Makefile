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
# Test magic $ld$ symbols which tell ld to view exported symbols
# differently than dyld sees them.
#
# In this test case aaa and bbb both moved between libfoo and libar
# between 10.4 and 10.5.
#


run: all

all:
	# In this test case aaa and bbb both moved between libfoo and libar
	# between 10.4 and 10.5.
	${CC} ${CCFLAGS} -dynamiclib bar.c bbb.c anotb.c -o libbar.dylib  
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c aaa.c bnota.c -o libfoo.dylib 
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	${CC} ${CCFLAGS} main.c -o main-10.4  libfoo.dylib libbar.dylib -mmacosx-version-min=10.4
	nm -m main-10.4 | grep _aaa | grep libbar | ${FAIL_IF_EMPTY}
	nm -m main-10.4 | grep _bbb | grep libfoo | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} main.c -o main-10.4a libbar.dylib libfoo.dylib -mmacosx-version-min=10.4
	nm -m main-10.4a | grep _aaa | grep libbar | ${FAIL_IF_EMPTY}
	nm -m main-10.4a | grep _bbb | grep libfoo | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} main.c -o main-10.5  libfoo.dylib libbar.dylib -mmacosx-version-min=10.5
	nm -m main-10.5 | grep _aaa | grep libfoo | ${FAIL_IF_EMPTY}
	nm -m main-10.5 | grep _bbb | grep libbar | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} main.c -o main-10.5a libbar.dylib libfoo.dylib  -mmacosx-version-min=10.5
	nm -m main-10.5a | grep _aaa | grep libfoo | ${FAIL_IF_EMPTY}
	nm -m main-10.5a | grep _bbb | grep libbar | ${FAIL_IF_EMPTY}
	# In this test case aaa and bbb both moved between subframeworks of Foo and Bar
	# between 10.4 and 10.5.
	mkdir -p Frameworks/Foo.framework/Frameworks/subFoo.framework
	${CC} ${CCFLAGS} -dynamiclib foo.c aaa.c -o Frameworks/Foo.framework/Frameworks/subFoo.framework/subFoo \
		-install_name /System/Library/Frameworks/Foo.framework/Frameworks/subFoo.framework/subFoo \
		-umbrella Foo
	${CC} ${CCFLAGS} -dynamiclib bnota.c -o Frameworks/Foo.framework/Foo \
		-install_name /System/Library/Frameworks/Frameworks/Foo.framework/Foo \
		Frameworks/Foo.framework/Frameworks/subFoo.framework/subFoo
	mkdir -p Frameworks/Bar.framework/Frameworks/subBar.framework
	${CC} ${CCFLAGS} -dynamiclib bar.c bbb.c -o Frameworks/Bar.framework/Frameworks/subBar.framework/subBar \
		-install_name /System/Library/Frameworks/Bar.framework/Frameworks/subBar.framework/subBar \
		-umbrella Bar
	${CC} ${CCFLAGS} -dynamiclib anotb.c -o Frameworks/Bar.framework/Bar \
		-install_name /System/Library/Frameworks/Frameworks/Bar.framework/Bar \
		Frameworks/Bar.framework/Frameworks/subBar.framework/subBar
	${CC} ${CCFLAGS} main.c -o main-10.4  -framework Foo -framework Bar  -mmacosx-version-min=10.4 \
		-F./Frameworks -F./Frameworks/Bar.framework/Frameworks -F./Frameworks/Foo.framework/Frameworks 
	nm -m main-10.4 | grep _aaa | grep " Bar" | ${FAIL_IF_EMPTY}
	nm -m main-10.4 | grep _bbb | grep " Foo" | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} main.c -o main-10.4a -framework Bar  -framework Foo  -mmacosx-version-min=10.4 \
		-F./Frameworks -F./Frameworks/Bar.framework/Frameworks -F./Frameworks/Foo.framework/Frameworks 
	nm -m main-10.4a | grep _aaa | grep " Bar" | ${FAIL_IF_EMPTY}
	nm -m main-10.4a | grep _bbb | grep " Foo" | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} main.c -o main-10.5  -framework Foo -framework Bar  -mmacosx-version-min=10.5 \
		-F./Frameworks -F./Frameworks/Bar.framework/Frameworks -F./Frameworks/Foo.framework/Frameworks 
	nm -m main-10.5 | grep _aaa | grep " Foo" | ${FAIL_IF_EMPTY}
	nm -m main-10.5 | grep _bbb | grep " Bar" | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} main.c -o main-10.5a -framework Bar  -framework Foo  -mmacosx-version-min=10.5 \
		-F./Frameworks -F./Frameworks/Bar.framework/Frameworks -F./Frameworks/Foo.framework/Frameworks 
	nm -m main-10.5a | grep _aaa | grep " Foo" | ${FAIL_IF_EMPTY}
	nm -m main-10.5a | grep _bbb | grep " Bar" | ${FAIL_IF_EMPTY}
	${PASS_IFF} /usr/bin/true


clean:

	rm -rf  libbar.dylib libfoo.dylib main-10.4 main-10.5 main-10.4a main-10.5a Frameworks
