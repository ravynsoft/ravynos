##
# Copyright (c) 2011 Apple Inc. All rights reserved.
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
# Sanity check -dead_strip does not remove initializers and terminators
#


run: all

all:
	${CXX} ${CCFLAGS} main.cxx other.cxx -dead_strip -o main
	${FAIL_IF_BAD_MACHO} main
	nm main | grep dead_door_knob | ${FAIL_IF_STDIN}
	nm main | grep ctr | ${FAIL_IF_EMPTY}
	nm main | grep dtr | ${FAIL_IF_EMPTY}
	${CXX} ${CCFLAGS} -static main.cxx other.cxx -dead_strip -nostdlib -e _main -o main-static -Wl,-new_linker
	${FAIL_IF_BAD_MACHO} main-static
	nm main-static | grep dead_door_knob | ${FAIL_IF_STDIN}
	nm main-static | grep ctr | ${FAIL_IF_EMPTY}
	nm main-static | grep dtr | ${FAIL_IF_EMPTY}
	${PASS_IFF_GOOD_MACHO} main-static

clean:
	rm -rf main main-static
