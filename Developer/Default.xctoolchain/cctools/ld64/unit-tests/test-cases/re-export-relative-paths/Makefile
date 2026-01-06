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
# Test that @loader_path and @executable_path can be resolved finding indirect dylibs
#


run: all

all:
	mkdir -p hide rhide
	${CC} ${CCFLAGS} -dynamiclib foo.c -install_name '@loader_path/libfoo.dylib' -o hide/libfoo.dylib 
	${FAIL_IF_BAD_MACHO} hide/libfoo.dylib
	${CC} ${CCFLAGS} -dynamiclib bar.c -o hide/libbar.dylib -install_name '@executable_path/hide/libbar.dylib'
	${FAIL_IF_BAD_MACHO} hide/libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib baz.c -o rhide/libbaz.dylib -install_name '@rpath/libbaz.dylib'
	${FAIL_IF_BAD_MACHO} hide/libbar.dylib
	${CC} ${CCFLAGS} -dynamiclib wrap.c -o hide/libwrap.dylib -Wl,-reexport-lfoo -Wl,-reexport-lbar -Lhide -Wl,-rpath,@loader_path/../rhide -Wl,-reexport-lbaz -Lrhide
	${FAIL_IF_BAD_MACHO} hide/libwrap.dylib
	${CC} ${CCFLAGS} main.c -o main hide/libwrap.dylib
	${CC} ${CCFLAGS} main.c -dynamiclib -o libmain.dylib hide/libwrap.dylib -Wl,-executable_path,`pwd`/main
	${CC} ${CCFLAGS} main.c -dynamiclib -o libmain.dylib hide/libwrap.dylib -Wl,-executable_path,`pwd`
	${PASS_IFF} /usr/bin/true


clean:

	rm -rf hide rhide main libmain.dylib
