##
# Copyright (c) 2006-2007 Apple Inc. All rights reserved.
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
# The point of this test is a sanity check -order_file.
# The main1 test verifies that C functions can be re-ordered
# The main2 test verifies that a block of assembly is not moves en mas
# The main1 test verifies that an order file with spaces and comments works
#

run: all

all:
	${CC} ${CCFLAGS} main.c extra.s -o main1 -Wl,-order_file -Wl,main1.order 
	${FAIL_IF_BAD_MACHO} main1
	nm -n -g -j main1 | grep "_main" > main1.nm
	${PASS_IFF} diff main1.nm main1.expected

	${CC} ${CCFLAGS} main.c extra.s -o main2 -Wl,-order_file -Wl,main2.order 
	${FAIL_IF_BAD_MACHO} main2
	nm -n -j main2 | egrep '^_[a-z]+[0-9]$$' > main2.nm
	${PASS_IFF} diff main2.nm main2.expected

	${CC} ${CCFLAGS} main.c -c -o main.o
	${CC} ${CCFLAGS} main.o extra.s -o main3 -Wl,-order_file -Wl,main3.order 
	${FAIL_IF_BAD_MACHO} main3
	nm -n -g -j main3 | grep "_main" > main3.nm
	${PASS_IFF} diff main3.nm main3.expected

	${CC} ${CCFLAGS} main.c extra.s -DSUBSECTIONS=1 -o main4 -Wl,-order_file -Wl,main4.order 
	${FAIL_IF_BAD_MACHO} main4
	nm -n -g -j main4 | egrep '_[a-zA-Z]+[1-9]' > main4.nm
	${PASS_IFF} diff main4.nm main4.expected




clean:
	rm -rf main1 main2 main3 main4 main.o *.nm
