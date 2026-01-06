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
# Test that a public re-exported library is automatically added as a dependent
# unless nothing is used from it.
#


run: all

all:

	${CC} ${CCFLAGS} -dynamiclib pub.c -o libpub.dylib  -install_name /usr/lib/libpub.dylib
	${CC} ${CCFLAGS} -dynamiclib bar.c -o libbar.dylib 
	${CC} ${CCFLAGS} -dynamiclib baz.c -o libbaz.dylib  
	# verify uses of symbols from re-exported dylibs have ordinal that allows dyld to search amoung re-exported dylibs 
	${CC} ${CCFLAGS} -dynamiclib foo.c -DUSE_BAZ -o libfoo2.dylib -Wl,-reexport_library libpub.dylib -Wl,-reexport_library libbar.dylib  -Wl,-reexport_library libbaz.dylib 
	${DYLDINFO} -bind      libfoo2.dylib | grep _bar | grep this-image | ${FAIL_IF_EMPTY}
	${DYLDINFO} -lazy_bind libfoo2.dylib | grep _bar | grep this-image | ${FAIL_IF_EMPTY}
	${DYLDINFO} -bind      libfoo2.dylib | grep _baz | grep this-image | ${FAIL_IF_EMPTY}
	${DYLDINFO} -lazy_bind libfoo2.dylib | grep _baz | grep this-image | ${FAIL_IF_EMPTY}
	# verify that if only one non-publc re-exported dylib, don't use magic ordinal
	${CC} ${CCFLAGS} -dynamiclib foo.c -o libfoo1.dylib -Wl,-reexport_library libpub.dylib -Wl,-reexport_library libbar.dylib 
	${DYLDINFO} -bind      libfoo1.dylib | grep _bar | grep this-image | ${FAIL_IF_STDIN}
	${DYLDINFO} -lazy_bind libfoo1.dylib | grep _bar | grep this-image | ${FAIL_IF_STDIN}
	${PASS_IFF_GOOD_MACHO} libfoo2.dylib


clean:

	rm -rf libbar.dylib libbaz.dylib libpub.dylib libfoo1.dylib libfoo2.dylib  
