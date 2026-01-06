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
# The point of this test is a sanity check that ld
# can link a progam with dtrace static probes
#

all: run

run: main main-dead_strip libmain.dylib main-r
	${FAIL_IF_BAD_MACHO} main
	${PASS_IFF_GOOD_MACHO} main-r

main: main.c foo.h bar.h
	${CC} ${CCFLAGS} main.c -o main 

main-dead_strip: main.c foo.h bar.h
	${CC} ${CCFLAGS} main.c -o main-dead_strip -dead_strip 

main-r: main.c foo.h bar.h
	${CC} ${CCFLAGS} main.c -c -o main.o 
	#${FAIL_IF_BAD_OBJ} main.o
	${LD} -r main.o -o main-r.o 
	#${FAIL_IF_BAD_OBJ} main-r.o
	${CC} ${CCFLAGS} main-r.o -o main-r

libmain.dylib: main.c foo.h bar.h
	${CC} ${CCFLAGS} main.c -dynamiclib -o libmain.dylib 

foo.h:	foo.d
	dtrace -h -s foo.d

bar.h:	bar.d
	dtrace -h -s bar.d

clean:
	rm -rf main libmain.dylib main-r main-dead_strip foo.h bar.h *.o
