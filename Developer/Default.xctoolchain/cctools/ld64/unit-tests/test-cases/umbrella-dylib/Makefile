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
# Test out building an umbrella dylib.  a and c are co-dependent.  b depends on c.
#


run: all

all:
	# build bootstrap dylib of with all code
	${CC} ${CCFLAGS} -dynamiclib a.c b.c c.c -o libBig.stub  -install_name /usr/local/lib/libBig.dylib 
	# link each sub library against bootstrap to intra-Big symbols
	${CC} ${CCFLAGS} -dynamiclib a.c -o liba.dylib libBig.stub -install_name /usr/local/lib/liba.dylib -umbrella Big
	${CC} ${CCFLAGS} -dynamiclib b.c -o libb.dylib libBig.stub -install_name /usr/local/lib/libb.dylib -umbrella Big
	${CC} ${CCFLAGS} -dynamiclib c.c -o libc.dylib libBig.stub -install_name /usr/local/lib/libc.dylib -umbrella Big
	${CC} ${CCFLAGS} -dynamiclib -Wl,-reexport_library,liba.dylib -Wl,-reexport_library,libb.dylib -Wl,-reexport_library,libc.dylib -o libBig.dylib -install_name /usr/local/lib/libBig.dylib 
	# re-link against correct dylibs now that everything is built
	${CC} ${CCFLAGS} -dynamiclib a.c -o liba.dylib libc.dylib -install_name /usr/local/lib/liba.dylib -umbrella Big
	${DYLDINFO} -lazy_bind liba.dylib | grep c2 | grep libc | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} -dynamiclib b.c -o libb.dylib libc.dylib -install_name /usr/local/lib/libb.dylib -umbrella Big
	${CC} ${CCFLAGS} -dynamiclib c.c -o libc.dylib liba.dylib -install_name /usr/local/lib/libc.dylib -umbrella Big
	${DYLDINFO} -lazy_bind libc.dylib | grep a2 | grep liba | ${FAIL_IF_EMPTY}
	${CC} ${CCFLAGS} -dynamiclib -Wl,-reexport_library,liba.dylib -Wl,-reexport_library,libb.dylib -Wl,-reexport_library,libc.dylib -o libBig.dylib -install_name /usr/local/lib/libBig.dylib 
	${CC} ${CCFLAGS} main.c -o main libBig.dylib -L.
	${PASS_IFF_GOOD_MACHO} main


clean:

	rm -rf libBig.stub liba.dylib libb.dylib libc.dylib libBig.dylib main
