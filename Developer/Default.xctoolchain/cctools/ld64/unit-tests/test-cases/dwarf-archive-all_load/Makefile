##
# Copyright (c) 2007-2008 Apple Inc. All rights reserved.
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
# Test that using -all_load to pull all .o files out of an archive
# proeduces good "debug notes".
#

run: 
	${CC} ${CCFLAGS} -gdwarf-2 foo.c -c -o foo.o
	${FAIL_IF_BAD_OBJ} foo.o
	${CC} ${CCFLAGS} -gdwarf-2 bar.c -c -o bar.o
	${FAIL_IF_BAD_OBJ} bar.o
	${CC} ${CCFLAGS} -gdwarf-2 baz.c -c -o baz.o
	${FAIL_IF_BAD_OBJ} baz.o
	libtool -static bar.o baz.o foo.o -o liball.a
	${CC} ${CCFLAGS} liball.a -all_load -dynamiclib -o liball.dylib -nodefaultlibs -lSystem
	${FAIL_IF_BAD_MACHO} liball.dylib
	nm -ap liball.dylib | ./stabs-filter.pl > liball.dylib.stabs
	${PASS_IFF} diff liball.dylib.stabs expected-stabs

clean:
	rm -rf *.o liball.a liball.dylib liball.dylib.stabs 
