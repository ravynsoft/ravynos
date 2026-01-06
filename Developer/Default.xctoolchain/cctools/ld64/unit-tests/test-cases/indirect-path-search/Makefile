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
# Test that -F and -L work when finding indirect libraries
#


run: all

all:

# build foo that re-exports bar
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib 
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -lbar -L. -sub_library libbar
	${FAIL_IF_BAD_MACHO} libfoo.dylib

# build an alternate libbar that also has baz
	mkdir -p hide
	${CC} ${CCFLAGS} -dynamiclib bar.c baz.c -o hide/libbar.dylib -install_name libbar.dylib
	${FAIL_IF_BAD_MACHO} hide/libbar.dylib

# build an executable that depends on a symbol in the alternate bar to validate that -L is used for indirect dylibs
	${CC} ${CCFLAGS} main.c -o main -lfoo -Lhide -L. 
	${FAIL_IF_BAD_MACHO} main



# build Foo.framework that re-exports Bar.framework
	mkdir -p Bar.framework Foo.framework
	${CC} ${CCFLAGS} -dynamiclib bar.c -o Bar.framework/Bar -install_name "`pwd`/Bar.framework/Bar"
	${FAIL_IF_BAD_MACHO} Bar.framework/Bar
	${CC} ${CCFLAGS} -dynamiclib foo.c -o Foo.framework/Foo -F. -framework Bar -sub_umbrella Bar
	${FAIL_IF_BAD_MACHO} Foo.framework/Foo

# build an alternate Bar.framework that also has baz
	mkdir -p hide/Bar.framework
	${CC} ${CCFLAGS} -dynamiclib bar.c baz.c -o hide/Bar.framework/Bar -install_name "`pwd`/Bar.framework/Bar"
	${FAIL_IF_BAD_MACHO} hide/Bar.framework/Bar

# build an executable that depends on a symbol in the alternate Bar.framework to validate that -F is used for indirect dylibs
	${CC} ${CCFLAGS} main.c -o main -Fhide -F. -framework Foo
	${FAIL_IF_BAD_MACHO} main



# build foo that links against bar
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib 
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib libbar.dylib  
	${FAIL_IF_BAD_MACHO} libfoo.dylib

# build an alternate libbar that also has baz
	mkdir -p hide
	${CC} ${CCFLAGS} -dynamiclib bar.c baz.c -o hide/libbar.dylib -install_name libbar.dylib
	${FAIL_IF_BAD_MACHO} hide/libbar.dylib

# build a flat executable that depends on a symbol in the alternate bar to validate that -L is used for indirect dylibs
	${CC} ${CCFLAGS} -flat_namespace main.c -o main -lfoo -Lhide -L. 
	${FAIL_IF_BAD_MACHO} main



# build Foo.framework that re-exports libbar.dylib embedded in framework
	mkdir -p Foo.framework
	${CC} ${CCFLAGS} -dynamiclib bar.c baz.c -o Foo.framework/libbar.dylib -install_name "`pwd`/Foo.framework/libbar.dylib"
	${FAIL_IF_BAD_MACHO} Foo.framework/libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -o Foo.framework/Foo -F. Foo.framework/libbar.dylib -sub_library libbar
	${FAIL_IF_BAD_MACHO} Foo.framework/Foo

# build an alternate libbar.dylib that does not have baz
	mkdir -p hide
	${CC} ${CCFLAGS} -dynamiclib bar.c -o hide/libbar.dylib
	${FAIL_IF_BAD_MACHO} hide/libbar.dylib

# build an executable that depends on a symbol not in the alternate libbar.dylib to validate dylibs embedded in frameworks are not searched for
	${CC} ${CCFLAGS} main.c -o main -Lhide -F. -framework Foo
	${PASS_IFF_GOOD_MACHO} main


clean:

	rm -rf hide libbar.dylib libfoo.dylib Foo.framework Bar.framework main
