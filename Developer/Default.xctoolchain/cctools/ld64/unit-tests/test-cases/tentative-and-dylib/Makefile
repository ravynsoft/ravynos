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

SHELL = bash # use bash shell so we can redirect just stderr


#
# Test how tentative definitions interact with dylibs
#

run: all

all:
	${CC} ${CCFLAGS} bar.c -dynamiclib -o libbar.dylib
	${CC} ${CCFLAGS} foo.c -dynamiclib -o libfoo.dylib -Wl,-reexport_library,libbar.dylib
	# verify -warn_commons works
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main -Wl,-warn_commons 2> warnings.log
	grep "using common symbol" warnings.log | ${FAIL_IF_EMPTY}
	# verify -commons use_dylibs works
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main -Wl,-commons,use_dylibs 
	nm -m main | grep _var | grep libfoo | ${FAIL_IF_EMPTY}
	# verify -commons ignore_dylibs works
	${CC} ${CCFLAGS} main.c libfoo.dylib -o main -Wl,-commons,ignore_dylibs 
	nm -m main | grep _var | grep __DATA | ${FAIL_IF_EMPTY}
	# verify -commons error works
	 ${FAIL_IF_SUCCESS} ${CC} ${CCFLAGS} main.c libfoo.dylib -o main -Wl,-commons,error  2> warnings.log
	# verify -commons use_dylibs works with indirect dylibs
	${CC} ${CCFLAGS} main.c  -Dvar=bar libfoo.dylib -o main -Wl,-commons,use_dylibs 
	nm -m main | grep _bar | grep libfoo | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main
		

clean:
	rm -rf main libfoo.dylib libbar.dylib warnings.log
