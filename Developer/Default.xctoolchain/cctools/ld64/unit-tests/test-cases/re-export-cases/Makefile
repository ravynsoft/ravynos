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
# Test all the different ways that re-exports can be specified and implemented
#


all: all-${ARCH}

all-i386: all-new all-old
all-x86_64: all-new all-old
all-armv6: all-new 
all-armv7: all-new 


all-new:	
	# -sub_library for 10.5
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib   ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -lbar -L. -sub_library libbar ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	otool -lv libfoo.dylib | grep LC_REEXPORT_DYLIB | ${FAIL_IF_EMPTY}
	otool -lv libfoo.dylib | grep LC_SUB_LIBRARY | ${FAIL_IF_STDIN}

	# -sub_umbrella for 10.5
	mkdir -p Bar.framework Foo.framework
	${CC} ${CCFLAGS} -dynamiclib bar.c -o Bar.framework/Bar -install_name "`pwd`/Bar.framework/Bar"  ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Bar.framework/Bar
	${CC} ${CCFLAGS} -dynamiclib foo.c -o Foo.framework/Foo -F. -framework Bar -sub_umbrella Bar  ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Foo.framework/Foo
	otool -lv Foo.framework/Foo | grep LC_REEXPORT_DYLIB | ${FAIL_IF_EMPTY}
	otool -lv Foo.framework/Foo | grep LC_SUB_UMBRELLA | ${FAIL_IF_STDIN}

	# -umbrella for 10.5
	mkdir -p Bar.framework Foo.framework
	${CC} ${CCFLAGS} -dynamiclib bar.c -o Bar.framework/Bar -install_name "`pwd`/Bar.framework/Bar" -umbrella Foo ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Bar.framework/Bar
	${CC} ${CCFLAGS} -dynamiclib foo.c -o Foo.framework/Foo -F. -framework Bar ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Foo.framework/Foo
	otool -lv Bar.framework/Bar | grep LC_SUB_FRAMEWORK | ${FAIL_IF_EMPTY}
	otool -lv Foo.framework/Foo | grep LC_REEXPORT_DYLIB | ${FAIL_IF_EMPTY}

	# -reexport_library for 10.5
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib  ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-reexport_library,libbar.dylib ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	otool -lv libfoo.dylib | grep LC_REEXPORT_DYLIB | ${FAIL_IF_EMPTY}
	otool -lv libfoo.dylib | grep LC_SUB_LIBRARY | ${FAIL_IF_STDIN}
	
	# -reexport-l for 10.5
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib   ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-reexport-lbar -L. ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	otool -lv libfoo.dylib | grep LC_REEXPORT_DYLIB | ${FAIL_IF_EMPTY}
	otool -lv libfoo.dylib | grep LC_SUB_LIBRARY | ${FAIL_IF_STDIN}

	# -reexport_framework for 10.5
	mkdir -p Bar.framework Foo.framework
	${CC} ${CCFLAGS} -dynamiclib bar.c -o Bar.framework/Bar -install_name "`pwd`/Bar.framework/Bar"  ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Bar.framework/Bar
	${CC} ${CCFLAGS} -dynamiclib foo.c -o Foo.framework/Foo -F. -Wl,-reexport_framework,Bar ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Foo.framework/Foo
	otool -lv Foo.framework/Foo | grep LC_REEXPORT_DYLIB | ${FAIL_IF_EMPTY}
	otool -lv Foo.framework/Foo | grep LC_SUB_UMBRELLA | ${FAIL_IF_STDIN}
	
	# -reexport_framework and -umbrella for 10.4
	mkdir -p Bar.framework Foo.framework
	${CC} ${CCFLAGS} -dynamiclib bar.c -o Bar.framework/Bar -install_name "`pwd`/Bar.framework/Bar" -umbrella Foo ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Bar.framework/Bar
	${CC} ${CCFLAGS} -dynamiclib foo.c -o Foo.framework/Foo -F. -Wl,-reexport_framework,Bar ${VERSION_NEW_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Foo.framework/Foo
	otool -lv Bar.framework/Bar | grep LC_SUB_FRAMEWORK | ${FAIL_IF_EMPTY}
	otool -lv Foo.framework/Foo | grep LC_REEXPORT_DYLIB | ${FAIL_IF_EMPTY}

	${PASS_IFF} /usr/bin/true



all-old:
	# -sub_library for 10.4
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib  ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -lbar -L. -sub_library libbar ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	otool -lv libfoo.dylib | grep LC_SUB_LIBRARY | ${FAIL_IF_EMPTY}
	otool -lv libfoo.dylib | grep LC_REEXPORT_DYLIB | ${FAIL_IF_STDIN}
	
	# -sub_umbrella for 10.4
	mkdir -p Bar.framework Foo.framework
	${CC} ${CCFLAGS} -dynamiclib bar.c -o Bar.framework/Bar -install_name "`pwd`/Bar.framework/Bar"  ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Bar.framework/Bar
	${CC} ${CCFLAGS} -dynamiclib foo.c -o Foo.framework/Foo -F. -framework Bar -sub_umbrella Bar  ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Foo.framework/Foo
	otool -lv Foo.framework/Foo | grep LC_SUB_UMBRELLA | ${FAIL_IF_EMPTY}
	otool -lv Foo.framework/Foo | grep LC_REEXPORT_DYLIB | ${FAIL_IF_STDIN}	

	# -umbrella for 10.4
	mkdir -p Bar.framework Foo.framework
	${CC} ${CCFLAGS} -dynamiclib bar.c -o Bar.framework/Bar -install_name "`pwd`/Bar.framework/Bar" -umbrella Foo ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Bar.framework/Bar
	${CC} ${CCFLAGS} -dynamiclib foo.c -o Foo.framework/Foo -F. -framework Bar ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Foo.framework/Foo
	otool -lv Bar.framework/Bar | grep LC_SUB_FRAMEWORK | ${FAIL_IF_EMPTY}
	otool -lv Foo.framework/Foo | grep LC_REEXPORT_DYLIB | ${FAIL_IF_STDIN}

	# -reexport_library for 10.4
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib  ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-reexport_library,libbar.dylib ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	otool -lv libfoo.dylib | grep LC_SUB_LIBRARY | ${FAIL_IF_EMPTY}
	otool -lv libfoo.dylib | grep LC_REEXPORT_DYLIB | ${FAIL_IF_STDIN}	

	# -reexport-l for 10.4
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib  ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo.dylib -Wl,-reexport-lbar -L. ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} libfoo.dylib
	otool -lv libfoo.dylib | grep LC_SUB_LIBRARY | ${FAIL_IF_EMPTY}
	otool -lv libfoo.dylib | grep LC_REEXPORT_DYLIB | ${FAIL_IF_STDIN}
	
	# -reexport_framework for 10.4
	mkdir -p Bar.framework Foo.framework
	${CC} ${CCFLAGS} -dynamiclib bar.c -o Bar.framework/Bar -install_name "`pwd`/Bar.framework/Bar"  ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Bar.framework/Bar
	${CC} ${CCFLAGS} -dynamiclib foo.c -o Foo.framework/Foo -F. -Wl,-reexport_framework,Bar ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Foo.framework/Foo
	otool -lv Foo.framework/Foo | grep LC_SUB_UMBRELLA | ${FAIL_IF_EMPTY}
	otool -lv Foo.framework/Foo | grep LC_REEXPORT_DYLIB | ${FAIL_IF_STDIN}

	# -reexport_framework and -umbrella for 10.4
	mkdir -p Bar.framework Foo.framework
	${CC} ${CCFLAGS} -dynamiclib bar.c -o Bar.framework/Bar -install_name "`pwd`/Bar.framework/Bar" -umbrella Foo ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Bar.framework/Bar
	${CC} ${CCFLAGS} -dynamiclib foo.c -o Foo.framework/Foo -F. -Wl,-reexport_framework,Bar ${VERSION_OLD_LINKEDIT}
	${FAIL_IF_BAD_MACHO} Foo.framework/Foo
	otool -lv Bar.framework/Bar | grep LC_SUB_FRAMEWORK | ${FAIL_IF_EMPTY}
	otool -lv Foo.framework/Foo | grep LC_REEXPORT_DYLIB | ${FAIL_IF_STDIN}

	${PASS_IFF} /usr/bin/true


clean:

	rm -rf hide libbar.dylib libfoo.dylib Foo.framework Bar.framework 
