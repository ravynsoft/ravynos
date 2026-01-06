##
# Copyright (c) 2013 Apple Inc. All rights reserved.
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
#  Verify order files can pick file in archives
#

run: all

all:
	${CC} ${CCFLAGS} foo.c -c -o foo.o
	libtool -static foo.o -o libfoo.a 
	${CC} ${CCFLAGS} main.c -c -o main.o
	${CC} ${CCFLAGS} main.o libfoo.a -Wl,-order_file,main.order -o main
	${FAIL_IF_BAD_MACHO} main
	nm -n -j main | egrep "_main|_foo" > main.actual
	${PASS_IFF} diff main.actual main.expected


clean:
	rm -rf *.o libfoo.a main main.actual
